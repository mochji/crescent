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

#include "conf.h"
#include "limit.h"

#include "core/object.h"

struct
crs_Frame {
	struct crs_Object* base;
	int                top;
	struct crs_Frame*  next;
	struct crs_Frame*  previous;
};

struct
crs_Thread {
	struct crs_GCHeader header;
	struct {
		size_t             size;
		struct crs_Object* base;
		struct crs_Object* top;
		short              calls;
		short              cCalls;
		struct crs_Frame*  frame;
	}                 stack;
	int               status;
	char*             error;
	jmp_buf*          handler;
	struct crs_State* state;
};

/*
 * GC usage, next, and last
 *
 * When a block is allocated/freed, the size of it is added to/subtracted from
 * the usage. Once the usage exceeds next, the GC is triggered, and how much
 * the usage has grown between now and the last step (usage - last), determines
 * how much work will be done in this step.
 */

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

struct
crs_State {
	struct {
		crs_byte              status;
		crs_byte              phase;
		crs_mem               usage;
		crs_mem               next;
		crs_mem               last;
		unsigned short        params[3];
		struct crs_GCHeader*  all;
		struct crs_GCHeader*  immune;
		struct crs_GCHeader*  gray;
		struct crs_GCHeader*  grayAgain;
		struct crs_GCHeader** sweep;
	}                  gc;
	char*              memoryError;
	struct crs_Object  nilValue;
	struct crs_Thread* thread;
	crs_CFunction*     panic;
};

typedef struct crs_Frame  crs_Frame;
typedef struct crs_Thread crs_Thread;
typedef struct crs_State  crs_State;

extern crs_State*
crsE_newState(void);

extern void
crsE_freeState(crs_State* state);

extern crs_Thread*
crsE_newThread(crs_State* state);

extern void
crsE_freeThread(crs_Thread* thread);

#endif
