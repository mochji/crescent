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
#include <stddef.h>
#include <setjmp.h>

#include "conf.h"

#include "core/object.h"
#include "core/state.h"
#include "core/call.h"

#include "api/crescent.h"

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
	crescent_GState* gState = crescentG_blankGState();
	crescent_State*  state  = crescentG_blankLState();

	gState->threadCount = 1;
	gState->threads[0]  = state;
	gState->baseThread  = state;

	state->gState = gState;

	return state;
}

void
crescent_closeState(crescent_State* state) {
	crescentG_closeGState(state->gState);
}

size_t
crescent_absoluteIndex(crescent_State* state, size_t index) {
	return state->stack.topFrame->base + index - 1;
}

int
crescent_validIndex(crescent_State* state, size_t index) {
	return index > 0 && index <= state->stack.topFrame->top;
}

size_t
crescent_getTop(crescent_State* state) {
	return state->stack.topFrame->top;
}

void
crescent_setTop(crescent_State* state, size_t newTop) {
	size_t oldTop = state->stack.topFrame->top;

	crescentC_resizeStack(state, newTop);

	state->stack.topFrame->top = newTop;

	for (size_t a = oldTop; a < newTop; a++) {
		state->stack.data[state->stack.topFrame->base + a].type = CRESCENT_TYPE_NIL;
	}
}

crescent_Status
crescent_type(crescent_State* state, size_t index) {
	return state->stack.data[state->stack.topFrame->base + index].type;
}

char*
crescent_typeName(crescent_Status type) {
	switch (type) {
		case CRESCENT_TYPE_NONE:
			return "no value";

			break;
		case CRESCENT_TYPE_NIL:
			return "nil";

			break;
		case CRESCENT_TYPE_BOOLEAN:
			return "boolean";

			break;
		case CRESCENT_TYPE_INTEGER:
			return "integer";

			break;
		case CRESCENT_TYPE_FLOAT:
			return "float";

			break;
	}

	return NULL;
}

int
crescent_isNil(crescent_State* state, size_t index) {
	return state->stack.data[state->stack.topFrame->base + index].type == CRESCENT_TYPE_NIL;
}

int
crescent_isBoolean(crescent_State* state, size_t index) {
	size_t absoluteIndex = state->stack.topFrame->base + index - 1;

	return state->stack.data[absoluteIndex].type == CRESCENT_TYPE_BOOLEAN;
}

int
crescent_isInteger(crescent_State* state, size_t index) {
	size_t absoluteIndex = state->stack.topFrame->base + index - 1;

	return state->stack.data[absoluteIndex].type == CRESCENT_TYPE_INTEGER;
}

int
crescent_isFloat(crescent_State* state, size_t index) {
	size_t absoluteIndex = state->stack.topFrame->base + index - 1;

	return state->stack.data[absoluteIndex].type == CRESCENT_TYPE_FLOAT;
}

crescent_Boolean
crescent_toBooleanX(crescent_State* state, size_t index, int* isBoolean) {
	size_t           absoluteIndex = state->stack.topFrame->base + index - 1;
	crescent_Object* object;

	if (isBoolean != NULL) {
		*isBoolean = 0;
	}

	if (index == 0 || index > state->stack.topFrame->top) {
		return 0;
	}

	object = &state->stack.data[absoluteIndex];

	if (object->type == CRESCENT_TYPE_BOOLEAN) {
		if (isBoolean != NULL) {
			*isBoolean = 1;
		}

		return object->value.b;
	}

	if (object->type == CRESCENT_TYPE_INTEGER) {
		if (object->value.i) {
			return 1;
		}

		return 0;
	}

	if (object->type == CRESCENT_TYPE_FLOAT) {
		if (object->value.f) {
			return 1;
		}

		return 0;
	}

	return 0;
}

crescent_Integer
crescent_toIntegerX(crescent_State* state, size_t index, int* isInteger) {
	size_t           absoluteIndex = state->stack.topFrame->base + index - 1;
	crescent_Object* object;

	if (isInteger != NULL) {
		*isInteger = 0;
	}

	if (index == 0 || index > state->stack.topFrame->top) {
		return 0;
	}

	object = &state->stack.data[absoluteIndex];

	if (object->type == CRESCENT_TYPE_INTEGER) {
		if (isInteger != NULL) {
			*isInteger = 1;
		}

		return object->value.i;
	}

	if (object->type == CRESCENT_TYPE_BOOLEAN) {
		return (crescent_Integer)object->value.b;
	}

	if (object->type == CRESCENT_TYPE_FLOAT) {
		return (crescent_Integer)object->value.f;
	}

	return 0;
}

crescent_Float
crescent_toFloatX(crescent_State* state, size_t index, int* isFloat) {
	size_t           absoluteIndex = state->stack.topFrame->base + index - 1;
	crescent_Object* object;

	if (isFloat != NULL) {
		*isFloat = 0;
	}

	if (index == 0 || index > state->stack.topFrame->top) {
		return 0;
	}

	object = &state->stack.data[absoluteIndex];

	if (object->type == CRESCENT_TYPE_FLOAT) {
		if (isFloat != NULL) {
			*isFloat = 1;
		}

		return object->value.f;
	}

	if (object->type == CRESCENT_TYPE_BOOLEAN) {
		return (crescent_Float)object->value.b;
	}

	if (object->type == CRESCENT_TYPE_INTEGER) {
		return (crescent_Float)object->value.i;
	}

	return 0;
}

crescent_Boolean
crescent_toBoolean(crescent_State* state, size_t index) {
	return crescent_toBooleanX(state, index, NULL);
}

crescent_Integer
crescent_toInteger(crescent_State* state, size_t index) {
	return crescent_toIntegerX(state, index, NULL);
}

crescent_Float
crescent_toFloat(crescent_State* state, size_t index) {
	return crescent_toFloatX(state, index, NULL);
}

void
crescent_pushNil(crescent_State* state) {
	crescent_setTop(state, state->stack.topFrame->top + 1);
}

void
crescent_pushBoolean(crescent_State* state, crescent_Boolean value) {
	size_t index = state->stack.topFrame->base + state->stack.topFrame->top;

	crescentC_resizeStack(state, state->stack.topFrame->top + 1);

	state->stack.topFrame->top += 1;

	state->stack.data[index].type    = CRESCENT_TYPE_BOOLEAN;
	state->stack.data[index].value.b = value;
}

void
crescent_pushInteger(crescent_State* state, crescent_Integer value) {
	size_t index = state->stack.topFrame->base + state->stack.topFrame->top;

	crescentC_resizeStack(state, state->stack.topFrame->top + 1);

	state->stack.topFrame->top += 1;

	state->stack.data[index].type    = CRESCENT_TYPE_INTEGER;
	state->stack.data[index].value.i = value;
}

void
crescent_pushFloat(crescent_State* state, crescent_Float value) {
	size_t index = state->stack.topFrame->base + state->stack.topFrame->top;

	crescentC_resizeStack(state, state->stack.topFrame->top + 1);

	state->stack.topFrame->top += 1;

	state->stack.data[index].type    = CRESCENT_TYPE_FLOAT;
	state->stack.data[index].value.f = value;
}

int
crescent_callC(crescent_State* state, int (*function)(crescent_State*), size_t argCount) {
	if (argCount > state->stack.topFrame->top) {
		argCount = state->stack.topFrame->top;
	}

	return crescentC_callC(state, function, argCount);
}

int
crescent_pCallC(crescent_State* state, int (*function)(crescent_State*), size_t argCount, crescent_Status* status) {
	if (argCount > state->stack.topFrame->top) {
		argCount = state->stack.topFrame->top;
	}

	return crescentC_pCallC(state, function, argCount, status);
}

void
crescent_error(crescent_State* state, char* error) {
	crescentC_setError(state, error);
	crescentC_throw(state, CRESCENT_STATUS_ERROR);
}

char*
crescent_getError(crescent_State* state) {
	return state->error;
}
