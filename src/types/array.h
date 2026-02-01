/*
 * https://github.com/mochji/crescent
 * types/array.h
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#ifndef CRS_TYPES_ARRAY_H
#define CRS_TYPES_ARRAY_H

#include <stddef.h>

#include "conf.h"
#include "limit.h"

#include "core/object.h"

extern crs_Array*
crsA_new(crs_Thread* thread, size_t size);

extern void
crsA_free(crs_Thread* thread, crs_Array* array);

extern int
crsA_compare(crs_Array* arrayA, crs_Array* arrayB);

#endif
