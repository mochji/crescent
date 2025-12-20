/*
 * https://github.com/mochji/crescent
 * core/call.h
 *
 * Copyright (C) 2025 mochji
 * MIT License
 */

#ifndef CRS_CORE_CALL_H
#define CRS_CORE_CALL_H

#include "conf.h"

#include "core/object.h"
#include "core/state.h"

/* error handling */

extern void
crsC_setError(crs_State* state, char* error);

extern void
crsC_moveError(crs_State* to, crs_State* from);

extern void __attribute__((noreturn))
crsC_throw(crs_State* state, int status);

extern void __attribute__((noreturn))
crsC_memoryError(crs_State* state);

extern void
crsC_restoreStack(crs_State* state, short level);

/* stack resizing */

extern int
crsC_reallocStack(crs_State* state, size_t newSize, int throw);

extern int
crsC_resizeStack(crs_State* state, size_t needed, int throw);

extern int
crsC_checkTop(crs_State* state, int top, int throw);

extern int
crsC_checkFree(crs_State* state, int free, int throw);

/* calling */

extern void
crsC_startCall(crs_State* state, crs_Frame* frame,  int top, int args);

extern void
crsC_endCall(crs_State* state, int results);

extern int
crsC_callC(crs_State* state, crs_CFunction* function, int args, int maxResults);

#endif
