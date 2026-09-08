/*
 * https://github.com/mochji/crescent
 * core/call.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <setjmp.h>

#include "crescent/conf.h"
#include "limit.h"

#include "types/string.h"
#include "core/state.h"
#include "core/memory.h"
#include "core/format.h"
#include "vm/vm.h"

#include "core/call.h"

#define STACK_MAX (SIZE_MAX / sizeof(crs_Object))

/*
 * WARNING: As a pcall might catch and handle any error, be careful to ensure
 *          the state of the thread when you throw one.
 */

noret crsC_throw(crs_Thread* thread, int status) {
    crs_Jump*  jump  = thread->jump;
    crs_State* state = thread->state;

    while (jump != NULL) {
        assert(jump->status == CRS_OK);
        jump->status = status;
        longjmp(jump->buffer, 1);
    }

    if (state->panic != NULL) {
        state->panic(thread);
    }

    abort();
}

noret crsC_error(crs_Thread* thread, char* message) {
    crs_String* error = crsS_new(thread, message);

    obj_setgc(&thread->error, error);
    crsC_throw(thread, CRS_ERROR);
}

noret crsC_errorf(crs_Thread* thread, char* format, ...) {
    crs_String* error;
    va_list     args;

    va_start(args, format);
    error = crsF_vformat(thread, format, args);
    va_end(args);

    obj_setgc(&thread->error, error);
    crsC_throw(thread, CRS_ERROR);
}

void crsC_unwind(crs_Thread* thread, short level) {
    while (thread->stack.level > level) {
        crs_Frame* frame    = thread->stack.frame;
        thread->stack.top   = frame->base;
        thread->stack.frame = frame->previous;
        thread->stack.level--;

        mem_free(thread, frame);
    }
}

int crsC_try(crs_Thread* thread, crs_PFunction* function,
                                 void* data, void** result) {
    crs_Jump jump;
    void*    returned = NULL;

    jump.status   = CRS_OK;
    jump.previous = thread->jump;
    thread->jump  = &jump;

    if (!setjmp(jump.buffer)) {
        returned = function(thread, data);
    }

    if (result != NULL) {
        *result = returned;
    }

    thread->jump = jump.previous;
    return jump.status;
}

static int reallocStack(crs_Thread* thread, size_t newSize, int throw) {
    crs_Object* newStack = mem_vresize(thread,
        thread->stack.base, newSize, thread->stack.size);

    if (newStack == NULL) {
        if (throw) {
            crsM_error(thread);
        }

        return 1;
    }

    ptrdiff_t  offset = newStack - thread->stack.base;
    crs_Frame* frame  = thread->stack.frame;

    thread->stack.size = newSize;
    thread->stack.base = newStack;
    thread->stack.top += offset;

    while (frame != NULL) {
        frame->base += offset;
        frame        = frame->previous;
    }

    return 0;
}

int crsC_resizeStack(crs_Thread* thread, size_t needed, int throw) {
    needed += CRS_MIN_STACK;

    if (needed > STACK_MAX) {
        if (throw) {
            crsC_error(thread, "stack overflow");
        }

        return 1;
    }

    size_t size    = thread->stack.size;
    size_t newSize = needed + needed / 2;

    if (newSize > STACK_MAX) {
        newSize = needed;
    } else if (newSize < CRS_MIN_STACK) {
        newSize = CRS_MIN_STACK;
    }

    if (needed <= size / 3 && size > CRS_MIN_STACK) {
        reallocStack(thread, newSize, 0);

        return 0;
    } else if (needed > size) {
        return reallocStack(thread, newSize, throw);
    }

    return 0;
}

/* resize the stack frame to have 'top' elements */
int crsC_checkTop(crs_Thread* thread, int top, int throw) {
    if (top > CRS_MAX_TOP) {
        if (throw) {
            crsC_error(thread, "stack overflow");
        }

        return 1;
    } else if (top < CRS_MIN_TOP) {
        top = CRS_MIN_TOP;
    }

    crs_Object* stack  = thread->stack.base;
    crs_Frame*  frame  = thread->stack.frame;
    ptrdiff_t   needed = (frame->base - stack) + top;

    frame = frame->previous;

    while (frame != NULL) {
        ptrdiff_t frameNeeds = (frame->base - stack) + frame->top;

        if (frameNeeds > needed) {
            needed = frameNeeds;
        }

        frame = frame->previous;
    }

    if (crsC_resizeStack(thread, (size_t)needed, throw)) {
        return 1;
    }

    thread->stack.frame->top = top;
    return 0;
}

/* ensure there is at least 'free' free elements on the stack */
int crsC_checkFree(crs_Thread* thread, int free, int throw) {
    size_t needed = (size_t)((thread->stack.top - thread->stack.base) + free);

    if (needed > thread->stack.size - CRS_MIN_FREE) {
        return crsC_resizeStack(thread, needed, throw);
    }

    return 0;
}

static void checkResults(crs_Thread* thread, int wanted) {
    crs_Frame* frame    = thread->stack.frame;
    crs_Frame* previous = frame->previous;
    size_t     needed   = (size_t)((frame->base - thread->stack.base) + wanted);
    int        free     = (int)(previous->top - (frame->base - previous->base));

    /* previous frame cannot hold results? */
    if (wanted > CRS_MAX_TOP - free) {
        crsC_error(thread, "stack overflow");
    }

    /* need to grow stack? (only possible when wanted > results) */
    if (needed > thread->stack.size) {
        crsC_resizeStack(thread, needed, 1);
    }

    if (wanted > free && !(previous->flags & CALL_VM)) {
        previous->top += wanted - free;
    }
}

/* create and initialize a new stack frame */
static crs_Frame* startCall(crs_Thread* thread, int top, int args) {
    if (thread->stack.level >= CRS_MAX_LEVEL) {
        crsC_error(thread, "stack overflow");
    }

    crsC_checkFree(thread, top - args, 1);

    crs_Frame* frame    = mem_new(thread, crs_Frame);
    crs_Frame* previous = thread->stack.frame;

    if (frame == NULL) {
        crsM_error(thread);
    }

    frame->previous = previous;
    frame->base     = thread->stack.top - args;
    frame->top      = top;
    frame->flags    = 0;

    thread->stack.frame = frame;
    thread->stack.level++;

    return frame;
}

/* return 'wanted' elements and pop the top stack frame */
static void endCall(crs_Thread* thread, int results, int wanted) {
    if (results > MAX_RET) {
        results = MAX_RET;
    }

    if (wanted == CRS_RETALL) {
        wanted = results;
    }

    crs_Frame* frame    = thread->stack.frame;
    crs_Frame* previous = frame->previous;
    int        top      = (int)(thread->stack.top - frame->base);

    /* only return as much as the frame has */
    results = results > top ? top : results;
    checkResults(thread, wanted);

    crs_Object* from = thread->stack.top - results;
    crs_Object* to   = frame->base;

    /* move top 'results' elements down to previous frame */
    if (results < top) {
        for (int i = 0; i < results; i++) {
            obj_seto(to, from);
            to++;
            from++;
        }
    } else {
        to += results;
    }

    /* return nil for missing elements */
    for (int i = results; i < wanted; i++) {
        obj_setn(to);
        to++;
    }

    thread->stack.top  -= top - wanted;
    thread->stack.frame = previous;
    thread->stack.level--;

    mem_free(thread, frame);
}

void crsC_call(crs_Thread* thread, crs_Function* function,
                                   int args, int wanted) {
    if (args > function->args) {
        /* discard unused arguments */
        thread->stack.top -= args - function->args;
        args               = function->args;
    }

    crs_Frame* frame = startCall(thread, function->top, args);
    frame->flags    |= CALL_VM;
    frame->i.v.f     = function;

    /* fill missing args with nil */
    for (int i = args; i < function->args; i++) {
        obj_setn(thread->stack.top);
        thread->stack.top++;
    }

    endCall(thread, crsV_execute(thread, function), wanted);
}

void crsC_callC(crs_Thread* thread, crs_CFunction* function,
                                    int args, int wanted) {
    startCall(
        thread,
        args < CRS_MIN_TOP
            ? CRS_MIN_TOP
            : args,
        args
    )->i.c.f = function;

    endCall(thread, function(thread), wanted);
}

crs_Object* crsC_anchor(crs_Thread* thread, crs_GCHeader* header) {
    obj_seth(thread->stack.top, header);
    return thread->stack.top++;
}

void crsC_unanchor(crs_Thread* thread) {
    thread->stack.top--;
}
