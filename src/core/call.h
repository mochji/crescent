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

void __attribute__((noreturn))
crsC_throw(crs_Thread* thread);

void __attribute__((noreturn))
crsC_error(crs_Thread* thread, char* message);

void
crsC_restoreStack(crs_Thread* thread, short level);

int
crsC_resizeStack(crs_Thread* thread, size_t needed, int throw);

int
crsC_checkTop(crs_Thread* thread, int top, int throw);

int
crsC_checkFree(crs_Thread* thread, int free, int throw);

int
crsC_callC(crs_Thread* thread, crs_CFunction* function, int args, int maxResults);

#define call_sethandler(t, h) \
	{(h).status = CRS_STATUS_OK; (h).level = (t)->stack.calls; (h).previous = (t)->handler; (t)->handler = &(h);}

#endif
