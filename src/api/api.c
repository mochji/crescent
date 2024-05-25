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

static crescent_Object*
crescent_getIndex(crescent_State* state, int index) {
	if (index == 0) {
		return &state->gState->nilValue;
	}

	if (index < 0) {
		index = state->stack.topFrame->top + index + 1;
	}

	return index <= state->stack.topFrame->top ?
		&state->stack.data[state->stack.topFrame->base + index - 1] :
		&state->gState->nilValue;
}

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

int
crescent_validIndex(crescent_State* state, int index) {
	if (index == 0) {
		return 0;
	}

	if (index < 0) {
		return -index <= state->stack.topFrame->top;
	}

	return index <= state->stack.topFrame->top;
}

int
crescent_getTop(crescent_State* state) {
	return state->stack.topFrame->top;
}

void
crescent_setTop(crescent_State* state, int newTop) {
	crescentC_resizeStack(state, newTop, 1);

	unsigned int absoluteTop = state->stack.topFrame->base + newTop;
	unsigned int oldTop      = state->stack.topFrame->base + state->stack.topFrame->top;

	for (unsigned int a = oldTop; a < absoluteTop; a++) {
		state->stack.data[a].type = CRESCENT_TYPE_NIL;
	}

	state->stack.topFrame->top = newTop;
}

int
crescent_type(crescent_State* state, int index) {
	return crescent_getIndex(state, index)->type;
}

size_t
crescent_length(crescent_State* state, int index) {
	crescent_Object* object = crescent_getIndex(state, index);

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
crescent_clone(crescent_State* state, int index) {
	crescent_Object* object  = crescent_getIndex(state, index);
	unsigned int     toIndex = state->stack.topFrame->base + state->stack.topFrame->top;

	crescentC_resizeStack(state, state->stack.topFrame->top + 1, 1);

	if (crescentO_clone(&state->stack.data[toIndex], object)) {
		crescentC_memoryError(state);
	}

	state->stack.topFrame->top += 1;
}

void
crescent_deepClone(crescent_State* state, int index) {
	crescent_Object* object  = crescent_getIndex(state, index);
	unsigned int     toIndex = state->stack.topFrame->base + state->stack.topFrame->top;

	crescentC_resizeStack(state, state->stack.topFrame->top + 1, 1);

	if (crescentO_deepClone(&state->stack.data[toIndex], object)) {
		crescentC_memoryError(state);
	}

	state->stack.topFrame->top += 1;
}

int
crescent_isNil(crescent_State* state, int index) {
	return crescent_getIndex(state, index)->type == CRESCENT_TYPE_NIL;
}

int
crescent_isBoolean(crescent_State* state, int index) {
	return crescent_getIndex(state, index)->type == CRESCENT_TYPE_BOOLEAN;
}

int
crescent_isInteger(crescent_State* state, int index) {
	return crescent_getIndex(state, index)->type == CRESCENT_TYPE_INTEGER;
}

int
crescent_isFloat(crescent_State* state, int index) {
	return crescent_getIndex(state, index)->type == CRESCENT_TYPE_FLOAT;
}

int
crescent_isNumber(crescent_State* state, int index) {
	int type = crescent_getIndex(state, index)->type;

	return type == CRESCENT_TYPE_INTEGER || type == CRESCENT_TYPE_FLOAT;
}

int
crescent_isString(crescent_State* state, int index) {
	return crescent_getIndex(state, index)->type == CRESCENT_TYPE_STRING;
}

int
crescent_isArray(crescent_State* state, int index) {
	return crescent_getIndex(state, index)->type == CRESCENT_TYPE_ARRAY;
}

int
crescent_isCFunction(crescent_State* state, int index) {
	return crescent_getIndex(state, index)->type == CRESCENT_TYPE_CFUNCTION;
}

int
crescent_toBooleanX(crescent_State* state, int index, int* isBoolean) {
	return crescentO_toBoolean(crescent_getIndex(state, index), isBoolean);
}

crescent_Integer
crescent_toIntegerX(crescent_State* state, int index, int* isInteger) {
	return crescentO_toInteger(crescent_getIndex(state, index), isInteger);
}

crescent_Float
crescent_toFloatX(crescent_State* state, int index, int* isFloat) {
	return crescentO_toFloat(crescent_getIndex(state, index), isFloat);
}

const char*
crescent_toStringX(crescent_State* state, int index, int* isString) {
	return crescentO_toString(crescent_getIndex(state, index), isString, NULL);
}

int
crescent_toBoolean(crescent_State* state, int index) {
	return crescentO_toBoolean(crescent_getIndex(state, index), NULL);
}

crescent_Integer
crescent_toInteger(crescent_State* state, int index) {
	return crescentO_toInteger(crescent_getIndex(state, index), NULL);
}

crescent_Float
crescent_toFloat(crescent_State* state, int index) {
	return crescentO_toFloat(crescent_getIndex(state, index), NULL);
}

const char*
crescent_toString(crescent_State* state, int index) {
	return crescentO_toString(crescent_getIndex(state, index), NULL, NULL);
}

crescent_CFunction*
crescent_toCFunction(crescent_State* state, int index) {
	return crescentO_toCFunction(crescent_getIndex(state, index), NULL);
}

void
crescent_pushNil(crescent_State* state) {
	unsigned int absoluteIndex = state->stack.topFrame->base + state->stack.topFrame->top;

	crescentC_resizeStack(state, state->stack.topFrame->top + 1, 1);

	state->stack.data[absoluteIndex].type = CRESCENT_TYPE_NIL;
	state->stack.topFrame->top           += 1;
}

void
crescent_pushBoolean(crescent_State* state, int value) {
	unsigned int absoluteIndex = state->stack.topFrame->base + state->stack.topFrame->top;

	crescentC_resizeStack(state, state->stack.topFrame->top + 1, 1);

	state->stack.data[absoluteIndex].type    = CRESCENT_TYPE_BOOLEAN;
	state->stack.data[absoluteIndex].value.b = value;
	state->stack.topFrame->top              += 1;
}

void
crescent_pushInteger(crescent_State* state, crescent_Integer value) {
	unsigned int absoluteIndex = state->stack.topFrame->base + state->stack.topFrame->top;

	crescentC_resizeStack(state, state->stack.topFrame->top + 1, 1);

	state->stack.data[absoluteIndex].type    = CRESCENT_TYPE_INTEGER;
	state->stack.data[absoluteIndex].value.i = value;
	state->stack.topFrame->top              += 1;
}

void
crescent_pushFloat(crescent_State* state, crescent_Float value) {
	unsigned int absoluteIndex = state->stack.topFrame->base + state->stack.topFrame->top;

	crescentC_resizeStack(state, state->stack.topFrame->top + 1, 1);

	state->stack.data[absoluteIndex].type    = CRESCENT_TYPE_FLOAT;
	state->stack.data[absoluteIndex].value.f = value;
	state->stack.topFrame->top              += 1;
}

void
crescent_pushString(crescent_State* state, const char* str) {
	unsigned int     absoluteIndex = state->stack.topFrame->base + state->stack.topFrame->top;
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
	unsigned int    absoluteIndex = state->stack.topFrame->base + state->stack.topFrame->top;
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
	unsigned int absoluteIndex = state->stack.topFrame->base + state->stack.topFrame->top;

	crescentC_resizeStack(state, state->stack.topFrame->top + 1, 1);

	state->stack.data[absoluteIndex].type    = CRESCENT_TYPE_CFUNCTION;
	state->stack.data[absoluteIndex].value.c = value;
	state->stack.topFrame->top              += 1;
}

void
crescent_pop(crescent_State* state, int amount) {
	if (amount > state->stack.topFrame->top) {
		amount = state->stack.topFrame->top;
	}

	crescentC_resizeStack(state, state->stack.topFrame->top - amount, 1);

	state->stack.topFrame->top -= amount;
}

void
crescent_remove(crescent_State* state, int index) {
	crescent_Object* object = crescent_getIndex(state, index);

	crescentO_free(object);

	for (int a = 0; a < state->stack.topFrame->top - index; a++) {
		*object = *(object + 1);
		object++;
	}

	object->type = CRESCENT_TYPE_NIL;

	crescentC_resizeStack(state, state->stack.topFrame->top - 1, 1);

	state->stack.topFrame->top -= 1;
}

int
crescent_call(crescent_State* state, int index, int argCount) {
	return crescentV_call(state, crescent_getIndex(state, index), argCount, INT_MAX);
}

int
crescent_pCall(crescent_State* state, int index, int argCount, int* status) {
	return crescentV_pCall(state, crescent_getIndex(state, index), argCount, INT_MAX, status);
}

int
crescent_callK(crescent_State* state, int index, int argCount, int maxResults) {
	return crescentV_call(state, crescent_getIndex(state, index), argCount, maxResults);
}

int
crescent_pCallK(crescent_State* state, int index, int argCount, int maxResults, int* status) {
	return crescentV_pCall(state, crescent_getIndex(state, index), argCount, maxResults, status);
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
	if (state->error == NULL) {
		crescent_pushNil(state);

		return;
	}

	unsigned int     absoluteIndex = state->stack.topFrame->base + state->stack.topFrame->top;
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
