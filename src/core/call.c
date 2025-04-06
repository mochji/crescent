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

void
crescentC_setError(crescent_State* state, char* error) {
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
		crescentC_throw(state, CRESCENT_STATUS_NOMEM);
	}

	strcpy(state->error, error);
}

void
crescentC_moveError(crescent_State* to, crescent_State* from) {
	if (from->error == from->gState->memoryError) {
		to->error = to->gState->memoryError;
	} else {
		to->error = from->error;
	}

	from->error = NULL;
}

void
crescentC_throw(crescent_State* state, int status) {
	crescent_GState* gState = state->gState;

	if (state->errorJump != NULL) {
		state->errorJump->status = status;
		longjmp(state->errorJump->buffer, 1);
	}

	if (gState->baseThread->errorJump != NULL) {
		crescentC_moveError(gState->baseThread, state);
		crescentC_throw(gState->baseThread, status);
	}

	if (gState->panic != NULL) {
		gState->panic(state);
	}

	abort();
}

void
crescentC_memoryError(crescent_State* state) {
	char* memoryError = state->gState->memoryError;

	if (state->error != memoryError) {
		free(state->error);
	}

	state->error = memoryError;
	crescentC_throw(state, CRESCENT_STATUS_NOMEM);
}

void
crescentC_restoreStack(crescent_State* state) {
	crescent_ErrorJump* errorJump = state->errorJump;

	crescent_Object* from = state->stack.top - 1;
	crescent_Object* to   = errorJump->top;

	while (from >= to) {
		crescentO_free(from--);
	}

	state->stack.top    = errorJump->top;
	state->stack.calls  = errorJump->calls;
	state->stack.cCalls = errorJump->cCalls;
	state->stack.frame  = errorJump->frame;
}

int
crescentC_reallocStack(crescent_State* state, size_t newSize) {
	crescent_Object* newStack = realloc(state->stack.base, newSize * sizeof(crescent_Object));

	if (newStack == NULL) {
		return 1;
	}

	ptrdiff_t       offset = newStack - state->stack.base;
	crescent_Frame* frame  = state->stack.frame;

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
crescentC_checkFree(crescent_State* state, int free) {
	size_t needed = (state->stack.top - state->stack.base) + free;

	if (needed > CRESCENT_MAX_STACK) {
		return 1;
	}

	if (needed > state->stack.size) {
		return crescentC_reallocStack(state, needed);
	}

	return 0;
}

int
crescentC_checkTop(crescent_State* state, int top) {
	crescent_Object* stack  = state->stack.base;
	crescent_Frame*  frame  = state->stack.frame;
	size_t           needed = (frame->base - stack) + top;

	frame = frame->previous;

	while (frame != NULL) {
		size_t frameNeeds = (frame->base - stack) + frame->top;

		if (frameNeeds > needed) {
			needed = frameNeeds;
		}

		frame = frame->previous;
	}

	size_t size    = state->stack.size;
	size_t newSize = needed + needed / 2;

	if (needed <= size / 3) {
		if (size == CRESCENT_MIN_STACK) {
			return 0;
		}

		if (newSize < CRESCENT_MIN_STACK) {
			newSize = CRESCENT_MIN_STACK;
		}

		crescentC_reallocStack(state, newSize);
	} else if (needed > size) {
		if (newSize > CRESCENT_MAX_STACK) {
			return 2;
		}

		if (crescentC_reallocStack(state, newSize)) {
			return 1;
		}
	}

	state->stack.frame->top = top;

	return 0;
}

void
crescentC_startCall(crescent_State* state, crescent_Frame* frame, int top, int args) {
	crescent_Frame* oldFrame = state->stack.frame;

	int failed = crescentC_checkFree(state, top - args);

	if (failed) {
		if (failed == 2) {
			crescentC_setError(state, "stack overflow");
			crescentC_throw(state, CRESCENT_STATUS_ERROR);
		} else {
			crescentC_memoryError(state);
		}
	}

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
crescentC_endCall(crescent_State* state, int results) {
	crescent_Frame* frame    = state->stack.frame;
	crescent_Frame* oldFrame = frame->previous;

	int discarded = (state->stack.top - frame->base) - results;

	if (discarded > 0) {
		crescent_Object* from;
		crescent_Object* to;

		to = frame->base;

		for (int a = 0; a < discarded; a++) {
			crescentO_free(to++);
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
crescentC_callC(crescent_State* state, crescent_CFunction* function, int args, int maxResults) {
	crescent_Frame frame;
	int            results;

	crescentC_startCall(
		state,
		&frame,
		args < CRESCENT_MIN_TOP
			? CRESCENT_MIN_TOP
			: args,
		args
	);

	results = function(state);

	if (results > maxResults) {
		results = maxResults;
	}

	crescentC_endCall(state, results);

	return results;
}
