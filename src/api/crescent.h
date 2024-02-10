/*
 * https://github.com/mochji/crescent
 * api/crescent.h
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

#ifndef API_CRESCENT_H
#define API_CRESCENT_H

#include <stdlib.h>
#include <stddef.h>
#include <setjmp.h>

#include "conf.h"

#include "core/object.h"
#include "core/state.h"
#include "core/call.h"

extern int
crescent_version();

extern int
crescent_release();

extern crescent_State*
crescent_openState();

extern void
crescent_closeState(crescent_State* state);

extern size_t
crescent_absoluteIndex(crescent_State* state, size_t index);

extern int
crescent_validIndex(crescent_State* state, size_t index);

extern size_t
crescent_getTop(crescent_State* state);

extern void
crescent_setTop(crescent_State* state);

extern crescent_Type
crescent_type(crescent_State* state, size_t index);

extern int
crescent_isNil(crescent_State* state, size_t index);

extern int
crescent_isInteger(crescent_State* state, size_t index);

extern int
crescent_isFloat(crescent_State* state, size_t index);

extern crescent_Integer
crescent_toIntegerX(crescent_State* state, size_t index, int* isInteger);

extern crescent_Float
crescent_toFloatX(crescent_State* state, size_t index, int* isFloat);

extern crescent_Integer
crescent_toInteger(crescent_State* state, size_t index);

extern crescent_Float
crescent_toFloat(crescent_State* state, size_t index);

extern void
crescent_pushNil(crescent_State* state);

extern void
crescent_pushInteger(crescent_State* state, crescent_Integer value);

extern void
crescent_pushFloat(crescent_State* state, crescent_Float value);

extern int
crescent_callC(crescent_State* state, int (*function)(crescent_State*));

extern int
crescent_pCallC(crescent_State* state, int (*function)(crescent_State*), crescent_Status* status);

extern void
crescent_error(crescent_State* state, char* error);

#endif
