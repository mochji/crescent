/*
 * https://github.com/mochji/crescent
 * core/state.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include <stdlib.h>
#include <stddef.h>

#include "conf.h"
#include "limit.h"

#include "core/object.h"
#include "core/memory.h"
#include "core/gc.h"

#include "core/state.h"

static crs_State*
newState(void) {
	crs_State* state = malloc(sizeof(crs_State));

	if (state == NULL) {
		return NULL;
	}

	state->nilValue.type = CRS_TYPE_NIL;
	state->thread        = NULL;
	state->panic         = NULL;

	return state;
}

static crs_Thread*
newThread(void) {
	crs_Thread* thread = malloc(sizeof(crs_Thread) + sizeof(crs_Frame));
	crs_Frame*  frame  = (crs_Frame*)(thread + 1);

	if (thread == NULL) {
		return NULL;
	}

	thread->stack.size   = CRS_MIN_STACK;
	thread->stack.base   = calloc(thread->stack.size, sizeof(crs_Object));
	thread->stack.top    = thread->stack.base;
	thread->stack.calls  = 1;
	thread->stack.cCalls = 1;
	thread->stack.frame  = frame;

	if (thread->stack.base == NULL) {
		free(thread);

		return NULL;
	}

	frame->base     = thread->stack.base;
	frame->top      = CRS_MIN_TOP;
	frame->previous = NULL;

	thread->handler    = NULL;
	thread->error.type = CRS_TYPE_NIL;
	thread->state      = NULL;

	return thread;
}

crs_Thread*
crsE_open(void) {
	crs_State*  state = newState();
	crs_Thread* thread;

	if (state == NULL) {
		return NULL;
	}

	thread = newThread();

	if (thread == NULL) {
		free(state);

		return NULL;
	}

	state->thread = thread;
	thread->state = state;

	if (crsG_init(state)) {
		crsE_freeState(state);

		return NULL;
	}

	return thread;
}

void
crsE_freeThread(crs_Thread* thread) {
	mem_vfree(thread, thread->stack.base, thread->stack.size, crs_Object);
	mem_dealloc(thread, thread, sizeof(crs_Thread) + sizeof(crs_Frame));
}

void
crsE_freeState(crs_State* state) {
	crsG_freeAll(state);
	crsE_freeThread(state->thread);
	free(state);
}
