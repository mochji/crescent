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
	gState->panic         = NULL;

	return gState;
}

void
crescentG_closeGState(crescent_GState* gState) {
	if (gState == NULL) {
		return;
	}

	if (gState->baseThread) {
		crescent_State* next = gState->baseThread;
		crescent_State* current;

		while (next != NULL) {
			current = next;
			next    = next->next;

			crescentG_closeLState(current);
		}
	}

	free(gState);
}

crescent_State*
crescentG_blankLState(void) {
	crescent_State* state = malloc(sizeof(crescent_State) + sizeof(crescent_Frame));

	if (state == NULL) {
		return NULL;
	}

	state->stack.size     = CRESCENT_STACK_INITSIZE;
	state->stack.data     = calloc(state->stack.size, sizeof(crescent_Object));
	state->stack.calls    = 0;
	state->stack.cCalls   = 0;
	state->stack.topFrame = (crescent_Frame*)(state + 1);

	if (state->stack.data == NULL) {
		free(state);

		return NULL;
	}

	state->stack.topFrame->base     = state->stack.data;
	state->stack.topFrame->top      = 0;
	state->stack.topFrame->next     = NULL;
	state->stack.topFrame->previous = NULL;

	state->error       = NULL;
	state->errorJump   = NULL;
	state->next        = NULL;
	state->gState      = NULL;

	return state;
}

void
crescentG_closeLState(crescent_State* state) {
	if (state == NULL) {
		return;
	}

	crescent_Object* object = state->stack.data;

	for (; object < object + state_abstop(state); object++) {
		crescentO_free(object);
	}

	if (state->error != state->gState->memoryError) {
		free(state->error);
	}

	free(state->stack.data);
	free(state);
}
