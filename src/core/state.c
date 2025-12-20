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

crs_GState*
crsE_blankGState(void) {
	crs_GState* gState = malloc(sizeof(crs_GState));

	if (gState == NULL) {
		return NULL;
	}

	gState->memoryError   = "out of memory";
	gState->nilValue.type = CRS_TYPE_NIL;
	gState->mainThread    = NULL;
	gState->lastThread    = NULL;
	gState->panic         = NULL;

	return gState;
}

void
crsE_closeGState(crs_GState* gState) {
	if (gState == NULL) {
		return;
	}

	crs_State* current;
	crs_State* next = gState->mainThread;

	while (next != NULL) {
		current = next;
		next    = next->next;

		crsE_closeLState(current);
	}

	free(gState);
}

crs_State*
crsE_blankLState(void) {
	crs_State* state = malloc(sizeof(crs_State) + sizeof(crs_Frame));
	crs_Frame* frame = (crs_Frame*)(state + 1);

	if (state == NULL) {
		return NULL;
	}

	state->stack.size   = CRS_MIN_STACK;
	state->stack.base   = calloc(state->stack.size, sizeof(crs_Object));
	state->stack.top    = state->stack.base;
	state->stack.calls  = 0;
	state->stack.cCalls = 0;
	state->stack.frame  = frame;

	if (state->stack.base == NULL) {
		free(state);

		return NULL;
	}

	frame->base     = state->stack.base;
	frame->top      = CRS_MIN_TOP;
	frame->next     = NULL;
	frame->previous = NULL;

	state->status  = CRS_STATUS_OK;
	state->error   = NULL;
	state->handler = NULL;
	state->next    = NULL;
	state->gState  = NULL;

	return state;
}

void
crsE_closeLState(crs_State* state) {
	if (state == NULL) {
		return;
	}

	crs_Object* object = state->stack.base;

	while (object < state->stack.top) {
		crsO_free(object++);
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
