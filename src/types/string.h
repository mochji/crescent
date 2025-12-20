/*
 * https://github.com/mochji/crescent
 * types/string.h
 *
 * Copyright (C) 2025 mochji
 * MIT License
 */

#ifndef CRS_TYPES_STRING_H
#define CRS_TYPES_STRING_H

#include <stddef.h>
#include <stdarg.h>

#include "conf.h"

#include "core/object.h"

extern crs_String*
crsS_new(size_t length);

extern crs_String*
crsS_nullString(void);

extern crs_String*
crsS_as(char* str);

extern crs_String*
crsS_clone(crs_String* string);

extern void
crsS_free(crs_String* string);

extern int
crsS_resize(crs_String* string, size_t newLength);

extern int
crsS_compare(crs_String* stringA, crs_String* stringB);

extern size_t
crsS_hash(char* str);

extern char*
crsS_copy(char* str);

extern int
crsS_hexValue(char c);

extern crs_Integer
crsS_toInteger(char* str, int* success);

extern crs_Float
crsS_toFloat(char* str, int* success);

#endif
