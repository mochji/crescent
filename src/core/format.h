/*
 * https://github.com/mochji/crescent
 * core/format.h
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#ifndef CRS_CORE_FORMAT_H
#define CRS_CORE_FORMAT_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "conf.h"
#include "limit.h"

#define fmt_int(b, s, v)      snprintf((b), (s), "%d", (v))
#define fmt_unsigned(b, s, v) snprintf((b), (s), "%u", (v))
#define fmt_pointer(b, s, v)  snprintf((b), (s), "%p", (v))
#define fmt_integer(b, s, v)  snprintf((b), (s), CRS_INTEGER_FMT, (v))
#define fmt_float(b, s, v)    snprintf((b), (s), "%g", (v))

#if CRS_INTEGER_TYPE == CRS_INTEGER_INT
#define fmt_tointeger(s, e) ((crs_Integer)strtol((s), (e), 0))
#elif CRS_INTEGER_TYPE == CRS_INTEGER_LONG
#define fmt_tointeger(s, e) ((crs_Integer)strtol((s), (e), 0))
#elif CRS_INTEGER_TYPE == CRS_INTEGER_LLONG
#define fmt_tointeger(s, e) ((crs_Integer)strtoll((s), (e), 0))
#endif

#if CRS_FLOAT_TYPE == CRS_FLOAT_FLOAT
#define fmt_tofloat(s, e) strtof((s), (e))
#elif CRS_FLOAT_TYPE == CRS_FLOAT_DOUBLE
#define fmt_tofloat(s, e) strtod((s), (e))
#elif CRS_FLOAT_TYPE == CRS_FLOAT_LDOUBLE
#define fmt_tofloat(s, e) strtold((s), (e))
#endif

crs_String*
crsF_format(crs_Thread* thread, char* format, ...);

crs_String*
crsF_vformat(crs_Thread* thread, char* format, va_list args);

int
crsF_toInteger(char* str, crs_Integer* value, size_t length);

int
crsF_toFloat(char* str, crs_Float* value, size_t length);

#endif
