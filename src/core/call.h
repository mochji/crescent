/*
 * https://github.com/mochji/crescent
 * core/call.h
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#ifndef CRS_CORE_CALL_H
#define CRS_CORE_CALL_H

#include <stddef.h>

#include "crescent/conf.h"
#include "limit.h"

#include "core/object.h"

typedef void* (crs_PFunction)(crs_Thread*, void*);

noret crsC_throw(crs_Thread* thread, int status);
noret crsC_error(crs_Thread* thread, char* message);
noret crsC_errorf(crs_Thread* thread, char* format, ...);
void  crsC_unwind(crs_Thread* thread, short level);
int   crsC_try(crs_Thread* thread, crs_PFunction* function,
                                   void* data, void** result);
int crsC_resizeStack(crs_Thread* thread, size_t needed, int throw);
int crsC_checkTop(crs_Thread* thread, int top, int throw);
int crsC_checkFree(crs_Thread* thread, int free, int throw);
void crsC_call(crs_Thread* thread, crs_Function* function,
                                   int args, int wanted);
void crsC_callC(crs_Thread* thread, crs_CFunction* function,
                                    int args, int wanted);
crs_Object* crsC_anchor(crs_Thread* thread, crs_GCHeader* header);
void        crsC_unanchor(crs_Thread* thread);

#define call_savetop(t)       ((t)->stack.top - (t)->stack.base)
#define call_restoretop(t, v) ((t)->stack.top = (t)->stack.base + (v))

#endif
