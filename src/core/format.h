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
#include <stdarg.h>

#include "conf.h"
#include "limit.h"

#define fmt_pointer(b, s, v) snprintf((b), (s), "%p", (v))
#define fmt_integer(b, s, v) snprintf((b), (s), CRS_INTEGER_FMT, (v))
#define fmt_float(b, s, v)   snprintf((b), (s), "%g", (v))

crs_String*
crsF_format(crs_Thread* thread, char* format, ...);

crs_String*
crsF_vformat(crs_Thread* thread, char* format, va_list args);

crs_Integer
crsF_toInteger(char* str, int* success);

crs_Float
crsF_toFloat(char* str, int* success);

#endif
