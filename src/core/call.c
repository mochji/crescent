/*
 * https://github.com/mochji/crescent
 * core/call.c
 *
 * idrk what this is
 * Copyright (C) 2024 mochji
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stddef.h>
#include <setjmp.h>

#include "conf.h"

#include "core/object.h"
#include "core/state.h"

#include "core/call.h"

void
crescentC_setError(crescent_State* state, char* error) {
	char*  stateError;
	size_t length = 0;

	while (error[length++]) {}

	stateError = malloc(length);

	if (stateError == NULL) {
		crescentC_memoryError(state);
	}

	for (size_t a = 0; a < length; a++) {
		stateError[a] = error[a];
	}

	state->error = stateError;
}

void
crescentC_throw(crescent_State* state, int status) {
	crescent_GState* gState = state->gState;

	/* TODO: find a better solution than goto, trying to call again causes -Winfinite-recursion */

throwAgain:

	if (state->errorJump != NULL) {
		state->errorJump->status = status;
		longjmp(state->errorJump->buffer, 1);
	}

	if (gState->baseThread->errorJump != NULL) {
		state = gState->baseThread;

		goto throwAgain;
	}

	if (gState->panic != NULL) {
		gState->panic(state);
	}

	abort();
}

void
crescentC_memoryError(crescent_State* state) {
	state->error = (char*)state + sizeof(crescent_State);

	crescentC_throw(state, CRESCENT_STATUS_NOMEM);
}

int
crescentC_stackUsage(crescent_State* state) {
	size_t absoluteTop = state->stack.topFrame->base + state->stack.topFrame->top;

	return (absoluteTop * 100 + state->stack.size / 2) / state->stack.size;
}

void
crescentC_growStack(crescent_State* state, size_t newTop) {
	size_t           absoluteTop = state->stack.topFrame->base + newTop;
	size_t           newSize     = state->stack.size;
	crescent_Object* newData;

	int usage = (absoluteTop * 100 + newSize / 2) / newSize;

	while (usage > CRESCENT_STACK_GROWTHRESHOLD) {
		newSize *= 2;

		usage = (absoluteTop * 100 + newSize / 2) / newSize;
	}

	newData = realloc(state->stack.data, newSize * sizeof(crescent_Object));

	if (newData == NULL) {
		crescentC_memoryError(state);
	}

	state->stack.size = newSize;
	state->stack.data = newData;
}

void
crescentC_shrinkStack(crescent_State* state, size_t newTop) {
	size_t           absoluteTop = state->stack.topFrame->base + newTop;
	size_t           newSize     = state->stack.size;
	crescent_Object* newData;

	int usage = (absoluteTop * 100 + newSize / 2) / newSize;

	while (usage < CRESCENT_STACK_SHRINKTHRESHOLD && newSize > CRESCENT_STACK_INITSIZE) {
		newSize /= 2;

		usage = (absoluteTop * 100 + newSize / 2) / newSize;
	}

	newData = realloc(state->stack.data, newSize * sizeof(crescent_Object));

	if (newData == NULL) {
		crescentC_memoryError(state);
	}

	state->stack.size = newSize;
	state->stack.data = newData;
}

void
crescentC_resizeStack(crescent_State* state, size_t newTop) {
	size_t absoluteTop = state->stack.topFrame->base + newTop;
	int    usage       = (absoluteTop * 100 + state->stack.size / 2) / state->stack.size;

	if (usage < CRESCENT_STACK_SHRINKTHRESHOLD) {
		if (state->stack.size == CRESCENT_STACK_INITSIZE) {
			return;
		}

		crescentC_shrinkStack(state, newTop);

		return;
	}

	if (usage > CRESCENT_STACK_GROWTHRESHOLD) {
		crescentC_growStack(state, newTop);

		return;
	}
}

int
crescentC_callC(crescent_State* state, crescent_CFunction* function, size_t argCount, int maxResults) {
	crescent_Frame*  newTopFrame;
	crescent_Frame*  oldTopFrame;

	if (state->stack.frameCount >= state->stack.maxFrames) {
		crescentC_setError(state, "stack overflow");
		crescentC_throw(state, CRESCENT_STATUS_ERROR);
	}

	newTopFrame = malloc(sizeof(crescent_Frame));
	oldTopFrame = state->stack.topFrame;

	if (newTopFrame == NULL) {
		crescentC_memoryError(state);
	}

	newTopFrame->base     = oldTopFrame->base + oldTopFrame->top;
	newTopFrame->top      = argCount;
	newTopFrame->next     = NULL;
	newTopFrame->previous = oldTopFrame;

	state->stack.frameCount                         += 1;
	state->stack.frames[state->stack.frameCount - 1] = newTopFrame;
	state->stack.topFrame                            = newTopFrame;

	crescentC_resizeStack(state, argCount);

	size_t fromBaseIndex = oldTopFrame->base + oldTopFrame->top - argCount;
	size_t toBaseIndex   = newTopFrame->base;

	crescent_Object* stack = state->stack.data;

	for (size_t a = 0; a < argCount; a++) {
		stack[toBaseIndex + a] = stack[fromBaseIndex + a];
	}

	int results = function(state);

	if (results < 0) {
		results = 0;
	} else if ((size_t)results > newTopFrame->top) {
		results = newTopFrame->top;
	}

	if (results > maxResults) {
		results = maxResults;
	}

	fromBaseIndex = newTopFrame->base + newTopFrame->top - results;
	toBaseIndex   = oldTopFrame->base + oldTopFrame->top;

	for (int a = 0; a < results; a++) {
		stack[toBaseIndex + a] = stack[fromBaseIndex + a];
	}

	toBaseIndex = newTopFrame->base + results;

	for (size_t a = 0; a < newTopFrame->top; a++) {
		stack[toBaseIndex + a].type = CRESCENT_TYPE_NONE;
	}

	free(newTopFrame);

	state->stack.frameCount                     -= 1;
	state->stack.frames[state->stack.frameCount] = NULL;
	state->stack.topFrame                        = oldTopFrame;

	oldTopFrame->next = NULL;
	oldTopFrame->top += results;

	crescentC_resizeStack(state, oldTopFrame->top);

	return results;
}

int
crescentC_pCallC(crescent_State* state, crescent_CFunction* function, size_t argCount, int maxResults, int* status) {
	crescent_ErrorJump* oldErrorJump = NULL;
	int                 results;

	if (state->errorJump != NULL) {
		oldErrorJump = state->errorJump;
	}

	state->errorJump = malloc(sizeof(crescent_ErrorJump));

	if (state->errorJump == NULL) {
		crescentC_memoryError(state);
	}

	state->errorJump->status = CRESCENT_STATUS_OK;

	if (setjmp(state->errorJump->buffer) == 0) {
		results = crescentC_callC(state, function, argCount, maxResults);
	} else {
		results = 0;
	}

	if (status != NULL) {
		*status = state->errorJump->status;
	}

	free(state->errorJump);

	state->errorJump = oldErrorJump;

	return results;
}
