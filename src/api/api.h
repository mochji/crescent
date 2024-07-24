/*
 * https://github.com/mochji/crescent
 * api/api.h
 *
 * Copyright (C) 2024 mochji
 * MIT License
 */

#ifndef CRESCENT_API_API_H
#define CRESCENT_API_API_H

#include <stddef.h>

#include "conf.h"

typedef struct crescent_State crescent_State;

extern int
crescent_version(void);

extern int
crescent_release(void);

extern const char*
crescent_typeName(int type);

extern crescent_State*
crescent_open(void);

extern void
crescent_close(crescent_State* state);

extern void
crescent_setPanic(crescent_State* state, crescent_CFunction* function);

extern int
crescent_checkTop(crescent_State* state, int top);

extern int
crescent_getTop(crescent_State* state);

extern void
crescent_setTop(crescent_State* state, int top);

extern int
crescent_type(crescent_State* state, int index);

extern size_t
crescent_length(crescent_State* state, int index);

extern int
crescent_isNil(crescent_State* state, int index);

extern int
crescent_isBoolean(crescent_State* state, int index);

extern int
crescent_isInteger(crescent_State* state, int index);

extern int
crescent_isFloat(crescent_State* state, int index);

extern int
crescent_isNumber(crescent_State* state, int index);

extern int
crescent_isString(crescent_State* state, int index);

extern int
crescent_isCFunction(crescent_State* state, int index);

extern int
crescent_toBooleanX(crescent_State* state, int index, int* isBoolean);

extern crescent_Integer
crescent_toIntegerX(crescent_State* state, int index, int* isInteger);

extern crescent_Float
crescent_toFloatX(crescent_State* state, int index, int* isFloat);

extern const char*
crescent_toStringX(crescent_State* state, int index, int* isString);

extern int
crescent_toBoolean(crescent_State* state, int index);

extern crescent_Integer
crescent_toInteger(crescent_State* state, int index);

extern crescent_Float
crescent_toFloat(crescent_State* state, int index);

extern const char*
crescent_toString(crescent_State* state, int index);

extern crescent_CFunction*
crescent_toCFunction(crescent_State* state, int index);

extern void
crescent_pushNil(crescent_State* state);

extern void
crescent_pushBoolean(crescent_State* state, int value);

extern void
crescent_pushInteger(crescent_State* state, crescent_Integer value);

extern void
crescent_pushFloat(crescent_State* state, crescent_Float value);

extern const char*
crescent_pushString(crescent_State* state, const char* str);

extern void
crescent_pushCFunction(crescent_State* state, crescent_CFunction* function);

extern void
crescent_pop(crescent_State* state, int amount);

extern void
crescent_remove(crescent_State* state, int index);

extern int
crescent_call(crescent_State* state, int index, int args);

extern int
crescent_pCall(crescent_State* state, int index, int args, int* status);

extern int
crescent_callK(crescent_State* state, int index, int args, int maxResults);

extern int
crescent_pCallK(crescent_State* state, int index, int args, int maxResults, int* status);

extern void __attribute__((noreturn))
crescent_error(crescent_State* state, const char* error);

extern void
crescent_clearError(crescent_State* state);

extern const char*
crescent_getError(crescent_State* state);

#endif
