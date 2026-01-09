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

/* TODO: redo everything here. everything! */

extern crs_Array*
crsA_new(size_t size);

extern crs_Array*
crsA_clone(crs_Array* array);

extern void
crsA_free(crs_Array* array);

extern int
crsA_resize(crs_Array* array, size_t newLength);

extern int
crsA_compare(crs_Array* arrayA, crs_Array* arrayB);

#endif
