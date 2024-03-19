/*
 * https://github.com/mochji/crescent
 * core/object.h
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

#ifndef CORE_OBJECT_H
#define CORE_OBJECT_H

#include <stddef.h>

#include "conf.h"

#include "types/string.h"

union
crescent_Value {
	crescent_Boolean    b;
	crescent_Integer    i;
	crescent_Float      f;
	crescent_String*    s;
	crescent_CFunction* c;
};

struct
crescent_Object {
	unsigned char        type;
	union crescent_Value value;
};

typedef union  crescent_Value  crescent_Value;
typedef struct crescent_Object crescent_Object;

extern int
crescentO_compare(crescent_Object* a, crescent_Object* b);

extern int
crescentO_clone(crescent_Object* to, crescent_Object* from);

extern int
crescentO_deepClone(crescent_Object* to, crescent_Object* from);

extern void
crescentO_free(crescent_Object* object);

#endif
