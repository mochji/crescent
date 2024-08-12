/*
 * https://github.com/mochji/crescent
 * core/state.h
 *
 * Copyright (C) 2024 mochji
 * MIT License
 */

#ifndef CRESCENT_CORE_STATE_H
#define CRESCENT_CORE_STATE_H

#include <stddef.h>
#include <setjmp.h>

#include "conf.h"

#include "core/object.h"

struct
crescent_ErrorJump {
	struct crescent_ErrorJump* previous;
	struct crescent_Object*    top;
	struct crescent_Frame*     frame;
	int                        status;
	jmp_buf                    buffer;
};

struct
crescent_Frame {
	struct crescent_Object* base;
	int                     top;
	struct crescent_Frame*  next;
	struct crescent_Frame*  previous;
};

struct
crescent_State {
	struct {
		size_t                  size;
		struct crescent_Object* base;
		struct crescent_Object* top;
		short                   calls;
		short                   cCalls;
		struct crescent_Frame*  frame;
	} stack;
	char*                      error;
	struct crescent_ErrorJump* errorJump;
	struct crescent_State*     next;
	struct crescent_GState*    gState;
};

struct
crescent_GState {
	char*                  memoryError;
	struct crescent_Object nilValue;
	struct crescent_State* baseThread;
	struct crescent_State* lastThread;
	crescent_CFunction*    panic;
};

typedef struct crescent_ErrorJump crescent_ErrorJump;
typedef struct crescent_Frame     crescent_Frame;
typedef struct crescent_State     crescent_State;
typedef struct crescent_GState    crescent_GState;

extern crescent_GState*
crescentE_blankGState(void);

extern void
crescentE_closeGState(crescent_GState* gState);

extern crescent_State*
crescentE_blankLState(void);

extern void
crescentE_closeLState(crescent_State* state);

extern void
crescentE_bindThread(crescent_GState* gState, crescent_State* state);

#endif
