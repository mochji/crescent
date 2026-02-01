/*
 * https://github.com/mochji/crescent
 * core/memory.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include <stdlib.h>
#include <stddef.h>

#include "conf.h"
#include "limit.h"

#include "core/state.h"
#include "core/call.h"

#include "core/memory.h"

void
crsM_error(crs_Thread* thread) {
	obj_setgc(&thread->error, thread->state->memoryError);
	crsC_throw(thread);
}

void*
crsM_malloc(crs_Thread* thread, size_t size) {
	crs_State* state = thread->state;
	void*      block = malloc(size);

	if (!block) {
		/*
		 * TODO: when the gc is implemented, do a full collection and try
		 * again. if it still fails, then return NULL.
		 */

		return NULL;
	}

	state->gc.usage += size;

	return block;
}

void*
crsM_realloc(crs_Thread* thread, void* block, size_t size, size_t oldSize) {
	crs_State* state = thread->state;

	block = realloc(block, size);

	if (!block) {
		/*
		 * TODO: when the gc is implemented, do a full collection and try
		 * again. if it still fails, then return NULL.
		 */

		return NULL;
	}

	state->gc.usage -= oldSize;
	state->gc.usage += size;

	return block;
}

void
crsM_free(crs_Thread* thread, void* block, size_t size) {
	if (block == NULL) {
		return;
	}

	free(block);
	thread->state->gc.usage -= size;
}
