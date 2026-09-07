/*
 * https://github.com/mochji/crescent
 * core/state.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include <stdlib.h>
#include <stddef.h>
#include <setjmp.h>

#include "crescent/conf.h"
#include "limit.h"

#include "types/string.h"
#include "types/table.h"
#include "core/object.h"
#include "core/memory.h"
#include "core/call.h"
#include "core/gc.h"

#include "core/state.h"

static int initThread(crs_State* state, crs_Thread* thread) {
    crs_Object* stack = malloc(CRS_MIN_STACK * sizeof(crs_Object));
    crs_Frame*  frame = &thread->stack.baseFrame;

    if (stack == NULL) {
        return 1;
    }

    thread->stack.size   = CRS_MIN_STACK;
    thread->stack.base   = stack;
    thread->stack.top    = stack;
    thread->stack.level  = 0;
    thread->stack.frame  = frame;

    frame->base     = stack;
    frame->top      = CRS_MIN_TOP;
    frame->previous = NULL;
    frame->flags    = 0;
    frame->i.c.c    = NULL;

    thread->handler = NULL;
    thread->state   = state;
    obj_setn(&thread->error);

    state->gc.usage += CRS_MIN_STACK * sizeof(crs_Object);
    crsG_add(thread, thread, CRS_TYPE_THREAD);

    return 0;
}

static void* initState(crs_Thread* thread, void* data) {
    crs_State* state = (crs_State*)data;

    state->memoryError = crsS_new(thread, "out of memory");
    crsG_setImmune(thread);

    crs_Table* globals = crsT_new(thread);
    obj_setgc(&state->globals, globals);

    return NULL;
}

crs_Thread* crsE_open(void) {
    crs_State*  state = malloc(sizeof(crs_State));
    crs_Thread* thread;

    if (state == NULL) {
        return NULL;
    }

    thread = &state->thread;

    /* basic initialization */

    crsG_init(state);
    state->memoryError = NULL;
    state->panic       = NULL;

    if (initThread(state, thread)) {
        free(state);
        return NULL;
    }

    for (int i = 0; i < CRS_STRCACHE_SIZE; i++) {
        for (int j = 0; j < CRS_STRCACHE_BUCKETS; j++) {
            state->strings[i][j] = NULL;
        }
    }

    /* special objects */

    if (crsC_try(thread, &initState, state, NULL) != CRS_OK) {
        crsE_close(state);
        return NULL;
    }

    return thread;
}

void crsE_close(crs_State* state) {
    crsG_freeAll(state);
    free(state->thread.stack.base);
    free(state);
}

void crsE_freeThread(crs_Thread* thread) {
    crs_Frame* frame = thread->stack.frame;

    while (frame->previous != NULL) {
        crs_Frame* next = frame->previous;

        mem_free(thread, frame);
        frame = next;
    }

    mem_vfree(thread, thread->stack.base, thread->stack.size);
    mem_free(thread, thread);
}
