/*
 * https://github.com/mochji/crescent
 * api/api.c
 *
 * Copyright (C) 2024 mochji
 * MIT License
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <setjmp.h>
#include <limits.h>

#include "conf.h"
#include "limit.h"

#include "types/string.h"
#include "types/array.h"
#include "core/object.h"
#include "core/state.h"
#include "core/call.h"
#include "vm/vm.h"

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

static crescent_Object*
crescent_getIndex(crescent_State* state, int index) {
	if (index == 0) {
		return &state->gState->nilValue;
	}

	crescent_Object* object;

	if (index < 0) {
		object = state->stack.top + index;

		return object >= state->stack.frame->base ?
			object :
			&state->gState->nilValue;
	}

	object = state->stack.frame->base + index - 1;

	return object < state->stack.top ?
		object :
		&state->gState->nilValue;
}

static crescent_Object*
crescent_adjustTop(crescent_State* state, int amount) {
	crescent_Frame* frame = state->stack.frame;
	int             items = state->stack.top - frame->base;

	if (-amount > items) {
		amount = -items;
	}

	if (items + amount > frame->top) {
		crescentC_setError(state, "stack overflow");
		crescentC_throw(state, CRESCENT_STATUS_ERROR);
	}

	state->stack.top += amount;

	return state->stack.top - 1;
}

int
crescent_version(void) {
	return CRESCENT_VERSION;
}

int
crescent_release(void) {
	return CRESCENT_RELEASE;
}

const char*
crescent_typeName(int type) {
	return crescentO_typeName(type);
}

crescent_State*
crescent_open(void) {
	crescent_GState* gState = crescentE_blankGState();
	crescent_State*  state  = crescentE_blankLState();

	if (gState == NULL || state == NULL) {
		crescentE_closeGState(gState);
		crescentE_closeLState(state);

		return NULL;
	}

	gState->baseThread = state;
	gState->lastThread = state;
	gState->panic      = &crescent_panic;

	state->gState = gState;

	return state;
}

void
crescent_close(crescent_State* state) {
	crescentE_closeGState(state->gState);
}

void
crescent_setPanic(crescent_State* state, crescent_CFunction* function) {
	state->gState->panic = function;
}

int
crescent_checkTop(crescent_State* state, int top) {
	return !crescentC_checkTop(
		state,
		top < CRESCENT_MIN_TOP ?
			CRESCENT_MIN_TOP :
			top
	);
}

int
crescent_getTop(crescent_State* state) {
	return state->stack.top - state->stack.frame->base;
}

void
crescent_setTop(crescent_State* state, int top) {
	if (top < 0) {
		top = 0;
	}

	crescent_Frame* frame = state->stack.frame;

	crescent_Object* object = state->stack.top;
	crescent_Object* to     = frame->base + top;

	if (to >= frame->base + frame->top) {
		crescentC_setError(state, "stack overflow");
		crescentC_throw(state, CRESCENT_STATUS_ERROR);
	}

	if (object < to) {
		while (object < to) {
			(object++)->type = CRESCENT_TYPE_NIL;
		}
	} else {
		while (object > to) {
			crescentO_free(--object);
		}
	}

	state->stack.top = to;
}

int
crescent_type(crescent_State* state, int index) {
	return crescent_getIndex(state, index)->type;
}

size_t
crescent_length(crescent_State* state, int index) {
	return crescentV_length(state, crescent_getIndex(state, index));
}

void
crescent_clone(crescent_State* state, int index) {
	crescent_Object* from = crescent_getIndex(state, index);
	crescent_Object* to   = crescent_adjustTop(state, 1);

	if (crescentO_clone(to, from)) {
		state->stack.top -= 1;

		crescentC_memoryError(state);
	}
}

void
crescent_deepClone(crescent_State* state, int index) {
	crescent_Object* from = crescent_getIndex(state, index);
	crescent_Object* to   = crescent_adjustTop(state, 1);

	if (crescentO_deepClone(to, from)) {
		state->stack.top -= 1;

		crescentC_memoryError(state);
	}
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
	return obj_isnumber(crescent_getIndex(state, index)->type);
}

int
crescent_isString(crescent_State* state, int index) {
	return crescent_getIndex(state, index)->type == CRESCENT_TYPE_STRING;
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
	return crescentO_toString(crescent_getIndex(state, index), isString);
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
	return crescentO_toString(crescent_getIndex(state, index), NULL);
}

crescent_CFunction*
crescent_toCFunction(crescent_State* state, int index) {
	return crescentO_toCFunction(crescent_getIndex(state, index), NULL);
}

void
crescent_pushNil(crescent_State* state) {
	crescent_Object* object = crescent_adjustTop(state, 1);

	object->type = CRESCENT_TYPE_NIL;
}

void
crescent_pushBoolean(crescent_State* state, int value) {
	crescent_Object* object = crescent_adjustTop(state, 1);

	object->type    = CRESCENT_TYPE_BOOLEAN;
	object->value.b = value;
}

void
crescent_pushInteger(crescent_State* state, crescent_Integer value) {
	crescent_Object* object = crescent_adjustTop(state, 1);

	object->type    = CRESCENT_TYPE_INTEGER;
	object->value.i = value;
}

void
crescent_pushFloat(crescent_State* state, crescent_Float value) {
	crescent_Object* object = crescent_adjustTop(state, 1);

	object->type    = CRESCENT_TYPE_FLOAT;
	object->value.f = value;
}

void
crescent_pushString(crescent_State* state, const char* str) {
	crescent_Object* object = crescent_adjustTop(state, 1);
	crescent_String* string = crescentS_as((char*)str);

	if (string == NULL) {
		state->stack.top -= 1;

		crescentC_memoryError(state);
	}

	object->type    = CRESCENT_TYPE_STRING;
	object->value.s = string;
}

void
crescent_pushCFunction(crescent_State* state, crescent_CFunction* function) {
	crescent_Object* object = crescent_adjustTop(state, 1);

	object->type    = CRESCENT_TYPE_CFUNCTION;
	object->value.c = function;
}

void
crescent_pop(crescent_State* state, int amount) {
	if (amount <= 0) {
		return;
	}

	crescent_Frame* frame = state->stack.frame;
	int             items = state->stack.top - frame->base;

	if (amount > items) {
		amount = items;
	}

	crescent_Object* object = state->stack.top - 1;

	for (int a = 0; a < amount; a++) {
		crescentO_free(object--);
	}

	state->stack.top -= amount;
}

void
crescent_remove(crescent_State* state, int index) {
	if (index == 0) {
		return;
	}

	crescent_Frame* frame = state->stack.frame;
	int             items = state->stack.top - frame->base;

	if (index < 0) {
		if (-index > items) {
			return;
		}

		index += items + 1;
	}

	crescent_Object* object = frame->base + index - 1;

	crescentO_free(object);

	for (int a = 0; a < items; a++) {
		*object = *(object + 1);
		object++;
	}

	object->type = CRESCENT_TYPE_NIL;

	state->stack.top -= 1;
}

int
crescent_call(crescent_State* state, int index, int args) {
	return crescentV_call(state, crescent_getIndex(state, index), args, INT_MAX);
}

int
crescent_pCall(crescent_State* state, int index, int args, int* status) {
	return crescentV_pCall(state, crescent_getIndex(state, index), args, INT_MAX, status);
}

int
crescent_callK(crescent_State* state, int index, int args, int maxResults) {
	return crescentV_call(state, crescent_getIndex(state, index), args, maxResults);
}

int
crescent_pCallK(crescent_State* state, int index, int args, int maxResults, int* status) {
	return crescentV_pCall(state, crescent_getIndex(state, index), args, maxResults, status);
}

void __attribute__((noreturn))
crescent_error(crescent_State* state, const char* error) {
	crescentC_setError(state, (char*)error);
	crescentC_throw(state, CRESCENT_STATUS_ERROR);
}

void
crescent_clearError(crescent_State* state) {
	crescentC_setError(state, NULL);
}

const char*
crescent_getError(crescent_State* state) {
	return state->error;
}
