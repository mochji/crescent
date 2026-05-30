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

#define CRS_GC_FULL  0
#define CRS_GC_USAGE 1

#define CRS_GC_STOP       2
#define CRS_GC_STOPEM     3
#define CRS_GC_PAUSE      4
#define CRS_GC_STEP       5
#define CRS_GC_MULTIPLIER 6

int
crs_version(void);

int
crs_release(void);

crs_Thread*
crs_open(void);

void
crs_close(crs_Thread* thread);

void
crs_setPanic(crs_Thread* thread, crs_CFunction* function);

void
crs_error(crs_Thread* thread, int index);

int
crs_gc(crs_Thread* thread, int option);

int
crs_getGC(crs_Thread* thread, int option);

void
crs_setGC(crs_Thread* thread, int option, unsigned short value);

int
crs_getTop(crs_Thread* thread);

int
crs_checkTop(crs_Thread* thread, int top);

void
crs_setTop(crs_Thread* thread, int top);

void
crs_pop(crs_Thread* thread, int amount);

void
crs_remove(crs_Thread* thread, int index);

int
crs_type(crs_Thread* thread, int index);

const char*
crs_name(crs_Thread* thread, int index);

crs_Integer
crs_length(crs_Thread* thread, int index);

void
crs_get(crs_Thread* thread, int index, int keyIndex);

void
crs_set(crs_Thread* thread, int index, int keyIndex, int valueIndex);

void
crs_copy(crs_Thread* thread, int index);

int
crs_isNil(crs_Thread* thread, int index);

int
crs_isBoolean(crs_Thread* thread, int index);

int
crs_isInteger(crs_Thread* thread, int index);

int
crs_isFloat(crs_Thread* thread, int index);

int
crs_isNumber(crs_Thread* thread, int index);

int
crs_isCFunction(crs_Thread* thread, int index);

int
crs_isString(crs_Thread* thread, int index);

int
crs_toBooleanX(crs_Thread* thread, int index, int* match);

crs_Integer
crs_toIntegerX(crs_Thread* thread, int index, int* match);

crs_Float
crs_toFloatX(crs_Thread* thread, int index, int* match);

const char*
crs_toStringX(crs_Thread* thread, int index, int* match);

int
crs_toBoolean(crs_Thread* thread, int index);

crs_Integer
crs_toInteger(crs_Thread* thread, int index);

crs_Float
crs_toFloat(crs_Thread* thread, int index);

crs_CFunction*
crs_toCFunction(crs_Thread* thread, int index);

const char*
crs_toString(crs_Thread* thread, int index);

void
crs_pushNil(crs_Thread* thread);

void
crs_pushBoolean(crs_Thread* thread, int value);

void
crs_pushInteger(crs_Thread* thread, crs_Integer value);

void
crs_pushFloat(crs_Thread* thread, crs_Float value);

void
crs_pushCFunction(crs_Thread* thread, crs_CFunction* function);

void
crs_pushString(crs_Thread* thread, const char* str);

void
crs_pushTable(crs_Thread* thread);

int
crs_call(crs_Thread* thread, int index, int args);

int
crs_pCall(crs_Thread* thread, int index, int args, int* status);

int
crs_callX(crs_Thread* thread, int index, int args, int maxResults);

int
crs_pCallX(crs_Thread* thread, int index, int args, int maxResults, int* status);

#endif
