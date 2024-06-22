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

#include "conf.h"
#include "limit.h"

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
	gState->baseThread    = NULL;
	gState->lastThread    = NULL;
	gState->panic         = NULL;

	return gState;
}

void
crescentG_closeGState(crescent_GState* gState) {
	if (gState == NULL) {
		return;
	}

	crescent_State* current;
	crescent_State* next = gState->baseThread;

	while (next != NULL) {
		current = next;
		next    = next->next;

		crescentG_closeLState(current);
	}

	free(gState);
}

crescent_State*
crescentG_blankLState(void) {
	crescent_State* state = malloc(sizeof(crescent_State) + sizeof(crescent_Frame));
	crescent_Frame* frame = (crescent_Frame*)(state + 1);

	if (state == NULL) {
		return NULL;
	}

	state->stack.size     = CRESCENT_MIN_STACK;
	state->stack.base     = calloc(state->stack.size, sizeof(crescent_Object));
	state->stack.top      = state->stack.base;
	state->stack.calls    = 0;
	state->stack.cCalls   = 0;
	state->stack.topFrame = frame;

	if (state->stack.base == NULL) {
		free(state);

		return NULL;
	}

	frame->base     = state->stack.base;
	frame->top      = 0;
	frame->next     = NULL;
	frame->previous = NULL;

	state->error     = NULL;
	state->errorJump = NULL;
	state->next      = NULL;
	state->gState    = NULL;

	return state;
}

void
crescentG_closeLState(crescent_State* state) {
	if (state == NULL) {
		return;
	}

	crescent_Object* object = state->stack.base;

	for (; object < state->stack.top; object++) {
		crescentO_free(object);
	}

	if (state->error != state->gState->memoryError) {
		free(state->error);
	}

	free(state->stack.base);
	free(state);
}

void
crescentG_connectThread(crescent_GState* gState, crescent_State* state) {
	if (gState->lastThread == NULL) {
		gState->baseThread = state;
		gState->lastThread = state;

		state->gState = gState;
	} else {
		gState->lastThread->next = state;
		gState->lastThread       = state;

		state->gState = gState;
	}
}
