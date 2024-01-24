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
#include <stdint.h>

#include "conf.h"

#include "core/object.h"
#include "core/state.h"

crescent_State*
crescent_openState();

extern void
crescent_closeState(crescent_State* state);

extern int
crescent_setTop(crescent_State* state, size_t newTop);

extern size_t
crescent_getTop(crescent_State* state);

extern size_t
crescent_isValidIndex(crescent_State* state, size_t index);

extern crescent_Type
crescent_getType(crescent_State* state, size_t index);

extern int
crescent_isInteger(crescent_State* state, size_t index);

extern int
crescent_isFloat(crescent_State* state, size_t index);

extern int
crescent_isNil(crescent_State* state, size_t index);

extern crescent_Integer
crescent_toIntegerX(crescent_State* state, size_t index, int* isInteger);

extern crescent_Float
crescent_toFloatX(crescent_State* state, size_t index, int* isFloat);

extern crescent_Integer
crescent_toInteger(crescent_State* state, size_t index);

extern crescent_Float
crescent_toFloat(crescent_State* state, size_t index);

#endif
