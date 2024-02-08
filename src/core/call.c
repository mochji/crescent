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

void
crescentC_throw(crescent_State* state, crescent_Status status) {
	crescent_GState* gState = state->gState;

	if (status == CRESCENT_STATUS_OK) {
		return;
	}

	if (state->errorJump != NULL) {
		state->errorJump->status = status;
		longjmp(state->errorJump->buffer, 1);
	}

	if (gState->baseThread->errorJump != NULL) {
		crescentC_throw(gState->baseThread, status);
	}

	if (gState->panic != NULL) {
		gState->panic(state);
	}

	abort();
}

inline void
crescentC_memoryError(crescent_State* state) {
	state->error = ((char*)state + sizeof(crescent_State) + sizeof(crescent_ErrorJump));

	crescentC_throw(state, CRESCENT_STATUS_ERRMEM);
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
	if (state->stack.maxFrames >= state->stack.frameCount) {
		size_t           newMaxFrames = state->stack.frameCount + 4;
		crescent_Frame** newFrames    = realloc(state->stack.frames, newMaxFrames * sizeof(crescent_Frame*));

		if (newFrames == NULL) {
			crescentC_memoryError(state);
		}

		state->stack.maxFrames = newMaxFrames;
		state->stack.frames    = newFrames;
	}

	crescent_Frame* newTopFrame = malloc(sizeof(crescent_Frame));
	crescent_Frame* oldTopFrame = state->stack.topFrame;

	if (newTopFrame == NULL) {
		crescentC_memoryError(state);
	}

	newTopFrame->base     = oldTopFrame->top + 1;
	newTopFrame->top      = 0;
	newTopFrame->next     = NULL;
	newTopFrame->previous = oldTopFrame;

	oldTopFrame->next = newTopFrame;

	state->stack.frameCount                         += 1;
	state->stack.frames[state->stack.frameCount - 1] = newTopFrame;
	state->stack.topFrame                            = newTopFrame;
}

void
crescentC_popFrame(crescent_State* state) {
	if (state->stack.frameCount <= 1) {
		return;
	}

	crescent_Frame* newTopFrame = state->stack.frames[state->stack.frameCount - 2];

	free(state->stack.topFrame);

	newTopFrame->next = NULL;

	state->stack.frameCount -= 1;
	state->stack.topFrame    = newTopFrame;

	if (state->stack.maxFrames - state->stack.frameCount >= 4) {
		size_t           newMaxFrames = state->stack.frameCount - 4;
		crescent_Frame** newFrames    = realloc(state->stack.frames, newMaxFrames * sizeof(crescent_Frame*));

		if (newFrames == NULL) {
			crescentC_memoryError(state);
		}

		state->stack.maxFrames = newMaxFrames;
		state->stack.frames    = newFrames;
	}
}
