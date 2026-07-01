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
#include <stdarg.h>

#include "conf.h"

int
crs_version(void);

int
crs_release(void);

/*
 * ===========================
 *  state manipulation
 * ===========================
 */

crs_Thread*
crs_open(void);

void
crs_close(crs_Thread* thread);

void
crs_setPanic(crs_Thread* thread, crs_CFunction* function);

void
crs_error(crs_Thread* thread, int index);

/*
 * ===========================
 *  gc management
 * ===========================
 */

#define CRS_GC_FULL  0
#define CRS_GC_USAGE 1

#define CRS_GC_STOP       2
#define CRS_GC_STOPEM     3
#define CRS_GC_PAUSE      4
#define CRS_GC_STEP       5
#define CRS_GC_MULTIPLIER 6

int
crs_gc(crs_Thread* thread, int option);

int
crs_getGC(crs_Thread* thread, int option);

void
crs_setGC(crs_Thread* thread, int option, unsigned short value);

/*
 * ===========================
 *  stack manipulation
 * ===========================
 */

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

/*
 * ===========================
 *  basic object functions
 * ===========================
 */

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

/*
 * ===========================
 *  is* functions
 * ===========================
 */

#define crs_isNil(t, i)       (crs_type(t, i) == CRS_TYPE_NIL)
#define crs_isBoolean(t, i)   (crs_type(t, i) == CRS_TYPE_BOOLEAN)
#define crs_isInteger(t, i)   (crs_type(t, i) == CRS_TYPE_INTEGER)
#define crs_isFloat(t, i)     (crs_type(t, i) == CRS_TYPE_FLOAT)
#define crs_isCFunction(t, i) (crs_type(t, i) == CRS_TYPE_CFUNCTION)
#define crs_isString(t, i)    (crs_type(t, i) == CRS_TYPE_STRING)

int
crs_isNumber(crs_Thread* thread, int index);

/*
 * ===========================
 *  to* functions
 * ===========================
 */

int
crs_toBooleanX(crs_Thread* thread, int index, int* equal);

crs_Integer
crs_toIntegerX(crs_Thread* thread, int index, int* equal);

crs_Float
crs_toFloatX(crs_Thread* thread, int index, int* equal);

const char*
crs_toStringX(crs_Thread* thread, int index, int* match);

#define crs_toBoolean(t, i) crs_toBooleanX(t, i, NULL)
#define crs_toInteger(t, i) crs_toIntegerX(t, i, NULL)
#define crs_toFloat(t, i)   crs_toFloatX(t, i, NULL)
#define crs_toString(t, i)  crs_toStringX(t, i, NULL)

/*
 * ===========================
 *  push* functions
 * ===========================
 */

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

void
crs_format(crs_Thread* thread, char* format, ...);

void
crs_vformat(crs_Thread* thread, char* format, va_list args);

/*
 * ===========================
 *  calling
 * ===========================
 */

void
crs_call(crs_Thread* thread, int index, int args, int wanted);

int
crs_pcall(crs_Thread* thread, int index, int args, int wanted);

#endif
