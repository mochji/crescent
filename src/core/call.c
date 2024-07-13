/*
 * https://github.com/mochji/crescent
 * core/call.c
 *
 * Copyright (C) 2024 mochji
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
		crescentC_throw(state, CRESCENT_STATUS_ERRMEM);
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
	crescentC_throw(state, CRESCENT_STATUS_ERRMEM);
}

int
crescentC_reallocStack(crescent_State* state, size_t size) {
	crescent_Object* stack = realloc(state->stack.base, size * sizeof(crescent_Object));

	if (stack == NULL) {
		return 1;
	}

	ptrdiff_t       offset = stack - state->stack.base;
	crescent_Frame* frame  = state->stack.topFrame;

	state->stack.size = size;
	state->stack.base = stack;
	state->stack.top += offset;

	while (frame != NULL) {
		frame->base += offset;
		frame        = frame->next;
	}

	return 0;
}

int
crescentC_resizeStack(crescent_State* state, int top, int throw) {
	size_t absTop  = (size_t)(state->stack.topFrame->base - state->stack.base) + top;
	size_t newSize = absTop + absTop / 2;
	int    failed  = 0;

	if (absTop <= state->stack.size / 3) {
		if (state->stack.size == CRESCENT_MIN_STACK) {
			return 0;
		}

		if (newSize < CRESCENT_MIN_STACK) {
			newSize = CRESCENT_MIN_STACK;
		}

		/*
		 * even if reallocating the stack fails here, we can just lie and say
		 * it did since we should never be accessing any pointer greater than
		 * state->stack.base + state->stack.size - 1, and if we are we were
		 * already fucked.
		 */

		crescentC_reallocStack(state, newSize);

		state->stack.size = newSize; /* yes we're setting it twice, shut up */
	} else if (absTop >= state->stack.size - 1) {
		failed = crescentC_reallocStack(state, newSize);
	}

	if (failed && throw) {
		crescentC_memoryError(state);
	}

	return failed;
}

void
crescentC_startCall(crescent_State* state, int args, crescent_Frame* newTopFrame) {
	crescent_Frame* oldTopFrame = state->stack.topFrame;

	oldTopFrame->top -= args;
	oldTopFrame->next = newTopFrame;

	newTopFrame->base     = state->stack.top - args;
	newTopFrame->top      = args;
	newTopFrame->next     = NULL;
	newTopFrame->previous = oldTopFrame;

	state->stack.calls   += 1;
	state->stack.cCalls  += 1;
	state->stack.topFrame = newTopFrame;
}

void
crescentC_endCall(crescent_State* state, int results) {
	crescent_Frame* newTopFrame = state->stack.topFrame;
	crescent_Frame* oldTopFrame = newTopFrame->previous;

	if (results != newTopFrame->top) {
		crescent_Object* fromBase;
		crescent_Object* toBase;

		toBase = newTopFrame->base;

		for (int a = 0; a < newTopFrame->top - results; a++) {
			crescentO_free(toBase + a);
		}

		fromBase = state->stack.top - results;
		toBase   = newTopFrame->base;

		for (int a = 0; a < results; a++) {
			*(toBase + a) = *(fromBase + a);
		}
	}

	oldTopFrame->top += results;
	oldTopFrame->next = NULL;

	state->stack.calls   -= 1;
	state->stack.cCalls  -= 1;
	state->stack.topFrame = oldTopFrame;
}
