/*
 * https://github.com/mochji/crescent
 * core/call.h
 *
 * idrk what this is
 * Copyright (C) 2024 mochji
 *
 * MIT License
 */

#ifndef CORE_CALL_H
#define CORE_CALL_H

#include "conf.h"

#include "core/object.h"
#include "core/state.h"

extern void
crescentC_setError(crescent_State* state, char* error);

extern void
crescentC_moveError(crescent_State* to, crescent_State* from);

extern void __attribute__((noreturn))
crescentC_panic(crescent_State* state);

extern void __attribute__((noreturn))
crescentC_throw(crescent_State* state, int status);

extern void __attribute__((noreturn))
crescentC_memoryError(crescent_State* state);

extern int
crescentC_growStack(crescent_State* state, int newTop);

extern int
crescentC_shrinkStack(crescent_State* state, int newTop);

extern int
crescentC_resizeStack(crescent_State* state, int newTop, int throw);

extern void
crescentC_startCall(crescent_State* state, int argCount, crescent_Frame* newTopFrame);

extern void
crescentC_endCall(crescent_State* state, int results);

#endif
