/*
 * https://github.com/mochji/crescent
 * core/call.c
 *
 * idrk what this is
 * Copyright (C) 2024 mochji
 *
 * MIT License
 */

#include <stdlib.h>
#include <stddef.h>
#include <setjmp.h>
#include <string.h>
#include <limits.h>

#include "conf.h"

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

	size_t length     = strlen(error);
	char*  stateError = malloc(length + 1);

	if (stateError == NULL) {
		crescentC_memoryError(state);
	}

	for (size_t a = 0; a < length; a++) {
		stateError[a] = error[a];
	}

	stateError[length] = '\0';

	state->error = stateError;
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
crescentC_panic(crescent_State* state) {
	crescent_GState* gState = state->gState;

	if (gState->panic != NULL) {
		gState->panic(state);
	}

	abort();
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

	crescentC_panic(state);
}

void
crescentC_memoryError(crescent_State* state) {
	state->error = state->gState->memoryError;
	crescentC_throw(state, CRESCENT_STATUS_ERRMEM);
}

int
crescentC_growStack(crescent_State* state, int newTop) {
	unsigned int     absoluteTop = state->stack.topFrame->base + newTop;
	size_t           newSize     = state->stack.size;
	crescent_Object* newData;

	int usage = (absoluteTop * 100 + newSize / 2) / newSize;

	while (usage > CRESCENT_STACK_GROWTHRESHOLD) {
		newSize *= 2;
		usage   /= 2;
	}

	newData = realloc(state->stack.data, newSize * sizeof(crescent_Object));

	if (newData == NULL) {
		return 1;
	}

	state->stack.size = newSize;
	state->stack.data = newData;

	return 0;
}

int
crescentC_shrinkStack(crescent_State* state, int newTop) {
	unsigned int     absoluteTop = state->stack.topFrame->base + newTop;
	size_t           newSize     = state->stack.size;
	crescent_Object* newData;

	int usage = (absoluteTop * 100 + newSize / 2) / newSize;

	while (usage < CRESCENT_STACK_SHRINKTHRESHOLD && newSize > CRESCENT_STACK_INITSIZE) {
		newSize /= 2;
		usage   *= 2;
	}

	size_t oldTop = state->stack.topFrame->base + state->stack.topFrame->top;

	for (size_t a = absoluteTop; a < oldTop; a++) {
		crescentO_free(&state->stack.data[a]);
	}

	newData = realloc(state->stack.data, newSize * sizeof(crescent_Object));

	if (newData == NULL) {
		return 1;
	}

	state->stack.size = newSize;
	state->stack.data = newData;

	return 0;
}

int
crescentC_resizeStack(crescent_State* state, int newTop, int throw) {
	unsigned int absoluteTop = state->stack.topFrame->base + newTop;
	int          usage       = (absoluteTop * 100 + state->stack.size / 2) / state->stack.size;
	int          failed      = 0;

	if (usage < CRESCENT_STACK_SHRINKTHRESHOLD) {
		if (state->stack.size == CRESCENT_STACK_INITSIZE) {
			return 0;
		}

		failed = crescentC_shrinkStack(state, newTop);
	} else if (usage > CRESCENT_STACK_GROWTHRESHOLD) {
		failed = crescentC_growStack(state, newTop);
	}

	if (failed && throw) {
		crescentC_memoryError(state);
	}

	return failed;
}

void
crescentC_startCall(crescent_State* state, int argCount, crescent_Frame* newTopFrame) {
	crescent_Frame* oldTopFrame = state->stack.topFrame;

	oldTopFrame->top -= argCount;
	oldTopFrame->next = newTopFrame;

	newTopFrame->base     = oldTopFrame->top;
	newTopFrame->top      = argCount;
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
		unsigned int fromBaseIndex;
		unsigned int toBaseIndex;

		toBaseIndex = newTopFrame->base;

		for (int a = 0; a < newTopFrame->top - results; a++) {
			crescentO_free(&state->stack.data[toBaseIndex + a]);
		}

		fromBaseIndex = newTopFrame->base + newTopFrame->top - results;
		toBaseIndex   = newTopFrame->base;

		for (int a = 0; a < results; a++) {
			state->stack.data[toBaseIndex + a] = state->stack.data[fromBaseIndex + a];
		}
	}

	oldTopFrame->top += results;
	oldTopFrame->next = NULL;

	state->stack.calls   -= 1;
	state->stack.cCalls  -= 1;
	state->stack.topFrame = oldTopFrame;
}
