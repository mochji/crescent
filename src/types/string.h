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

extern crs_String*
crsS_new(crs_Thread* thread, char* str);

extern crs_String*
crsS_external(crs_Thread* thread, char* str);

extern void
crsS_free(crs_Thread* thread, crs_String* string);

extern int
crsS_compare(crs_String* stringA, crs_String* stringB);

#endif
