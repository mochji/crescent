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

#include "core/state.h"

struct
crescent_String {
	size_t size;
	size_t length;
	char*  data;
};

typedef int                    crescent_Boolean;
typedef CRESCENT_INTEGER       crescent_Integer;
typedef CRESCENT_FLOAT         crescent_Float;
typedef struct crescent_String crescent_String;
typedef int                   (crescent_CFunction)(crescent_State*);

enum
crescent_Type {
	CRESCENT_TYPE_NONE,
	CRESCENT_TYPE_NIL,
	CRESCENT_TYPE_BOOLEAN,
	CRESCENT_TYPE_INTEGER,
	CRESCENT_TYPE_FLOAT,
	CRESCENT_TYPE_STRING,
	CRESCENT_TYPE_CFUNCTION
};

union
crescent_Value {
	crescent_Boolean    b;
	crescent_Integer    i;
	crescent_Float      f;
	crescent_String*    s;
	crescent_CFunction* cFunc;
};

struct
crescent_Object {
	enum  crescent_Type  type;
	union crescent_Value value;
};

typedef enum   crescent_Type   crescent_Type;
typedef union  crescent_Value  crescent_Value;
typedef struct crescent_Object crescent_Object;

#endif
