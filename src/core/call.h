/*
 * https://github.com/mochji/crescent
 * core/call.h
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#ifndef CRS_CORE_CALL_H
#define CRS_CORE_CALL_H

#include "conf.h"
#include "limit.h"

#include "core/object.h"

extern void __attribute__((noreturn))
crsC_throw(crs_Thread* thread);

extern void __attribute__((noreturn))
crsC_error(crs_Thread* thread, char* message);

extern void
crsC_restoreStack(crs_Thread* thread, short level);

extern int
crsC_resizeStack(crs_Thread* thread, size_t needed, int throw);

extern int
crsC_checkTop(crs_Thread* thread, int top, int throw);

extern int
crsC_checkFree(crs_Thread* thread, int free, int throw);

extern int
crsC_callC(crs_Thread* thread, crs_CFunction* function, int args, int maxResults);

#endif
