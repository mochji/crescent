/*
 * https://github.com/mochji/crescent
 * core/state.h
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#ifndef CRS_CORE_STATE_H
#define CRS_CORE_STATE_H

#include <stddef.h>
#include <setjmp.h>

#include "crescent/conf.h"
#include "limit.h"

#include "core/object.h"

#define CRS_STRCACHE_SIZE    32
#define CRS_STRCACHE_BUCKETS 4

typedef struct crs_Jump {
    int              status;
    jmp_buf          buffer;
    struct crs_Jump* previous;
} crs_Jump;

/* frame flags */
#define CALL_VM 1

#define call_isvm(r) ((r)->flags & CALL_VM)
#define call_hasdebug(r) \
    (((r)->flags & CALL_VM) && ((r)->i.v.f->flags & FUNC_DEBUG))

typedef struct crs_Frame {
    struct crs_Frame* previous;
    crs_Object*       base;
    int               top;
    crs_byte          flags;
    union {
        struct {
            crs_Function* f;
            crs_instr*    pc; /* last/currently executing instruction */
        } v;
        struct {
            crs_CFunction* f;
        } c;
    } i;
} crs_Frame;

struct crs_Thread {
    crs_GCHeader gc;
    struct {
        size_t      size;
        crs_Object* base;
        crs_Object* top;
        short       level;
        crs_Frame*  frame;
        crs_Frame   baseFrame;
    }                 stack;
    crs_Jump*         jump;
    crs_Object        error;
    struct crs_State* state;
};

typedef struct crs_State {
    struct {
        crs_byte       status;
        crs_byte       phase;
        crs_mem        usage; /* sum of requested sizes of allocated blocks */
        crs_mem        next;  /* usage at which to trigger gc */
        crs_mem        last;  /* usage after last gc step */
        unsigned short params[3];
        crs_GCHeader*  all;       /* objects subject to collection */
        crs_GCHeader*  immune;    /* objects immune from collection */
        crs_GCHeader*  gray;      /* to be traversed */
        crs_GCHeader*  grayAgain; /* to be traversed atomically */
        crs_GCHeader** sweep;
    }              gc;
    crs_String*    strings[CRS_STRCACHE_SIZE][CRS_STRCACHE_BUCKETS];
    crs_String*    memoryError;
    crs_CFunction* panic;
    crs_Object     globals;
    crs_Thread     thread;
} crs_State;

crs_Thread* crsE_open(void);
void        crsE_close(crs_State* state);
void        crsE_freeThread(crs_Thread* thread);

#endif
