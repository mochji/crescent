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
crs_State {
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
	struct crs_State*   next;
	struct crs_GState*  gState;
};

struct
crs_GState {
	char*             memoryError;
	struct crs_Object nilValue;
	struct crs_State* mainThread;
	struct crs_State* lastThread;
	crs_CFunction*    panic;
};

typedef struct crs_Frame  crs_Frame;
typedef struct crs_State  crs_State;
typedef struct crs_GState crs_GState;

extern crs_GState*
crsE_blankGState(void);

extern void
crsE_closeGState(crs_GState* gState);

extern crs_State*
crsE_blankLState(void);

extern void
crsE_closeLState(crs_State* state);

#endif
