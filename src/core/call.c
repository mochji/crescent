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
crescentC_throw(crescent_State* state, crescent_Status status) {
	crescent_GState* gState = state->gState;

	if (state->errorJump != NULL) {
		state->errorJump->status = status;
		longjmp(state->errorJump->buffer, 1);
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
crescentC_resizeStack(crescent_State* state, size_t newTop) {
	size_t absoluteTop = state->stack.topFrame->base + newTop;

	if (state->stack.size >= newTop) {
		if (state->stack.size == CRESCENT_CONF_STACK_INITSIZE) {
			return;
		}

		size_t usage = (absoluteTop * 100 + state->stack.size / 2) / state->stack.size;

		if (usage >= CRESCENT_CONF_STACK_SHRINKTHRESHOLD) {
			return;
		}

		if (usage <= CRESCENT_CONF_STACK_GROWTHRESHOLD) {
			return;
		}
	}

	size_t           newSize = state->stack.size;
	crescent_Object* newData;

	if (newTop >= state->stack.size) {
		while ((absoluteTop * 100 + state->stack.size / 2) / state->stack.size > CRESCENT_CONF_STACK_GROWTHRESHOLD) {
			newSize *= 2;
		}
	} else {
		while ((absoluteTop * 100 + state->stack.size / 2) / state->stack.size < CRESCENT_CONF_STACK_SHRINKTHRESHOLD) {
			newSize /= 2;
		}
	}

	newData = realloc(state->stack.data, newSize * sizeof(crescent_Object));

	if (newData == NULL) {
		crescentC_memoryError(state);
	}

	state->stack.size = newSize;
	state->stack.data = newData;
}

size_t
crescentC_callC(crescent_State* state, int (*function)(crescent_State*)) {
	crescent_Frame* newTopFrame = malloc(sizeof(crescent_Frame));
	crescent_Frame* oldTopFrame = state->stack.topFrame;
	size_t          results;

	if (newTopFrame == NULL) {
		crescentC_memoryError(state);
	}

	newTopFrame->base     = oldTopFrame->top;
	newTopFrame->top      = 0;
	newTopFrame->next     = NULL;
	newTopFrame->previous = oldTopFrame;

	oldTopFrame->next = newTopFrame;

	if (state->stack.maxFrames <= state->stack.frameCount) {
		size_t           newMaxFrames = state->stack.frameCount + 4;
		crescent_Frame** newFrames    = realloc(state->stack.frames, newMaxFrames * sizeof(crescent_Frame*));

		if (newFrames == NULL) {
			crescentC_memoryError(state);
		}

		state->stack.maxFrames = newMaxFrames;
		state->stack.frames    = newFrames;
	}

	state->stack.frameCount                         += 1;
	state->stack.frames[state->stack.frameCount - 1] = newTopFrame;
	state->stack.topFrame                            = newTopFrame;

	results = function(state);

	free(newTopFrame);

	state->stack.frameCount                     -= 1;
	state->stack.frames[state->stack.frameCount] = NULL;
	state->stack.topFrame                        = oldTopFrame;

	oldTopFrame->next = NULL;

	if (state->stack.maxFrames - state->stack.frameCount >= 4) {
		size_t           newMaxFrames = state->stack.frameCount + 4;
		crescent_Frame** newFrames    = realloc(state->stack.frames, newMaxFrames * sizeof(crescent_Frame*));

		if (newFrames == NULL) {
			crescentC_memoryError(state);
		}

		state->stack.maxFrames = newMaxFrames;
		state->stack.frames    = newFrames;
	}

	return results;
}

size_t
crescentC_pCallC(crescent_State* state, int (*function)(crescent_State*), crescent_Status* status) {
	crescent_ErrorJump* oldErrorJump = NULL;
	size_t              results;

	if (state->errorJump != NULL) {
		oldErrorJump = state->errorJump;
	}

	state->errorJump = malloc(sizeof(crescent_ErrorJump));

	if (state->errorJump == NULL) {
		crescentC_memoryError(state);
	}

	state->errorJump->status = CRESCENT_STATUS_OK;

	if (setjmp(state->errorJump->buffer) == 0) {
		crescentC_callC(state, function);
	} else {
		*status = state->errorJump->status;

		return 0;
	}

	*status = state->errorJump->status;

	free(state->errorJump);

	if (oldErrorJump != NULL) {
		state->errorJump = oldErrorJump;
	} else {
		state->errorJump = NULL;
	}

	return results;
}
