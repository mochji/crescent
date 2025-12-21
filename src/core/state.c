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

crs_State*
crsE_newState(void) {
	crs_State* state = malloc(sizeof(crs_State));

	if (state == NULL) {
		return NULL;
	}

	state->memoryError   = "out of memory";
	state->nilValue.type = CRS_TYPE_NIL;
	state->thread        = NULL;
	state->panic         = NULL;

	return state;
}

void
crsE_freeState(crs_State* state) {
	if (state == NULL) {
		return;
	}

	crsE_freeThread(state->thread);
	free(state);
}

crs_Thread*
crsE_newThread(crs_State* state) {
	crs_Thread* thread = malloc(sizeof(crs_Thread) + sizeof(crs_Frame));
	crs_Frame*  frame  = (crs_Frame*)(thread + 1);

	if (thread == NULL) {
		return NULL;
	}

	thread->stack.size   = CRS_MIN_STACK;
	thread->stack.base   = calloc(thread->stack.size, sizeof(crs_Object));
	thread->stack.top    = thread->stack.base;
	thread->stack.calls  = 0;
	thread->stack.cCalls = 0;
	thread->stack.frame  = frame;

	if (thread->stack.base == NULL) {
		free(thread);

		return NULL;
	}

	frame->base     = thread->stack.base;
	frame->top      = CRS_MIN_TOP;
	frame->next     = NULL;
	frame->previous = NULL;

	thread->status  = CRS_STATUS_OK;
	thread->error   = NULL;
	thread->handler = NULL;
	thread->state   = state;

	if (state->thread == NULL) {
		state->thread = thread;
	}

	state->thread = thread;

	return thread;
}

void
crsE_freeThread(crs_Thread* thread) {
	if (thread == NULL) {
		return;
	}

	crs_Object* object = thread->stack.base;

	while (object < thread->stack.top) {
		crsO_free(object++);
	}

	if (thread->error != thread->state->memoryError) {
		free(thread->error);
	}

	if (thread->handler != NULL) {
		free(thread->handler);
	}

	free(thread->stack.base);
	free(thread);
}
