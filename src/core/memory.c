/*
 * https://github.com/mochji/crescent
 * core/memory.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include <stdlib.h>
#include <stddef.h>

#include "crescent/conf.h"
#include "limit.h"

#include "core/state.h"
#include "core/call.h"
#include "core/gc.h"

#include "core/memory.h"

void crsM_error(crs_Thread* thread) {
    crs_String* error = thread->state->memoryError;

    if (error != NULL) {
        obj_setgc(&thread->error, error);
    }

    crsC_throw(thread, CRS_STATUS_MEMERR);
}

static void* tryAgain(crs_Thread* thread, size_t size, void* block) {
    if (gc_getstatus(thread->state, STOPEM)) {
        return NULL;
    }

    crsG_full(thread, 1);

    return block == NULL ? malloc(size) : realloc(block, size);
}

void* crsM_malloc(crs_Thread* thread, size_t size) {
    crs_State* state = thread->state;
    void*      block = malloc(size);

    if (block == NULL) {
        block = tryAgain(thread, size, NULL);

        if (block == NULL) {
            return NULL;
        }
    }

    state->gc.usage += size;

    return block;
}

void* crsM_realloc(crs_Thread* thread, void* block, size_t size,
                                       size_t oldSize) {
    crs_State* state    = thread->state;
    void*      newBlock = realloc(block, size);

    if (newBlock == NULL) {
        newBlock = tryAgain(thread, size, block);

        if (newBlock == NULL) {
            return NULL;
        }
    }

    state->gc.usage -= oldSize;
    state->gc.usage += size;

    return newBlock;
}

void crsM_free(crs_Thread* thread, void* block, size_t size) {
    if (block == NULL) {
        return;
    }

    free(block);
    thread->state->gc.usage -= size;
}
