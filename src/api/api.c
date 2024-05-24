/*
 * https://github.com/mochji/crescent
 * api/api.c
 *
 * idrk what this is
 * Copyright (C) 2024 mochji
 *
 * MIT License
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <setjmp.h>
#include <limits.h>

#include "conf.h"

#include "types/string.h"
#include "types/array.h"
#include "core/object.h"
#include "core/state.h"
#include "core/call.h"
#include "vm/vm.h"

/* TODO: throw errors on... stuff that cause errors idrk */

#define api_validindex(state, index) \
	(index > 0 && index <= state->stack.topFrame->top)

#define api_invalidindex(state, index) \
	(index == 0 || index > state->stack.topFrame->top)

#define api_absindex(state, index) \
	(state->stack.topFrame->base + index - 1)

#define api_abstop(state, index) \
	(state->stack.topFrame->base + state->stack.topFrame->top)

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
crescent_version(void) {
	return CRESCENT_VERSION;
}

int
crescent_release(void) {
	return CRESCENT_RELEASE;
}

crescent_State*
crescent_openState(void) {
	crescent_GState* gState = crescentG_blankGState();
	crescent_State*  state  = crescentG_blankLState();

	if (gState == NULL || state == NULL) {
		crescentG_closeGState(gState);
		crescentG_closeLState(state);

		return NULL;
	}

	gState->baseThread  = state;
	gState->panic       = &crescent_panic;

	state->gState = gState;

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
	return api_absindex(state, index);
}

int
crescent_validIndex(crescent_State* state, size_t index) {
	return api_validindex(state, index);
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
	crescentC_resizeStack(state, newTop, 1);

	size_t absoluteTop = state->stack.topFrame->base + newTop;
	size_t oldTop      = state->stack.topFrame->base + state->stack.topFrame->top;

	for (size_t a = oldTop; a < absoluteTop; a++) {
		state->stack.data[a].type = CRESCENT_TYPE_NIL;
	}

	state->stack.topFrame->top = newTop;
}

int
crescent_type(crescent_State* state, size_t index) {
	if (api_invalidindex(state, index)) {
		return CRESCENT_TYPE_NIL;
	}

	return state->stack.data[api_absindex(state, index)].type;
}

size_t
crescent_length(crescent_State* state, size_t index) {
	crescent_Object* object = &state->stack.data[api_absindex(state, index)];

	if (object->type == CRESCENT_TYPE_STRING) {
		return object->value.s->length;
	}

	if (object->type == CRESCENT_TYPE_ARRAY) {
		return object->value.a->length;
	}

	return 0;
}

const char*
crescent_typeName(int type) {
	return crescentO_typeName(type);
}

void
crescent_clone(crescent_State* state, size_t index) {
	size_t fromIndex = state->stack.topFrame->base + index - 1;
	size_t toIndex   = state->stack.topFrame->base + state->stack.topFrame->top;

	crescentC_resizeStack(state, state->stack.topFrame->top, 1);

	if (crescentO_clone(&state->stack.data[toIndex], &state->stack.data[fromIndex])) {
		crescentC_memoryError(state);
	}

	state->stack.topFrame->top += 1;
}

void
crescent_deepClone(crescent_State* state, size_t index) {
	size_t fromIndex = state->stack.topFrame->base + index - 1;
	size_t toIndex   = state->stack.topFrame->base + state->stack.topFrame->top;

	crescentC_resizeStack(state, state->stack.topFrame->top, 1);

	if (crescentO_deepClone(&state->stack.data[toIndex], &state->stack.data[fromIndex])) {
		crescentC_memoryError(state);
	}

	state->stack.topFrame->top += 1;
}

int
crescent_isNil(crescent_State* state, size_t index) {
	if (api_invalidindex(state, index)) {
		return 1;
	}

	return state->stack.data[api_absindex(state, index)].type == CRESCENT_TYPE_NIL;
}

int
crescent_isBoolean(crescent_State* state, size_t index) {
	if (api_invalidindex(state, index)) {
		return 0;
	}

	return state->stack.data[api_absindex(state, index)].type == CRESCENT_TYPE_BOOLEAN;
}

int
crescent_isInteger(crescent_State* state, size_t index) {
	if (api_invalidindex(state, index)) {
		return 0;
	}

	return state->stack.data[api_absindex(state, index)].type == CRESCENT_TYPE_INTEGER;
}

int
crescent_isFloat(crescent_State* state, size_t index) {
	if (api_invalidindex(state, index)) {
		return 0;
	}

	return state->stack.data[api_absindex(state, index)].type == CRESCENT_TYPE_FLOAT;
}

int
crescent_isNumber(crescent_State* state, size_t index) {
	if (api_invalidindex(state, index)) {
		return 0;
	}

	int type = state->stack.data[api_absindex(state, index)].type;

	return type == CRESCENT_TYPE_INTEGER || type == CRESCENT_TYPE_FLOAT;
}

int
crescent_isString(crescent_State* state, size_t index) {
	if (api_invalidindex(state, index)) {
		return 0;
	}

	return state->stack.data[api_absindex(state, index)].type == CRESCENT_TYPE_STRING;
}

int
crescent_isArray(crescent_State* state, size_t index) {
	if (api_invalidindex(state, index)) {
		return 0;
	}

	return state->stack.data[api_absindex(state, index)].type == CRESCENT_TYPE_ARRAY;
}

int
crescent_isCFunction(crescent_State* state, size_t index) {
	if (api_invalidindex(state, index)) {
		return 0;
	}

	return state->stack.data[api_absindex(state, index)].type == CRESCENT_TYPE_CFUNCTION;
}

int
crescent_toBooleanX(crescent_State* state, size_t index, int* isBoolean) {
	if (api_invalidindex(state, index)) {
		return 0;
	}

	return crescentO_toBoolean(&state->stack.data[api_absindex(state, index)], isBoolean);
}

crescent_Integer
crescent_toIntegerX(crescent_State* state, size_t index, int* isInteger) {
	if (api_invalidindex(state, index)) {
		return 0;
	}

	return crescentO_toInteger(&state->stack.data[api_absindex(state, index)], isInteger);
}

crescent_Float
crescent_toFloatX(crescent_State* state, size_t index, int* isFloat) {
	if (api_invalidindex(state, index)) {
		return 0;
	}

	return crescentO_toFloat(&state->stack.data[api_absindex(state, index)], isFloat);
}

const char*
crescent_toStringX(crescent_State* state, size_t index, int* isString) {
	if (api_invalidindex(state, index)) {
		return NULL;
	}

	return crescentO_toString(&state->stack.data[api_absindex(state, index)], isString, NULL);
}

int
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

const char*
crescent_toString(crescent_State* state, size_t index) {
	return crescent_toStringX(state, index, NULL);
}

crescent_CFunction*
crescent_toCFunction(crescent_State* state, size_t index) {
	if (api_invalidindex(state, index)) {
		return NULL;
	}

	return crescentO_toCFunction(&state->stack.data[api_absindex(state, index)], NULL);
}

void
crescent_pushNil(crescent_State* state) {
	size_t absoluteIndex = state->stack.topFrame->base + state->stack.topFrame->top;

	crescentC_resizeStack(state, ++state->stack.topFrame->top, 1);

	state->stack.data[absoluteIndex].type = CRESCENT_TYPE_NIL;
}

void
crescent_pushBoolean(crescent_State* state, int value) {
	size_t absoluteIndex = state->stack.topFrame->base + state->stack.topFrame->top;

	crescentC_resizeStack(state, ++state->stack.topFrame->top, 1);

	state->stack.data[absoluteIndex].type    = CRESCENT_TYPE_BOOLEAN;
	state->stack.data[absoluteIndex].value.b = value;
}

void
crescent_pushInteger(crescent_State* state, crescent_Integer value) {
	size_t absoluteIndex = state->stack.topFrame->base + state->stack.topFrame->top;

	crescentC_resizeStack(state, ++state->stack.topFrame->top, 1);

	state->stack.data[absoluteIndex].type    = CRESCENT_TYPE_INTEGER;
	state->stack.data[absoluteIndex].value.i = value;
}

void
crescent_pushFloat(crescent_State* state, crescent_Float value) {
	size_t absoluteIndex = state->stack.topFrame->base + state->stack.topFrame->top;

	crescentC_resizeStack(state, ++state->stack.topFrame->top, 1);

	state->stack.data[absoluteIndex].type    = CRESCENT_TYPE_FLOAT;
	state->stack.data[absoluteIndex].value.f = value;
}

void
crescent_pushString(crescent_State* state, const char* str) {
	size_t           absoluteIndex = state->stack.topFrame->base + state->stack.topFrame->top;
	crescent_String* string        = crescentS_as((char*)str);

	if (string == NULL) {
		crescentC_memoryError(state);
	}

	if (crescentC_resizeStack(state, state->stack.topFrame->top, 0)) {
		crescentS_free(string);
		crescentC_memoryError(state);
	}

	state->stack.data[absoluteIndex].type    = CRESCENT_TYPE_STRING;
	state->stack.data[absoluteIndex].value.s = string;

	state->stack.topFrame->top += 1;
}

void
crescent_pushArray(crescent_State* state) {
	size_t          absoluteIndex = state->stack.topFrame->base + state->stack.topFrame->top;
	crescent_Array* array         = crescentA_new(0);

	if (array == NULL) {
		crescentC_memoryError(state);
	}

	if (crescentC_resizeStack(state, state->stack.topFrame->top, 0)) {
		crescentA_free(array);
		crescentC_memoryError(state);
	}

	state->stack.data[absoluteIndex].type    = CRESCENT_TYPE_ARRAY;
	state->stack.data[absoluteIndex].value.a = array;

	state->stack.topFrame->top += 1;
}

void
crescent_pushCFunction(crescent_State* state, crescent_CFunction* value) {
	size_t absoluteIndex = state->stack.topFrame->base + state->stack.topFrame->top;

	crescentC_resizeStack(state, ++state->stack.topFrame->top, 1);

	state->stack.data[absoluteIndex].type    = CRESCENT_TYPE_CFUNCTION;
	state->stack.data[absoluteIndex].value.c = value;
}

void
crescent_pop(crescent_State* state, size_t amount) {
	if (amount > state->stack.topFrame->top) {
		amount = state->stack.topFrame->top;
	}

	crescentC_resizeStack(state, state->stack.topFrame->top - amount, 1);

	state->stack.topFrame->top -= amount;
}

void
crescent_remove(crescent_State* state, size_t index) {
	if (index == 0 || index > state->stack.topFrame->top) {
		return;
	}

	size_t baseIndex = api_absindex(state, index);

	for (size_t a = 0; a < state->stack.topFrame->top - index; a++) {
		state->stack.data[baseIndex + a] = state->stack.data[baseIndex + a + 1];
	}

	state->stack.data[baseIndex + state->stack.topFrame->top - index].type = CRESCENT_TYPE_NIL;

	crescentC_resizeStack(state, --state->stack.topFrame->top, 1);
}

int
crescent_call(crescent_State* state, size_t index, size_t argCount) {
	crescent_Object* object = &state->stack.data[api_absindex(state, index)];

	return crescentV_call(state, object, argCount, INT_MAX);
}

int
crescent_pCall(crescent_State* state, size_t index, size_t argCount, int* status) {
	crescent_Object* object = &state->stack.data[api_absindex(state, index)];

	return crescentV_pCall(state, object, argCount, INT_MAX, status);
}

int
crescent_callK(crescent_State* state, size_t index, size_t argCount, int maxResults) {
	crescent_Object* object = &state->stack.data[api_absindex(state, index)];

	return crescentV_call(state, object, argCount, maxResults);
}

int
crescent_pCallK(crescent_State* state, size_t index, size_t argCount, int maxResults, int* status) {
	crescent_Object* object = &state->stack.data[api_absindex(state, index)];

	return crescentV_pCall(state, object, argCount, maxResults, status);
}

int
crescent_error(crescent_State* state, const char* error) {
	crescentC_setError(state, (char*)error);
	crescentC_throw(state, CRESCENT_STATUS_ERROR);
}

void
crescent_clearError(crescent_State* state) {
	crescentC_setError(state, NULL);
}

void
crescent_pushError(crescent_State* state) {
	size_t           absoluteIndex = state->stack.topFrame->base + state->stack.topFrame->top;
	size_t           errorLength   = strlen(state->error);
	crescent_String* string        = crescentS_nullString();

	if (string == NULL) {
		crescentC_memoryError(state);
	}

	if (crescentC_resizeStack(state, state->stack.topFrame->top, 0)) {
		crescentS_free(string);
		crescentC_memoryError(state);
	}

	string->size   = errorLength + 1;
	string->length = errorLength;
	string->data   = state->error;

	state->stack.data[absoluteIndex].type    = CRESCENT_TYPE_STRING;
	state->stack.data[absoluteIndex].value.s = string;

	state->error = NULL;

	state->stack.topFrame->top += 1;
}

const char*
crescent_getError(crescent_State* state) {
	return state->error;
}
