/*
 * https://github.com/mochji/crescent
 * types/string.h
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#ifndef CRS_TYPES_STRING_H
#define CRS_TYPES_STRING_H

#include <stddef.h>
#include <stdarg.h>

#include "conf.h"
#include "limit.h"

#include "core/object.h"
#include "core/state.h"

crs_String*
crsS_newo(crs_Thread* thread, size_t length);

crs_String*
crsS_newl(crs_Thread* thread, char* str, size_t length);

crs_String*
crsS_new(crs_Thread* thread, char* str);

void
crsS_free(crs_Thread* thread, crs_String* string);

int
crsS_equal(crs_String* stringA, crs_String* stringB);

unsigned
crsS_hash(crs_String* string);

void
crsS_clearCache(crs_State* state);

#endif
