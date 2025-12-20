/*
 * https://github.com/mochji/crescent
 * core/state.h
 *
 * Copyright (C) 2025 mochji
 * MIT License
 */

#ifndef CRS_CORE_STATE_H
#define CRS_CORE_STATE_H

#include <stddef.h>
#include <setjmp.h>

#include "conf.h"

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
	struct {
		size_t             size;
		struct crs_Object* base;
		struct crs_Object* top;
		short              calls;
		short              cCalls;
		struct crs_Frame*  frame;
	} stack;
	int                 status;
	char*               error;
	jmp_buf*            handler;
	struct crs_Thread*  next;
	struct crs_State*   state;
};

struct
crs_State {
	char*              memoryError;
	struct crs_Object  nilValue;
	struct crs_Thread* mainThread;
	struct crs_Thread* lastThread;
	crs_CFunction*     panic;
};

typedef struct crs_Frame  crs_Frame;
typedef struct crs_Thread crs_Thread;
typedef struct crs_State  crs_State;

extern crs_State*
crsE_blankState(void);

extern void
crsE_closeState(crs_State* state);

extern crs_Thread*
crsE_blankThread(void);

extern void
crsE_closeThread(crs_Thread* thread);

#endif
