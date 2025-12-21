/*
 * https://github.com/mochji/crescent
 * core/memory.c
 *
 * Copyright (C) 2025 mochji
 * MIT License
 */

#include <stdlib.h>
#include <stddef.h>

#include "conf.h"
#include "limit.h"

#include "core/state.h"

#include "core/memory.h"

void*
crsM_malloc_(crs_Thread* thread, size_t size) {
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
crsM_realloc_(crs_Thread* thread, void* block, size_t size, size_t oldSize) {
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
crsM_free_(crs_Thread* thread, void* block, size_t size) {
	free(block);
	thread->state->gc.usage -= size;
}
