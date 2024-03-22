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

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <setjmp.h>
#include <limits.h>

#include "conf.h"

#include "types/string.h"
#include "core/object.h"
#include "core/state.h"
#include "core/call.h"

static int
crescent_panic(crescent_State* state) {
	char* error;

	if (state->error != NULL) {
		error = state->error;
	} else {
		error = "no error";
	}

	fprintf(stderr, "PANIC: error within unprotected call to Crescent API (%s)\n", error);

	return 0;
}

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

	if (gState == NULL || state == NULL) {
		free(state);
		free(gState);

		return NULL;
	}

	gState->threadCount = 1;
	gState->threads[0]  = state;
	gState->baseThread  = state;

	state->gState = gState;

	gState->panic = &crescent_panic;

	return state;
}

void
crescent_closeState(crescent_State* state) {
	crescentG_closeGState(state->gState);
}

void
crescent_setPanic(crescent_State* state, crescent_CFunction* function) {
	state->gState->panic = function;
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

size_t
crescent_getSize(crescent_State* state) {
	return state->stack.size;
}

int
crescent_getUsage(crescent_State* state) {
	size_t absoluteTop = state->stack.topFrame->base + state->stack.topFrame->top;

	return (absoluteTop * 100 + state->stack.size / 2) / state->stack.size;
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

int
crescent_type(crescent_State* state, size_t index) {
	return state->stack.data[state->stack.topFrame->base + index - 1].type;
}

char*
crescent_typeName(int type) {
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
		case CRESCENT_TYPE_STRING:
			return "string";

			break;
		case CRESCENT_TYPE_CFUNCTION:
			return "cfunction";

			break;
	}

	return NULL;
}

int
crescent_isNil(crescent_State* state, size_t index) {
	size_t absoluteIndex = state->stack.topFrame->base + index - 1;

	return state->stack.data[absoluteIndex].type == CRESCENT_TYPE_NIL;
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

int
crescent_isNumber(crescent_State* state, size_t index) {
	size_t absoluteIndex = state->stack.topFrame->base + index - 1;
	int    type          = state->stack.data[absoluteIndex].type;

	return type == CRESCENT_TYPE_INTEGER || type == CRESCENT_TYPE_FLOAT;
}

int
crescent_isString(crescent_State* state, size_t index) {
	size_t absoluteIndex = state->stack.topFrame->base + index - 1;

	return state->stack.data[absoluteIndex].type == CRESCENT_TYPE_STRING;
}

int
crescent_isCFunction(crescent_State* state, size_t index) {
	size_t absoluteIndex = state->stack.topFrame->base + index - 1;

	return state->stack.data[absoluteIndex].type == CRESCENT_TYPE_CFUNCTION;
}

crescent_Boolean
crescent_toBooleanX(crescent_State* state, size_t index, int* isBoolean) {
	size_t           absoluteIndex = state->stack.topFrame->base + index - 1;
	crescent_Object* object;

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

	if (isBoolean != NULL) {
		*isBoolean = 0;
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

	if (isInteger != NULL) {
		*isInteger = 0;
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

	if (isFloat != NULL) {
		*isFloat = 0;
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

char*
crescent_toString(crescent_State* state, size_t index) {
	size_t           absoluteIndex = state->stack.topFrame->base + index - 1;
	crescent_Object* object;

	if (index == 0 || index > state->stack.topFrame->top) {
		return NULL;
	}

	object = &state->stack.data[absoluteIndex];

	if (object->type == CRESCENT_TYPE_STRING) {
		return object->value.s->data;
	}

	return NULL;
}

crescent_CFunction*
crescent_toCFunction(crescent_State* state, size_t index) {
	size_t absoluteIndex = state->stack.topFrame->base + index;

	if (index == 0 || index > state->stack.topFrame->top) {
		return NULL;
	}

	if (state->stack.data[absoluteIndex].type == CRESCENT_TYPE_CFUNCTION) {
		return state->stack.data[absoluteIndex].value.c;
	}

	return NULL;
}

void
crescent_pushNil(crescent_State* state) {
	size_t absoluteIndex = state->stack.topFrame->base + state->stack.topFrame->top;

	crescentC_resizeStack(state, state->stack.topFrame->top++);

	state->stack.data[absoluteIndex].type = CRESCENT_TYPE_NIL;
}

void
crescent_pushBoolean(crescent_State* state, crescent_Boolean value) {
	size_t absoluteIndex = state->stack.topFrame->base + state->stack.topFrame->top;

	crescentC_resizeStack(state, state->stack.topFrame->top++);

	state->stack.data[absoluteIndex].type    = CRESCENT_TYPE_BOOLEAN;

	if (value) {
		state->stack.data[absoluteIndex].value.b = 1;
	} else {
		state->stack.data[absoluteIndex].value.b = 0;
	}
}

void
crescent_pushInteger(crescent_State* state, crescent_Integer value) {
	size_t absoluteIndex = state->stack.topFrame->base + state->stack.topFrame->top;

	crescentC_resizeStack(state, state->stack.topFrame->top++);

	state->stack.data[absoluteIndex].type    = CRESCENT_TYPE_INTEGER;
	state->stack.data[absoluteIndex].value.i = value;
}

void
crescent_pushFloat(crescent_State* state, crescent_Float value) {
	size_t absoluteIndex = state->stack.topFrame->base + state->stack.topFrame->top;

	crescentC_resizeStack(state, state->stack.topFrame->top++);

	state->stack.data[absoluteIndex].type    = CRESCENT_TYPE_FLOAT;
	state->stack.data[absoluteIndex].value.f = value;
}

void
crescent_pushString(crescent_State* state, char* str) {
	size_t           absoluteIndex = state->stack.topFrame->base + state->stack.topFrame->top;
	size_t           length        = 0;
	crescent_String* string;

	crescentC_resizeStack(state, state->stack.topFrame->top++);

	while (str[length]) {
		length++;
	}

	string = crescentS_new(length);

	if (string == NULL) {
		crescentC_memoryError(state);

		return;
	}

	for (size_t a = 0; a < length; a++) {
		string->data[a] = str[a];
	}

	state->stack.data[absoluteIndex].type    = CRESCENT_TYPE_STRING;
	state->stack.data[absoluteIndex].value.s = string;
}

void
crescent_pushCFunction(crescent_State* state, crescent_CFunction* function) {
	size_t absoluteIndex = state->stack.topFrame->base + state->stack.topFrame->top;

	crescentC_resizeStack(state, state->stack.topFrame->top++);

	state->stack.data[absoluteIndex].type    = CRESCENT_TYPE_CFUNCTION;
	state->stack.data[absoluteIndex].value.c = function;
}

void
crescent_pop(crescent_State* state, size_t amount) {
	if (state->stack.topFrame->top < amount) {
		amount = state->stack.topFrame->top;
	}

	size_t topIndex = state->stack.topFrame->base + state->stack.topFrame->top - 1;

	for (size_t a = 0; a < amount; a++) {
		crescentO_free(&state->stack.data[topIndex - a]);
	}

	state->stack.topFrame->top -= amount;

	crescentC_resizeStack(state, state->stack.topFrame->top);
}

void
crescent_remove(crescent_State* state, size_t index) {
	if (index == 0 || index > state->stack.topFrame->top) {
		return;
	}

	size_t baseIndex = state->stack.topFrame->base + index - 1;

	for (size_t a = 0; a < state->stack.topFrame->top - index; a++) {
		state->stack.data[baseIndex + a] = state->stack.data[baseIndex + a + 1];
	}

	state->stack.data[baseIndex + state->stack.topFrame->top - index].type = CRESCENT_TYPE_NONE;

	crescentC_resizeStack(state, --state->stack.topFrame->top);
}

int
crescent_callC(crescent_State* state, crescent_CFunction* function, size_t argCount) {
	if (argCount > state->stack.topFrame->top) {
		argCount = state->stack.topFrame->top;
	}

	return crescentC_callC(state, function, argCount, INT_MAX);
}

int
crescent_pCallC(crescent_State* state, crescent_CFunction* function, size_t argCount, int* status) {
	if (argCount > state->stack.topFrame->top) {
		argCount = state->stack.topFrame->top;
	}

	return crescentC_pCallC(state, function, argCount, INT_MAX, status);
}

/* TODO: error messages on attempt to call non function, needs format string func */

int
crescent_call(crescent_State* state, size_t index, size_t argCount) {
	if (argCount > state->stack.topFrame->top) {
		argCount = state->stack.topFrame->top;
	}

	size_t              absoluteIndex = state->stack.topFrame->base + index - 1;
	crescent_CFunction* function      = state->stack.data[absoluteIndex].value.c;

	return crescentC_callC(state, function, argCount, INT_MAX);
}

int
crescent_pCall(crescent_State* state, size_t index, size_t argCount, int* status) {
	if (argCount > state->stack.topFrame->top) {
		argCount = state->stack.topFrame->top;
	}

	size_t              absoluteIndex = state->stack.topFrame->base + index - 1;
	crescent_CFunction* function      = state->stack.data[absoluteIndex].value.c;

	return crescentC_pCallC(state, function, argCount, INT_MAX, status);
}

int
crescent_callK(crescent_State* state, size_t index, size_t argCount, int results) {
	if (argCount > state->stack.topFrame->top) {
		argCount = state->stack.topFrame->top;
	}

	if (results < 0) {
		results = 0;
	}

	size_t              absoluteIndex = state->stack.topFrame->base + index - 1;
	crescent_CFunction* function      = state->stack.data[absoluteIndex].value.c;

	return crescentC_callC(state, function, argCount, results);
}

int
crescent_pCallK(crescent_State* state, size_t index, size_t argCount, int results, int* status) {
	if (argCount > state->stack.topFrame->top) {
		argCount = state->stack.topFrame->top;
	}

	if (results < 0) {
		results = 0;
	}

	size_t              absoluteIndex = state->stack.topFrame->base + index - 1;
	crescent_CFunction* function      = state->stack.data[absoluteIndex].value.c;

	return crescentC_pCallC(state, function, argCount, results, status);
}

void
crescent_error(crescent_State* state, char* error) {
	crescentC_setError(state, error);
	crescentC_throw(state, CRESCENT_STATUS_ERROR);
}

void
crescent_clearError(crescent_State* state) {
	free(state->error);

	state->error = NULL;
}

char*
crescent_getError(crescent_State* state) {
	return state->error;
}
