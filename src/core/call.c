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

int
crescentC_setError(crescent_State* state, crescent_Status status) {
	size_t index = state->stack.topFrame->base + state->stack.topFrame->top;

	switch (status) {
		case CRESCENT_STATUS_ERRMEM:
			state->stack.topFrame->top      += 1;
			state->stack.data[index].type    = CRESCENT_TYPE_STRING;
			state->stack.data[index].value.s = state->gState->memoryErrorMsg;

			break;
		case CRESCENT_STATUS_ERRC:
			state->stack.topFrame->top      += 1;
			state->stack.data[index].type    = CRESCENT_TYPE_STRING;
			state->stack.data[index].value.s = malloc(sizeof(crescent_String));

			if (state->stack.data[index].value.s == NULL) {
				crescentC_throw(state, CRESCENT_STATUS_ERRMEM);
			}
		case CRESCENT_STATUS_ERROR:
			break;
		default:
			return 1;
	}

	return 0;
}

void
crescentC_throw(crescent_State* state, crescent_Status status) {
	crescent_GState* gState = state->gState;

	if (crescentC_setError(state, status)) {
		return;
	}

	if (state->errorJump != NULL) {
		state->errorJump->status = status;
		longjmp(state->errorJump->buffer, 1);
	}

	if (gState->panic != NULL) {
		(*gState->panic)(state);
	}

	abort();
}

inline int
crescentC_stackUsage(crescent_State* state) {
	size_t absoluteTop = state->stack.topFrame->base - state->stack.topFrame->top;

	return (absoluteTop * 100 + state->stack.size / 2) / state->stack.size;
}

inline size_t
crescentC_absoluteIndex(crescent_State* state, size_t index) {
	return state->stack.topFrame->base + index - 1;
}

void
crescentC_resizeStack(crescent_State* state, size_t newTop) {
	size_t absoluteTop = state->stack.topFrame->base - state->stack.topFrame->top;

	if (state->stack.size >= absoluteTop) {
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

	size_t           newSize;
	crescent_Object* newData;

	if (absoluteTop >= state->stack.size) {
		newSize = state->stack.size * 2;
	} else {
		newSize = state->stack.size / 2;
	}

	newData = realloc(state->stack.data, newSize);

	if (newData == NULL) {
		crescentC_throw(state, CRESCENT_STATUS_ERRMEM);
	}

	state->stack.size          = newSize;
	state->stack.data          = newData;
	state->stack.topFrame->top = newTop;
}

void
crescentC_pushFrame(crescent_State* state) {
	if (state->stack.frameCount == state->stack.maxFrames) {
		size_t           newMaxFrames = state->stack.maxFrames + 4;
		crescent_Frame** newFrames    = realloc(state->stack.frames, newMaxFrames * sizeof(crescent_Frame));

		if (newFrames == NULL) {
			crescentC_throw(state, CRESCENT_STATUS_ERRMEM);
		}

		state->stack.maxFrames = newMaxFrames;
		state->stack.frames    = newFrames;
	}

	crescent_Frame* topFrame  = malloc(sizeof(crescent_Frame));
	crescent_Frame* lastFrame = state->stack.topFrame;

	if (topFrame == NULL) {
		crescentC_throw(state, CRESCENT_STATUS_ERRMEM);
	}

	if (lastFrame != NULL) {
		lastFrame->next = topFrame;
	}

	if (lastFrame != NULL) {
		topFrame->base     = lastFrame->top + 1;
	}

	topFrame->top      = 0;
	topFrame->next     = NULL;
	topFrame->previous = lastFrame;

	state->stack.frameCount += 1;
}

void
crescentC_popFrame(crescent_State* state) {
	if (state->stack.frameCount <= 1) {
		crescentC_throw(state, CRESCENT_STATUS_ERRMEM);
	}

	free(state->stack.topFrame);
	state->stack.frames[state->stack.frameCount - 1] = NULL;

	crescent_Frame* newTopFrame = state->stack.frames[state->stack.frameCount - 2];

	newTopFrame->next = NULL;

	state->stack.frameCount -= 1;
	state->stack.topFrame    = newTopFrame;

	if (state->stack.maxFrames - state->stack.frameCount >= 4) {
		if (state->stack.maxFrames == 4) {
			return;
		}

		size_t           newMaxFrames = state->stack.maxFrames - 4;
		crescent_Frame** newFrames    = realloc(state->stack.frames, newMaxFrames * sizeof(crescent_Frame));

		if (newFrames == NULL) {
			crescentC_throw(state, CRESCENT_STATUS_ERRMEM);
		}

		state->stack.maxFrames = newMaxFrames;
		state->stack.frames    = newFrames;
	}
}
