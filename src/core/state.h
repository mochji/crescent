/*
 * https://github.com/mochji/crescent
 * core/state.h
 *
 * idrk what this is
 * Copyright (C) 2024 mochji
 *
 * MIT License
 */

#ifndef CORE_STATE_H
#define CORE_STATE_H

#include <stddef.h>
#include <setjmp.h>

#include "conf.h"

#include "core/object.h"

struct
crescent_ErrorJump {
	int     status;
	jmp_buf buffer;
};

struct
crescent_Frame {
	int                    base;
	int                    top;
	struct crescent_Frame* next;
	struct crescent_Frame* previous;
};

struct
crescent_State {
	struct {
		size_t                  size;
		struct crescent_Object* data;
		int                     frames;
		struct crescent_Frame*  topFrame;
	} stack;
	char*                      error;
	struct crescent_ErrorJump* errorJump;
	struct crescent_State*     next;
	struct crescent_GState*    gState;
};

struct
crescent_GState {
	char*                   memoryError;
	struct crescent_Object  nilValue;
	struct crescent_State*  baseThread;
	crescent_CFunction*     panic;
};

typedef struct crescent_ErrorJump crescent_ErrorJump;
typedef struct crescent_Frame     crescent_Frame;
typedef struct crescent_State     crescent_State;
typedef struct crescent_GState    crescent_GState;

extern crescent_GState*
crescentG_blankGState(void);

extern void
crescentG_closeGState(crescent_GState* gState);

extern crescent_State*
crescentG_blankLState(void);

extern void
crescentG_closeLState(crescent_State* state);

#define state_abstop(state) \
	((unsigned int)((state)->stack.topFrame->base + (state)->stack.topFrame->top))

#endif
