/*
 * https://github.com/mochji/crescent
 * core/state.c
 *
 * Copyright (C) 2025 mochji
 * MIT License
 */

#include <stdlib.h>
#include <stddef.h>

#include "conf.h"
#include "limit.h"

#include "core/object.h"

#include "core/state.h"

crescent_GState*
crescentE_blankGState(void) {
	crescent_GState* gState = malloc(sizeof(crescent_GState));

	if (gState == NULL) {
		return NULL;
	}

	gState->memoryError   = "out of memory";
	gState->nilValue.type = CRESCENT_TYPE_NIL;
	gState->mainThread    = NULL;
	gState->lastThread    = NULL;
	gState->panic         = NULL;

	return gState;
}

void
crescentE_closeGState(crescent_GState* gState) {
	if (gState == NULL) {
		return;
	}

	crescent_State* current;
	crescent_State* next = gState->mainThread;

	while (next != NULL) {
		current = next;
		next    = next->next;

		crescentE_closeLState(current);
	}

	free(gState);
}

crescent_State*
crescentE_blankLState(void) {
	crescent_State* state = malloc(sizeof(crescent_State) + sizeof(crescent_Frame));
	crescent_Frame* frame = (crescent_Frame*)(state + 1);

	if (state == NULL) {
		return NULL;
	}

	state->stack.size   = CRESCENT_MIN_STACK;
	state->stack.base   = calloc(state->stack.size, sizeof(crescent_Object));
	state->stack.top    = state->stack.base;
	state->stack.calls  = 0;
	state->stack.cCalls = 0;
	state->stack.frame  = frame;

	if (state->stack.base == NULL) {
		free(state);

		return NULL;
	}

	frame->base     = state->stack.base;
	frame->top      = CRESCENT_MIN_TOP;
	frame->next     = NULL;
	frame->previous = NULL;

	state->status  = CRESCENT_STATUS_OK;
	state->error   = NULL;
	state->handler = NULL;
	state->next    = NULL;
	state->gState  = NULL;

	return state;
}

void
crescentE_closeLState(crescent_State* state) {
	if (state == NULL) {
		return;
	}

	crescent_Object* object = state->stack.base;

	while (object < state->stack.top) {
		crescentO_free(object++);
	}

	if (state->error != state->gState->memoryError) {
		free(state->error);
	}

	if (state->handler != NULL) {
		free(state->handler);
	}

	free(state->stack.base);
	free(state);
}
