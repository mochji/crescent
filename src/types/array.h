/*
 * https://github.com/mochji/crescent
 * types/array.h
 *
 * idrk what this is
 * Copyright (C) 2024 mochji
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef TYPES_ARRAY_H
#define TYPES_ARRAY_H

#include <stddef.h>

#include "conf.h"

#include "core/object.h"

struct
crescent_Array {
	size_t                  size;
	size_t                  length;
	struct crescent_Object* data;
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
