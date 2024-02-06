/*
 * https://github.com/mochji/crescent
 * core/state.c
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

crescent_GState*
crescentG_blankGState() {
	crescent_GState* gState = malloc(sizeof(crescent_GState));

	if (gState == NULL) {
		return NULL;
	}

	gState->threadCount    = 0;
	gState->maxThreads     = 4;
	gState->threads        = malloc(4 * sizeof(crescent_Frame*));
	gState->baseThread     = NULL;
	gState->panic          = NULL;
	gState->memoryErrorMsg = malloc(sizeof(crescent_String));

	if (gState->memoryErrorMsg == NULL || gState->threads == NULL) {
		free(gState->memoryErrorMsg);
		free(gState->threads);
		free(gState);

		return NULL;
	}

	gState->memoryErrorMsg->size   = 13;
	gState->memoryErrorMsg->length = 13;
	gState->memoryErrorMsg->data   = "out of memory";

	return gState;
}

void
crescentG_closeGState(crescent_GState* gState) {
	crescent_State* currentState;

	for (size_t a = 0; a < gState->threadCount; a++) {
		currentState = gState->threads[a];

		if (currentState == NULL) {
			continue;
		}

		for (size_t b = 0; b < currentState->stack.frameCount; b++) {
			free(currentState->stack.frames[b]);
		}

		free(currentState->stack.frames);
		free(currentState->stack.data);
		free(currentState);
	}

	free(gState->memoryErrorMsg);
	free(gState->threads);
	free(gState);
}

crescent_State*
crescentG_blankLState() {
	crescent_State* state = malloc(sizeof(crescent_State) + sizeof(crescent_ErrorJump));

	if (state == NULL) {
		return NULL;
	}

	state->stack.size = CRESCENT_CONF_STACK_INITSIZE;
	state->stack.data = calloc(state->stack.size, sizeof(crescent_Object));

	if (state->stack.data == NULL) {
		free(state);

		return NULL;
	}

	state->stack.frameCount = 0;
	state->stack.maxFrames  = 4;
	state->stack.frames     = calloc(4, sizeof(crescent_Frame*));
	state->stack.topFrame   = NULL;

	if (state->stack.frames == NULL) {
		free(state->stack.data);
		free(state);

		return NULL;
	}

	state->stack.topFrame->base     = 0;
	state->stack.topFrame->top      = 0;
	state->stack.topFrame->next     = NULL;
	state->stack.topFrame->previous = NULL;

	state->threadIndex = 0;
	state->errorJump   = NULL;
	state->gState      = NULL;

	return state;
}

void
crescentG_closeLState(crescent_State* state) {
	for (size_t a = 0; a < state->stack.frameCount; a++) {
		if (state->stack.frames[a] == NULL) {
			continue;
		}

		free(state->stack.frames[a]);
	}

	free(state->stack.frames);
	free(state->stack.data);
	free(state);
}
