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

extern void
crescentC_setError(crescent_State* state, char* error);

extern void
crescentC_moveError(crescent_State* to, crescent_State* from);

extern void __attribute__((noreturn))
crescentC_throw(crescent_State* state, int status);

extern void __attribute__((noreturn))
crescentC_memoryError(crescent_State* state);

extern int
crescentC_reallocStack(crescent_State* state, size_t size);

extern int
crescentC_resizeStack(crescent_State* state, int top, int throw);

extern void
crescentC_startCall(crescent_State* state, int args);

extern void
crescentC_endCall(crescent_State* state, int results);

#endif
