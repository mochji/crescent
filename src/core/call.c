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

/* next: error handling and stack resizing */

void
crsC_setError(crs_State* state, char* error) {
	if (state->error != state->gState->memoryError) {
		free(state->error);
	}

	if (error == NULL) {
		state->error = NULL;

		return;
	}

	state->error = malloc(strlen(error) + 1);

	if (state->error == NULL) {
		state->error = state->gState->memoryError;
		crsC_throw(state, CRS_STATUS_NOMEM);
	}

	strcpy(state->error, error);
}

void
crsC_moveError(crs_State* to, crs_State* from) {
	if (from->error == from->gState->memoryError) {
		to->error = to->gState->memoryError;
	} else {
		to->error = from->error;
	}

	from->error = NULL;
}

void
crsC_throw(crs_State* state, int status) {
	crs_GState* gState = state->gState;

	if (state->handler != NULL) {
		state->status = status;
		longjmp(*state->handler, 1);
	}

	if (gState->mainThread->handler != NULL) {
		crsC_moveError(gState->mainThread, state);
		crsC_throw(gState->mainThread, status);
	}

	if (gState->panic != NULL) {
		gState->panic(state);
	}

	abort();
}

void
crsC_memoryError(crs_State* state) {
	char* memoryError = state->gState->memoryError;

	if (state->error != memoryError) {
		free(state->error);
	}

	state->error = memoryError;
	crsC_throw(state, CRS_STATUS_NOMEM);
}

void
crsC_restoreStack(crs_State* state, short level) {
	while (state->stack.calls > level) {
		crs_Frame* frame = state->stack.frame;

		crs_Object* from = state->stack.top - 1;
		crs_Object* to   = frame->base;

		while (from >= to) {
			crsO_free(from--);
		}

		state->stack.top     = to;
		state->stack.calls  -= 1;
		state->stack.cCalls -= 1;
		state->stack.frame   = frame->previous;

		free(frame);
	}
}

int
crsC_reallocStack(crs_State* state, size_t newSize, int throw) {
	crs_Object* newStack = realloc(state->stack.base, newSize * sizeof(crs_Object));

	if (newStack == NULL) {
		if (throw) {
			crsC_memoryError(state);
		}

		return 1;
	}

	ptrdiff_t  offset = newStack - state->stack.base;
	crs_Frame* frame  = state->stack.frame;

	state->stack.size = newSize;
	state->stack.base = newStack;
	state->stack.top += offset;

	while (frame != NULL) {
		frame->base += offset;
		frame        = frame->previous;
	}

	return 0;
}

int
crsC_resizeStack(crs_State* state, size_t needed, int throw) {
	if (needed > CRS_MAX_STACK) {
		if (throw) {
			crsC_setError(state, "stack overflow");
			crsC_throw(state, CRS_STATUS_ERROR);
		}

		return 1;
	}

	size_t size    = state->stack.size;
	size_t newSize = needed + needed / 2;

	if (newSize > CRS_MAX_STACK) {
		newSize = needed;
	} else if (newSize < CRS_MIN_STACK) {
		newSize = CRS_MIN_STACK;
	}

	if (needed <= size / 3 && size > CRS_MIN_STACK) {
		crsC_reallocStack(state, newSize, 0);

		return 0;
	} else if (needed > size) {
		return crsC_reallocStack(state, newSize, throw);
	}

	return 0;
}

int
crsC_checkTop(crs_State* state, int top, int throw) {
	crs_Object* stack  = state->stack.base;
	crs_Frame*  frame  = state->stack.frame;
	size_t      needed = (frame->base - stack) + top;

	frame = frame->previous;

	while (frame != NULL) {
		size_t frameNeeds = (frame->base - stack) + frame->top;

		if (frameNeeds > needed) {
			needed = frameNeeds;
		}

		frame = frame->previous;
	}

	if (crsC_resizeStack(state, needed, throw)) {
		return 1;
	}

	state->stack.frame->top = top;

	return 0;
}

int
crsC_checkFree(crs_State* state, int free, int throw) {
	size_t needed = (state->stack.top - state->stack.base) + free;

	if (needed > state->stack.size) {
		return crsC_resizeStack(state, needed, throw);
	}

	return 0;
}

void
crsC_startCall(crs_State* state, crs_Frame* frame, int top, int args) {
	crsC_checkFree(state, top - args, 1);

	crs_Frame* oldFrame = state->stack.frame;

	oldFrame->next = frame;

	frame->base     = state->stack.top - args;
	frame->top      = top;
	frame->next     = NULL;
	frame->previous = oldFrame;

	state->stack.calls  += 1;
	state->stack.cCalls += 1;
	state->stack.frame   = frame;
}

void
crsC_endCall(crs_State* state, int results) {
	crs_Frame* frame    = state->stack.frame;
	crs_Frame* oldFrame = frame->previous;

	int discarded = (state->stack.top - frame->base) - results;

	if (discarded > 0) {
		crs_Object* from;
		crs_Object* to;

		to = frame->base;

		for (int a = 0; a < discarded; a++) {
			crsO_free(to++);
		}

		from = state->stack.top - results;
		to   = frame->base;

		for (int a = 0; a < results; a++) {
			*(to++) = *(from++);
		}
	}

	oldFrame->top += results;
	oldFrame->next = NULL;

	state->stack.top    -= discarded;
	state->stack.calls  -= 1;
	state->stack.cCalls -= 1;
	state->stack.frame   = oldFrame;
}

int
crsC_callC(crs_State* state, crs_CFunction* function, int args, int maxResults) {
	crs_Frame frame;
	int       results;

	crsC_startCall(
		state,
		&frame,
		args < CRS_MIN_TOP
			? CRS_MIN_TOP
			: args,
		args
	);

	results = function(state);

	if (results > maxResults) {
		results = maxResults;
	}

	crsC_endCall(state, results);

	return results;
}
