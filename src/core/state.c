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

	gState->threadCount = 0;
	gState->maxThreads  = 4;
	gState->threads     = malloc(4 * sizeof(crescent_Frame*));
	gState->baseThread  = NULL;
	gState->panic       = NULL;

	if (gState->threads == NULL) {
		free(gState);

		return NULL;
	}

	return gState;
}

void
crescentG_closeGState(crescent_GState* gState) {
	if (gState == NULL) {
		return;
	}

	for (size_t a = 0; a < gState->threadCount; a++) {
		crescentG_closeLState(gState->threads[a]);
	}

	free(gState->threads);
	free(gState);
}

crescent_State*
crescentG_blankLState() {
	crescent_State* state = malloc(sizeof(crescent_State) + 14);

	if (state == NULL) {
		return NULL;
	}

	state->stack.size = CRESCENT_STACK_INITSIZE;
	state->stack.data = calloc(state->stack.size, sizeof(crescent_Object));

	if (state->stack.data == NULL) {
		free(state);

		return NULL;
	}

	state->stack.frameCount = 1;
	state->stack.maxFrames  = 256;
	state->stack.frames     = calloc(state->stack.maxFrames, sizeof(crescent_Frame*));
	state->stack.topFrame   = NULL;

	if (state->stack.frames == NULL) {
		free(state->stack.data);
		free(state);

		return NULL;
	}

	state->stack.frames[0] = malloc(sizeof(crescent_Frame));
	state->stack.topFrame  = state->stack.frames[0];

	if (state->stack.topFrame == NULL) {
		free(state->stack.frames);
		free(state->stack.data);
		free(state);

		return NULL;
	}

	state->stack.topFrame->base     = 0;
	state->stack.topFrame->top      = 0;
	state->stack.topFrame->next     = NULL;
	state->stack.topFrame->previous = NULL;

	state->threadIndex = 0;
	state->error       = NULL;
	state->errorJump   = NULL;
	state->gState      = NULL;

	/*
	 * Why does each LState have its own pre-allocated out of memory message?
	 *
	 * I wanted to put it into the GState, and then have the LState take it
	 * from there. Checking if its the preallocated GState message was easy,
	 * (checking if part of gstate structure with pointer arithmetic) but the
	 * way this code is written, closeLState can only access the LState since
	 * this is a core function.
	 */

	const char* memoryErrorMsg = "out of memory";
	char*       memoryError    = (char*)state + sizeof(crescent_State);

	for (size_t a = 0; a < 14; a++) {
		memoryError[a] = memoryErrorMsg[a];
	}

	return state;
}

void
crescentG_closeLState(crescent_State* state) {
	if (state == NULL) {
		return;
	}

	for (size_t a = 0; a < state->stack.topFrame->base + state->stack.topFrame->top; a++) {
		crescentO_free(&state->stack.data[a]);
	}

	for (size_t a = 0; a < state->stack.frameCount; a++) {
		free(state->stack.frames[a]);
	}

	size_t stateSize  = sizeof(crescent_State) + 14;
	char*  endOfState = (char*)state + stateSize;

	if (state->error < (char*)state || state->error >= endOfState) {
		free(state->error);
	}

	free(state->stack.frames);
	free(state->stack.data);
	free(state->errorJump);
	free(state);
}
