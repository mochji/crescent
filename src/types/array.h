/*
 * https://github.com/mochji/crescent
 * types/array.h
 *
 * Copyright (C) 2024 mochji
 * MIT License
 */

#ifndef CRESCENT_TYPES_ARRAY_H
#define CRESCENT_TYPES_ARRAY_H

#include <stddef.h>

#include "conf.h"

#include "core/object.h"

extern crescent_Array*
crescentA_new(size_t length);

extern crescent_Array*
crescentA_clone(crescent_Array* array);

extern void
crescentA_free(crescent_Array* array);

extern int
crescentA_resize(crescent_Array* array, size_t newLength);

extern int
crescentA_compare(crescent_Array* arrayA, crescent_Array* arrayB);

#endif
