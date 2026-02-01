/*
 * https://github.com/mochji/crescent
 * api/api.h
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#ifndef CRS_API_API_H
#define CRS_API_API_H

#include <stddef.h>

#include "conf.h"

extern int
crs_version(void);

extern int
crs_release(void);

extern const char*
crs_typeName(int type);

extern crs_Thread*
crs_open(void);

extern void
crs_close(crs_Thread* thread);

extern void
crs_setPanic(crs_Thread* thread, crs_CFunction* function);

extern void
crs_error(crs_Thread* thread, int index);

extern int
crs_getTop(crs_Thread* thread);

extern int
crs_checkTop(crs_Thread* thread, int top);

extern void
crs_setTop(crs_Thread* thread, int top);

extern void
crs_pop(crs_Thread* thread, int amount);

extern void
crs_remove(crs_Thread* thread, int index);

extern int
crs_type(crs_Thread* thread, int index);

extern size_t
crs_length(crs_Thread* thread, int index);

extern void
crs_clone(crs_Thread* thread, int index);

extern int
crs_isNil(crs_Thread* thread, int index);

extern int
crs_isBoolean(crs_Thread* thread, int index);

extern int
crs_isInteger(crs_Thread* thread, int index);

extern int
crs_isFloat(crs_Thread* thread, int index);

extern int
crs_isNumber(crs_Thread* thread, int index);

extern int
crs_isCFunction(crs_Thread* thread, int index);

extern int
crs_isString(crs_Thread* thread, int index);

extern int
crs_toBooleanX(crs_Thread* thread, int index, int* match);

extern crs_Integer
crs_toIntegerX(crs_Thread* thread, int index, int* match);

extern crs_Float
crs_toFloatX(crs_Thread* thread, int index, int* match);

extern const char*
crs_toStringX(crs_Thread* thread, int index, int* match);

extern int
crs_toBoolean(crs_Thread* thread, int index);

extern crs_Integer
crs_toInteger(crs_Thread* thread, int index);

extern crs_Float
crs_toFloat(crs_Thread* thread, int index);

extern crs_CFunction*
crs_toCFunction(crs_Thread* thread, int index);

extern const char*
crs_toString(crs_Thread* thread, int index);

extern void
crs_pushNil(crs_Thread* thread);

extern void
crs_pushBoolean(crs_Thread* thread, int value);

extern void
crs_pushInteger(crs_Thread* thread, crs_Integer value);

extern void
crs_pushFloat(crs_Thread* thread, crs_Float value);

extern void
crs_pushCFunction(crs_Thread* thread, crs_CFunction* function);

extern void
crs_pushString(crs_Thread* thread, const char* str);

extern int
crs_call(crs_Thread* thread, int index, int args);

extern int
crs_pCall(crs_Thread* thread, int index, int args, int* status);

extern int
crs_callX(crs_Thread* thread, int index, int args, int maxResults);

extern int
crs_pCallX(crs_Thread* thread, int index, int args, int maxResults, int* status);

#endif
