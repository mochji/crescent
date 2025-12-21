/*
 * https://github.com/mochji/crescent
 * core/call.c
 *
 * Copyright (C) 2025 mochji
 * MIT License
 */

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <setjmp.h>

#include "conf.h"
#include "limit.h"

#include "core/object.h"
#include "core/state.h"

#include "core/call.h"

/*
 * TODO: rework error handling
 *
 * error handling is to be replaced with a better object-error system, so
 * pretty much everything here related to error handling is to be redone.
 */

void
crsC_setError(crs_Thread* thread, char* error) {
	if (thread->error != thread->state->memoryError) {
		free(thread->error);
	}

	if (error == NULL) {
		thread->error = NULL;

		return;
	}

	thread->error = malloc(strlen(error) + 1);

	if (thread->error == NULL) {
		thread->error = thread->state->memoryError;
		crsC_throw(thread, CRS_STATUS_NOMEM);
	}

	strcpy(thread->error, error);
}

void
crsC_moveError(crs_Thread* to, crs_Thread* from) {
	if (from->error == from->state->memoryError) {
		to->error = to->state->memoryError;
	} else {
		to->error = from->error;
	}

	from->error = NULL;
}

void
crsC_throw(crs_Thread* thread, int status) {
	crs_State* state = thread->state;

	if (thread->handler != NULL) {
		thread->status = status;
		longjmp(*thread->handler, 1);
	}

	if (state->mainThread->handler != NULL) {
		crsC_moveError(state->mainThread, thread);
		crsC_throw(state->mainThread, status);
	}

	if (state->panic != NULL) {
		state->panic(thread);
	}

	abort();
}

void
crsC_memoryError(crs_Thread* thread) {
	char* memoryError = thread->state->memoryError;

	if (thread->error != memoryError) {
		free(thread->error);
	}

	thread->error = memoryError;
	crsC_throw(thread, CRS_STATUS_NOMEM);
}

void
crsC_restoreStack(crs_Thread* thread, short level) {
	while (thread->stack.calls > level) {
		crs_Frame* frame = thread->stack.frame;

		crs_Object* from = thread->stack.top - 1;
		crs_Object* to   = frame->base;

		while (from >= to) {
			crsO_free(from--);
		}

		thread->stack.top     = to;
		thread->stack.calls  -= 1;
		thread->stack.cCalls -= 1;
		thread->stack.frame   = frame->previous;
	}
}

int
crsC_reallocStack(crs_Thread* thread, size_t newSize, int throw) {
	crs_Object* newStack = realloc(thread->stack.base, newSize * sizeof(crs_Object));

	if (newStack == NULL) {
		if (throw) {
			crsC_memoryError(thread);
		}

		return 1;
	}

	ptrdiff_t  offset = newStack - thread->stack.base;
	crs_Frame* frame  = thread->stack.frame;

	thread->stack.size = newSize;
	thread->stack.base = newStack;
	thread->stack.top += offset;

	while (frame != NULL) {
		frame->base += offset;
		frame        = frame->previous;
	}

	return 0;
}

int
crsC_resizeStack(crs_Thread* thread, size_t needed, int throw) {
	if (needed > CRS_MAX_STACK) {
		if (throw) {
			crsC_setError(thread, "stack overflow");
			crsC_throw(thread, CRS_STATUS_ERROR);
		}

		return 1;
	}

	size_t size    = thread->stack.size;
	size_t newSize = needed + needed / 2;

	if (newSize > CRS_MAX_STACK) {
		newSize = needed;
	} else if (newSize < CRS_MIN_STACK) {
		newSize = CRS_MIN_STACK;
	}

	if (needed <= size / 3 && size > CRS_MIN_STACK) {
		crsC_reallocStack(thread, newSize, 0);

		return 0;
	} else if (needed > size) {
		return crsC_reallocStack(thread, newSize, throw);
	}

	return 0;
}

int
crsC_checkTop(crs_Thread* thread, int top, int throw) {
	crs_Object* stack  = thread->stack.base;
	crs_Frame*  frame  = thread->stack.frame;
	size_t      needed = (frame->base - stack) + top;

	frame = frame->previous;

	while (frame != NULL) {
		size_t frameNeeds = (frame->base - stack) + frame->top;

		if (frameNeeds > needed) {
			needed = frameNeeds;
		}

		frame = frame->previous;
	}

	if (crsC_resizeStack(thread, needed, throw)) {
		return 1;
	}

	thread->stack.frame->top = top;

	return 0;
}

int
crsC_checkFree(crs_Thread* thread, int free, int throw) {
	size_t needed = (thread->stack.top - thread->stack.base) + free;

	if (needed > thread->stack.size) {
		return crsC_resizeStack(thread, needed, throw);
	}

	return 0;
}

void
crsC_startCall(crs_Thread* thread, crs_Frame* frame, int top, int args) {
	crsC_checkFree(thread, top - args, 1);

	crs_Frame* oldFrame = thread->stack.frame;

	oldFrame->next = frame;

	frame->base     = thread->stack.top - args;
	frame->top      = top;
	frame->next     = NULL;
	frame->previous = oldFrame;

	thread->stack.calls  += 1;
	thread->stack.cCalls += 1;
	thread->stack.frame   = frame;
}

void
crsC_endCall(crs_Thread* thread, int results) {
	crs_Frame* frame    = thread->stack.frame;
	crs_Frame* oldFrame = frame->previous;

	int discarded = (thread->stack.top - frame->base) - results;

	if (discarded > 0) {
		crs_Object* from;
		crs_Object* to;

		to = frame->base;

		for (int a = 0; a < discarded; a++) {
			crsO_free(to++);
		}

		from = thread->stack.top - results;
		to   = frame->base;

		for (int a = 0; a < results; a++) {
			*(to++) = *(from++);
		}
	}

	oldFrame->top += results;
	oldFrame->next = NULL;

	thread->stack.top    -= discarded;
	thread->stack.calls  -= 1;
	thread->stack.cCalls -= 1;
	thread->stack.frame   = oldFrame;
}

int
crsC_callC(crs_Thread* thread, crs_CFunction* function, int args, int maxResults) {
	crs_Frame frame;
	int       results;

	crsC_startCall(
		thread,
		&frame,
		args < CRS_MIN_TOP
			? CRS_MIN_TOP
			: args,
		args
	);

	results = function(thread);

	if (results > maxResults) {
		results = maxResults;
	}

	crsC_endCall(thread, results);

	return results;
}
