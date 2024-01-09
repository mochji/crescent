/*
 * https://github.com/mochji/crescent
 * core/state.h
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

#ifndef CORE_STATE_H
#define CORE_STATE_H

#include <stdlib.h>
#include <stddef.h>

#include "../conf.h"

#include "object.h"

struct crescent_State {
	struct crescent_Stack {
		size_t           size;
		size_t           base;
		size_t           pointer;
		crescent_Object* data;
	} stack;
};

#endif
