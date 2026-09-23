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
    thread->stack.last   = stack + CRS_MIN_STACK;
    thread->stack.top    = stack;
    thread->stack.level  = 0;
    thread->stack.frame  = frame;

    frame->base     = stack;
    frame->top      = CRS_MIN_TOP;
    frame->previous = NULL;
    frame->flags    = 0;

    thread->jump    = NULL;
    thread->state   = state;
    obj_setn(&thread->error);

    state->gc.usage += CRS_MIN_STACK * sizeof(crs_Object);
    crsG_add(thread, thread, CRS_TYPE_THREAD);

    return 0;
}

static void* initState(crs_Thread* thread, void* data) {
    crs_State* state   = (crs_State*)data;
    crs_Table* globals = crsT_new(thread);
    obj_setgc(&state->globals, globals);

    crsS_init(thread);
    crsM_init(thread);

    return NULL;
}

/* create a new state and main thread */
crs_Thread* crsE_open(void) {
    crs_State*  state = malloc(sizeof(crs_State));
    crs_Thread* thread;

    if (state == NULL) {
        return NULL;
    }

    crsG_init(state);
    state->debug.warnF = NULL;
    state->memoryError = NULL;
    state->panic       = NULL;
    thread             = &state->thread;

    /* init main thread */
    if (initThread(state, thread)) {
        free(state);
        return NULL;
    }

    /* create special objects */
    if (crsC_try(thread, &initState, state, NULL) != CRS_OK) {
        crsE_close(thread);
        return NULL;
    }

    return thread;
}

void crsE_close(crs_Thread* thread) {
    crs_State* state = thread->state;

    crsG_freeAll(thread);
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
