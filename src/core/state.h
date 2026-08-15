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

typedef struct crs_Handler {
    int                 status;
    jmp_buf             buffer;
    struct crs_Handler* previous;
} crs_Handler;

typedef struct crs_Frame {
    crs_Object*       base;
    int               top;
    struct crs_Frame* previous;
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
    crs_Handler*      handler;
    crs_Object        error;
    struct crs_State* state;
};

/*
 * GC lists
 *
 * All collectable objects are in a long linked list, of which there are two:
 *
 * all:
 *   Objects in this list are subject to collection, as this is the list
 *   scanned during the sweep phase; most objects are added to this list.
 *
 * immune:
 *   Special objects, immune from collection, are added to this list. This
 *   list is not scanned during the sweep phase.
 */

/*
 * GC sets
 *
 * While an object must always be in a list, it isn't always in a set.
 *
 * gray:
 *   Objects in this list are gray and must be scanned for references to white
 *   objects.
 *
 * grayAgain:
 *   Objects in this list are also gray, but will be traversed once again in
 *   the atomic phase--hence the name. Objects in here have either been set
 *   back to gray by a write barrier or do not have write barriers (threads).
 */

typedef struct crs_State {
    struct {
        crs_byte       status;
        crs_byte       phase;
        crs_mem        usage; /* sum of requested sizes of allocated blocks */
        crs_mem        next;  /* usage at which to trigger gc */
        crs_mem        last;  /* usage after last gc step */
        unsigned short params[3];
        crs_GCHeader*  all;
        crs_GCHeader*  immune;
        crs_GCHeader*  gray;
        crs_GCHeader*  grayAgain;
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
