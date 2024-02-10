/*
 * https://github.com/mochji/crescent
 * core/call.h
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

#ifndef CORE_CALL_H
#define CORE_CALL_H

#include <stdlib.h>
#include <stddef.h>
#include <setjmp.h>

#include "conf.h"

#include "core/object.h"
#include "core/state.h"

extern void
crescentC_setError(crescent_State* state, char* error);

extern void
crescentC_throw(crescent_State* state, crescent_Status status);

extern void
crescentC_memoryError(crescent_State* state);

extern int
crescentC_stackUsage(crescent_State* state);

extern void
crescentC_resizeStack(crescent_State* state, size_t newTop);

extern size_t
crescentC_callC(crescent_State* state, int (*function)(crescent_State*), size_t argCount);

extern size_t
crescentC_pCallC(crescent_State* state, int (*function)(crescent_State*), size_t argCount, crescent_Status* status);

#endif
