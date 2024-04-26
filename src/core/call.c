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
#include <string.h>
#include <limits.h>

#include "conf.h"

#include "core/object.h"
#include "core/state.h"

#include "core/call.h"

void
crescentC_setError(crescent_State* state, char* error) {
	if (state->error != state->memoryError) {
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
		crescentC_throw(gState->baseThread, status);
	}

	crescentC_panic(state);
}

void
crescentC_memoryError(crescent_State* state) {
	state->error = state->memoryError;
	crescentC_throw(state, CRESCENT_STATUS_ERRMEM);
}

int
crescentC_growStack(crescent_State* state, size_t newTop) {
	size_t           absoluteTop = state->stack.topFrame->base + newTop;
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
crescentC_shrinkStack(crescent_State* state, size_t newTop) {
	size_t           absoluteTop = state->stack.topFrame->base + newTop;
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
crescentC_resizeStack(crescent_State* state, size_t newTop, int throw) {
	size_t absoluteTop = state->stack.topFrame->base + newTop;
	int    usage       = (absoluteTop * 100 + state->stack.size / 2) / state->stack.size;
	int    failed;

	if (usage < CRESCENT_STACK_SHRINKTHRESHOLD) {
		if (state->stack.size == CRESCENT_STACK_INITSIZE) {
			return 0;
		}

		failed = crescentC_shrinkStack(state, newTop);

		if (failed && throw) {
			crescentC_memoryError(state);
		}

		return failed;
	}

	if (usage > CRESCENT_STACK_GROWTHRESHOLD) {
		failed = crescentC_growStack(state, newTop);

		if (failed && throw) {
			crescentC_memoryError(state);
		}

		return failed;
	}

	return 0;
}

void
crescentC_startCall(crescent_State* state, size_t argCount) {
	crescent_Frame* newTopFrame;
	crescent_Frame* oldTopFrame;

	if (state->stack.frameCount >= state->stack.maxFrames) {
		crescentC_setError(state, "stack overflow");
		crescentC_throw(state, CRESCENT_STATUS_ERROR);
	}

	newTopFrame = malloc(sizeof(crescent_Frame));
	oldTopFrame = state->stack.topFrame;

	if (newTopFrame == NULL) {
		crescentC_memoryError(state);
	}

	if (argCount > oldTopFrame->top) {
		argCount = oldTopFrame->top;
	}

	newTopFrame->base     = oldTopFrame->base + oldTopFrame->top - argCount;
	newTopFrame->top      = argCount;
	newTopFrame->next     = NULL;
	newTopFrame->previous = oldTopFrame;

	oldTopFrame->top -= argCount;
	oldTopFrame->next = newTopFrame;

	state->stack.frameCount                         += 1;
	state->stack.frames[state->stack.frameCount - 1] = newTopFrame;
	state->stack.topFrame                            = newTopFrame;
}

void
crescentC_endCall(crescent_State* state, size_t results) {
	crescent_Frame* newTopFrame = state->stack.topFrame;
	crescent_Frame* oldTopFrame = newTopFrame->previous;

	if ((size_t)results > newTopFrame->top) {
		if (newTopFrame->top > INT_MAX) {
			results = INT_MAX;
		} else {
			results = newTopFrame->top;
		}
	}

	if (results != newTopFrame->top) {
		size_t fromBaseIndex;
		size_t toBaseIndex;

		toBaseIndex = newTopFrame->base;

		for (size_t a = 0; a < newTopFrame->top - results; a++) {
			crescentO_free(&state->stack.data[toBaseIndex + a]);
		}

		fromBaseIndex = newTopFrame->base + newTopFrame->top - results;
		toBaseIndex   = newTopFrame->base;

		for (size_t a = 0; a < results; a++) {
			state->stack.data[toBaseIndex + a] = state->stack.data[fromBaseIndex + a];
		}
	}

	free(newTopFrame);

	oldTopFrame->top += results;
	oldTopFrame->next = NULL;

	state->stack.frameCount                     -= 1;
	state->stack.frames[state->stack.frameCount] = NULL;
	state->stack.topFrame                        = oldTopFrame;
}

int
crescentC_callC(crescent_State* state, crescent_CFunction* function, size_t argCount, int maxResults) {
	if (maxResults < 0) {
		maxResults = 0;
	}

	crescentC_startCall(state, argCount);

	int results = function(state);

	if (results < 0) {
		results = 0;
	} else if (results > maxResults) {
		results = maxResults;
	}

	crescentC_endCall(state, (size_t)results);
	crescentC_resizeStack(state, state->stack.topFrame->top, 1);

	return results;
}

int
crescentC_pCallC(crescent_State* state, crescent_CFunction* function, size_t argCount, int maxResults, int* status) {
	crescent_ErrorJump* oldErrorJump  = state->errorJump;
	crescent_ErrorJump  newErrorJump  = {.status = CRESCENT_STATUS_OK};
	size_t              oldFrameIndex = state->stack.frameCount - 1;
	int                 results;

	state->errorJump = &newErrorJump;

	if (setjmp(newErrorJump.buffer) == 0) {
		results = crescentC_callC(state, function, argCount, maxResults);
	} else {
		state->errorJump = oldErrorJump; /* FIXME: this line is just a hack to get it to work */

		for (size_t a = oldFrameIndex; a < state->stack.frameCount; a++) {
			crescentC_endCall(state, 0);
		}

		if (crescentC_resizeStack(state, state->stack.topFrame->top, 1)) {
			state->error = "error in error handling";
			crescentC_throw(state, CRESCENT_STATUS_ERRERR);
		}

		results = 0;
	}

	if (status != NULL) {
		*status = newErrorJump.status;
	}

	state->errorJump = oldErrorJump;

	return results;
}
