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

typedef void* (crs_PFunction)(crs_Thread*, void*);

void __attribute__((noreturn))
crsC_throw(crs_Thread* thread, int status);

void __attribute__((noreturn))
crsC_error(crs_Thread* thread, char* message);

void __attribute__((noreturn))
crsC_errorf(crs_Thread* thread, char* format, ...);

void
crsC_restoreStack(crs_Thread* thread, short level);

int
crsC_try(crs_Thread* thread, crs_PFunction* function, void* data, void** result);

int
crsC_resizeStack(crs_Thread* thread, size_t needed, int throw);

int
crsC_checkTop(crs_Thread* thread, int top, int throw);

int
crsC_checkFree(crs_Thread* thread, int free, int throw);

void
crsC_call(crs_Thread* thread, crs_Function* function, int args, int wanted);

void
crsC_callC(crs_Thread* thread, crs_CFunction* function, int args, int wanted);

#endif
