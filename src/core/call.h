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

#include "conf.h"

#include "core/object.h"
#include "core/state.h"

extern void
crescentC_setError(crescent_State* state, char* error);

extern void
crescentC_throw(crescent_State* state, int status);

extern void
crescentC_memoryError(crescent_State* state);

extern int
crescentC_stackUsage(crescent_State* state);

extern void
crescentC_growStack(crescent_State* state, size_t usage);

extern void
crescentC_shrinkStack(crescent_State* state, size_t usage);

extern void
crescentC_resizeStack(crescent_State* state, size_t newTop);

extern void
crescentC_startCall(crescent_State* state, size_t argCount);

extern void
crescentC_endCall(crescent_State* state, size_t results);

extern void
crescentC_startTry(crescent_State* state);

extern void
crescentC_endTry(crescent_State* state);

extern int
crescentC_callC(crescent_State* state, crescent_CFunction* function, size_t argCount, int maxResults);

extern int
crescentC_pCallC(crescent_State* state, crescent_CFunction* function, size_t argCount, int maxResults, int* status);

#endif
