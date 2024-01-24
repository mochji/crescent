/*
 * https://github.com/mochji/crescent
 * api/crescent.c
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

#include <stdlib.h>
#include <stdint.h>

#include "conf.h"

#include "core/object.h"
#include "core/state.h"

int
crescent_version() {
	return CRESCENT_VERSION;
}

int
crescent_release() {
	return CRESCENT_RELEASE;
}

crescent_State*
crescent_openState() {
	crescent_State* state = malloc(sizeof(crescent_State));

	if (state == NULL) {
		return NULL;
	}

	state->stack.size = CRESCENT_CONF_STACK_MAXFREE;
	state->stack.top  = 0;
	state->stack.data = malloc(state->stack.size * sizeof(crescent_Object));

	if (state->stack.data == NULL) {
		free(state);

		return NULL;
	}

	for (size_t a = 0; a < state->stack.size; a++) {
		state->stack.data[a].type    = CRESCENT_TYPE_NIL;
		state->stack.data[a].value.i = 0;
	}

	return state;
}

void
crescent_closeState(crescent_State* state) {
	free(state->stack.data);
	free(state);
}

int
crescent_setTop(crescent_State* state, size_t newTop) {
	if (newTop <= state->stack.size) {
		size_t freeSpace = state->stack.size - newTop;

		if (freeSpace <= CRESCENT_CONF_STACK_MINFREE) {
			return 0;
		}

		if (freeSpace >= CRESCENT_CONF_STACK_MAXFREE) {
			return 0;
		}
	}

	size_t           newSize;
	crescent_Object* newData;

	if (newTop >= state->stack.size) {
		newSize = state->stack.size * 2;
	} else {
		newSize = state->stack.size / 2;
	}

	newData = realloc(state->stack.data, newSize);

	if (newData == NULL) {
		return 1;
	}

	state->stack.size = newSize;
	state->stack.top  = newTop;
	state->stack.data = newData;

	return 0;
}

size_t
crescent_getTop(crescent_State* state) {
	return state->stack.size;
}

size_t
crescent_isValidIndex(crescent_State* state, size_t index) {
	return (index > 0 && index <= state->stack.top);
}

crescent_Type
crescent_getType(crescent_State* state, size_t index) {
	if (index == 0 || index > state->stack.top) {
		return CRESCENT_TYPE_NOVALUE;
	}

	return state->stack.data[index - 1].type;
}

int
crescent_isInteger(crescent_State* state, size_t index) {
	if (index == 0 || index > state->stack.top) {
		return 0;
	}

	return state->stack.data[index - 1].type == CRESCENT_TYPE_INTEGER;
}

int
crescent_isFloat(crescent_State* state, size_t index) {
	if (index == 0 || index > state->stack.top) {
		return 0;
	}

	return state->stack.data[index - 1].type == CRESCENT_TYPE_FLOAT;
}

int
crescent_isNil(crescent_State* state, size_t index) {
	if (index == 0 || index > state->stack.top) {
		return 0;
	}

	return state->stack.data[index - 1].type == CRESCENT_TYPE_NIL;
}
