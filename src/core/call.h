/*
 * https://github.com/mochji/crescent
 * core/call.h
 *
 * Copyright (C) 2024 mochji
 * MIT License
 */

#ifndef CRESCENT_CORE_CALL_H
#define CRESCENT_CORE_CALL_H

#include "conf.h"

#include "core/object.h"
#include "core/state.h"

void
crescentC_setError(crescent_State* state, char* error);

void
crescentC_moveError(crescent_State* to, crescent_State* from);

void __attribute__((noreturn))
crescentC_throw(crescent_State* state, int status);

extern void __attribute__((noreturn))
crescentC_memoryError(crescent_State* state);

extern void
crescentC_correctStack(crescent_State* state, crescent_Object* stack);

extern int
crescentC_reallocStack(crescent_State* state, size_t size);

extern int
crescentC_resizeStack(crescent_State* state, int top, int throw);

extern void
crescentC_startCall(crescent_State* state, int args, crescent_Frame* newTopFrame);

extern void
crescentC_endCall(crescent_State* state, int results);

#endif
