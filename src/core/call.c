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
#include <string.h>

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

	state->error = malloc(strlen(error) + 1);

	if (state->error == NULL) {
		crescentC_memoryError(state);
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
	if (state->error != state->gState->memoryError) {
		free(state->error);
	}

	state->error = state->gState->memoryError;
	crescentC_throw(state, CRESCENT_STATUS_ERRMEM);
}

void
crescentC_correctPointers(crescent_State* state, crescent_Object* newData) {
	ptrdiff_t       offset = newData - state->stack.data;
	crescent_Frame* frame  = state->stack.topFrame;

	while (frame != NULL) {
		frame->base += offset;
		frame        = frame->next;
	}
}

int
crescentC_growStack(crescent_State* state, int usage) {
	size_t           newSize = state->stack.size * 2;
	crescent_Object* newData;

	usage /= 2;

	while (usage > CRESCENT_STACK_GROWTHRESHOLD) {
		newSize *= 2;
		usage   /= 2;
	}

	newData = realloc(state->stack.data, newSize * sizeof(crescent_Object));

	if (newData == NULL) {
		return 1;
	}

	crescentC_correctPointers(state, newData);

	state->stack.size = newSize;
	state->stack.data = newData;

	return 0;
}

int
crescentC_shrinkStack(crescent_State* state, size_t absTop, int usage) {
	size_t           newSize = state->stack.size / 2;
	crescent_Object* newData;

	usage *= 2;

	while (usage < CRESCENT_STACK_SHRINKTHRESHOLD && newSize > CRESCENT_STACK_INITSIZE) {
		newSize /= 2;
		usage   *= 2;
	}

	crescent_Object* object = state->stack.data + absTop;
	crescent_Object* to     = state->stack.topFrame->base + state->stack.topFrame->top;

	for (; object <= to; object++) {
		crescentO_free(object);
	}

	newData = realloc(state->stack.data, newSize * sizeof(crescent_Object));

	if (newData == NULL) {
		return 1;
	}

	crescentC_correctPointers(state, newData);

	state->stack.size = newSize;
	state->stack.data = newData;

	return 0;
}

int
crescentC_resizeStack(crescent_State* state, int top, int throw) {
	size_t absTop = state_absbase(state) + top;
	int    usage  = (absTop * 100) / state->stack.size;
	int    failed = 0;

	if (usage < CRESCENT_STACK_SHRINKTHRESHOLD) {
		if (state->stack.size == CRESCENT_STACK_INITSIZE) {
			return 0;
		}

		failed = crescentC_shrinkStack(state, absTop, usage);
	} else if (usage > CRESCENT_STACK_GROWTHRESHOLD) {
		failed = crescentC_growStack(state, usage);
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

	newTopFrame->base     = oldTopFrame->base + oldTopFrame->top;
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

		fromBase = newTopFrame->base + newTopFrame->top - results;
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
