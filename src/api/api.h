/*
 * https://github.com/mochji/crescent
 * api/api.h
 *
 * Copyright (C) 2025 mochji
 * MIT License
 */

#ifndef CRS_API_API_H
#define CRS_API_API_H

#include <stddef.h>

#include "conf.h"

typedef struct crs_State crs_State;

extern int
crs_version(void);

extern int
crs_release(void);

extern const char*
crs_typeName(int type);

extern crs_State*
crs_open(void);

extern void
crs_close(crs_State* state);

extern void
crs_setPanic(crs_State* state, crs_CFunction* function);

extern int
crs_checkTop(crs_State* state, int top);

extern int
crs_getTop(crs_State* state);

extern void
crs_setTop(crs_State* state, int top);

extern int
crs_type(crs_State* state, int index);

extern size_t
crs_length(crs_State* state, int index);

extern void
crs_clone(crs_State* state, int index);

extern void
crs_deepClone(crs_State* state, int index);

extern int
crs_isNil(crs_State* state, int index);

extern int
crs_isBoolean(crs_State* state, int index);

extern int
crs_isInteger(crs_State* state, int index);

extern int
crs_isFloat(crs_State* state, int index);

extern int
crs_isNumber(crs_State* state, int index);

extern int
crs_isString(crs_State* state, int index);

extern int
crs_isCFunction(crs_State* state, int index);

extern int
crs_toBooleanX(crs_State* state, int index, int* match);

extern crs_Integer
crs_toIntegerX(crs_State* state, int index, int* match);

extern crs_Float
crs_toFloatX(crs_State* state, int index, int* match);

extern const char*
crs_toStringX(crs_State* state, int index, int* match);

extern int
crs_toBoolean(crs_State* state, int index);

extern crs_Integer
crs_toInteger(crs_State* state, int index);

extern crs_Float
crs_toFloat(crs_State* state, int index);

extern const char*
crs_toString(crs_State* state, int index);

extern crs_CFunction*
crs_toCFunction(crs_State* state, int index);

extern void
crs_pushNil(crs_State* state);

extern void
crs_pushBoolean(crs_State* state, int value);

extern void
crs_pushInteger(crs_State* state, crs_Integer value);

extern void
crs_pushFloat(crs_State* state, crs_Float value);

extern void
crs_pushString(crs_State* state, const char* str);

extern void
crs_pushCFunction(crs_State* state, crs_CFunction* function);

extern void
crs_pop(crs_State* state, int amount);

extern void
crs_remove(crs_State* state, int index);

extern int
crs_call(crs_State* state, int index, int args);

extern int
crs_pCall(crs_State* state, int index, int args, int* status);

extern int
crs_callK(crs_State* state, int index, int args, int maxResults);

extern int
crs_pCallK(crs_State* state, int index, int args, int maxResults, int* status);

extern void __attribute__((noreturn))
crs_error(crs_State* state, const char* error);

extern void
crs_clearError(crs_State* state);

extern const char*
crs_getError(crs_State* state);

#endif
