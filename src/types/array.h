/*
 * https://github.com/mochji/crescent
 * types/array.h
 *
 * idrk what this is
 * Copyright (C) 2024 mochji
 *
 * MIT License
 */

#ifndef CRESCENT_TYPES_ARRAY_H
#define CRESCENT_TYPES_ARRAY_H

#include <stddef.h>

#include "conf.h"

#include "core/object.h"

struct
crescent_Array {
	size_t                  size;
	size_t                  length;
	struct crescent_Object* value;
	size_t                  references;
};

typedef struct crescent_Array crescent_Array;

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
