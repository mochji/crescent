/*
 * https://github.com/mochji/crescent
 * core/state.c
 *
 * idrk what this is
 * Copyright (C) 2024 mochji
 *
 * MIT License
 */

#include <stdlib.h>
#include <stddef.h>
#include <setjmp.h>

#include "conf.h"

#include "core/object.h"

#include "core/state.h"

crescent_GState*
crescentG_blankGState(void) {
	crescent_GState* gState = malloc(sizeof(crescent_GState));

	if (gState == NULL) {
		return NULL;
	}

	gState->memoryError   = "out of memory";
	gState->nilValue.type = CRESCENT_TYPE_NIL;
	gState->maxThreads    = 1;
	gState->threadCount   = 0;
	gState->threads       = calloc(1, sizeof(crescent_State*));
	gState->baseThread    = NULL;
	gState->panic         = NULL;

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
crescentG_blankLState(void) {
	crescent_State* state = malloc(sizeof(crescent_State) + sizeof(crescent_Frame));

	if (state == NULL) {
		return NULL;
	}

	state->stack.size = CRESCENT_STACK_INITSIZE;
	state->stack.data = calloc(state->stack.size, sizeof(crescent_Object));

	if (state->stack.data == NULL) {
		free(state);

		return NULL;
	}

	state->stack.frames    = 1;
	state->stack.topFrame  = (crescent_Frame*)(state + 1);

	state->stack.topFrame->base     = 0;
	state->stack.topFrame->top      = 0;
	state->stack.topFrame->next     = NULL;
	state->stack.topFrame->previous = NULL;

	state->error       = NULL;
	state->errorJump   = NULL;
	state->threadIndex = 0;
	state->gState      = NULL;

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

	if (state->error != state->gState->memoryError) {
		free(state->error);
	}

	free(state->stack.data);
	free(state);
}
