/*
 * https://github.com/mochji/crescent
 * core/state.h
 *
 * idrk what this is
 * Copyright (C) 2024 mochji
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef CORE_STATE_H
#define CORE_STATE_H

#include <stddef.h>
#include <setjmp.h>

#include "conf.h"

#include "core/object.h"

struct
crescent_ErrorJump {
	jmp_buf buffer;
	int     status;
};

struct
crescent_Frame {
	size_t                 base;
	size_t                 top;
	struct crescent_Frame* next;
	struct crescent_Frame* previous;
};

struct
crescent_State {
	struct {
		size_t                  size;
		size_t                  maxFrames;
		size_t                  frameCount;
		struct crescent_Object* data;
		struct crescent_Frame** frames;
		struct crescent_Frame*  topFrame;
	} stack;
	size_t                     threadIndex;
	char*                      error;
	struct crescent_ErrorJump* errorJump;
	struct crescent_GState*    gState;
};

struct
crescent_GState {
	size_t                  maxThreads;
	size_t                  threadCount;
	struct crescent_State** threads;
	struct crescent_State*  baseThread;
	crescent_CFunction*     panic;
};

typedef struct crescent_ErrorJump crescent_ErrorJump;
typedef struct crescent_Frame     crescent_Frame;
typedef struct crescent_State     crescent_State;
typedef struct crescent_GState    crescent_GState;

extern crescent_GState*
crescentG_blankGState();

extern void
crescentG_closeGState(crescent_GState* gState);

extern crescent_State*
crescentG_blankLState();

extern void
crescentG_closeLState(crescent_State* state);

#endif
