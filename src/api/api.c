/*
 * https://github.com/mochji/crescent
 * api/api.c
 *
 * Copyright (C) 2025 mochji
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
crs_panic(crs_State* state) {
	char* error;

	if (state->error != NULL) {
		error = state->error;
	} else {
		error = "no error";
	}

	fprintf(stderr, "PANIC: error within unprotected call to Crescent API (%s)\n", error);

	return 0;
}

static crs_Object*
crs_getIndex(crs_State* state, int index) {
	if (index == 0) {
		return &state->gState->nilValue;
	}

	crs_Object* object;

	if (index < 0) {
		object = state->stack.top + index;

		return object >= state->stack.frame->base
			? object
			: &state->gState->nilValue;
	}

	object = state->stack.frame->base + index - 1;

	return object < state->stack.top
		? object
		: &state->gState->nilValue;
}

static crs_Object*
crs_adjustTop(crs_State* state, int amount) {
	crs_Frame* frame = state->stack.frame;
	int        items = state->stack.top - frame->base;

	if (-amount > items) {
		amount = -items;
	}

	if (items + amount > frame->top) {
		crsC_setError(state, "stack overflow");
		crsC_throw(state, CRS_STATUS_ERROR);
	}

	state->stack.top += amount;

	return state->stack.top - 1;
}

int
crs_version(void) {
	return CRS_VERSION;
}

int
crs_release(void) {
	return CRS_RELEASE;
}

const char*
crs_typeName(int type) {
	return crsO_typeName(type);
}

crs_State*
crs_open(void) {
	crs_GState* gState = crsE_blankGState();
	crs_State*  state  = crsE_blankLState();

	if (gState == NULL || state == NULL) {
		crsE_closeGState(gState);
		crsE_closeLState(state);

		return NULL;
	}

	gState->mainThread = state;
	gState->lastThread = state;
	gState->panic      = &crs_panic;

	state->gState = gState;

	return state;
}

void
crs_close(crs_State* state) {
	crsE_closeGState(state->gState);
}

void
crs_setPanic(crs_State* state, crs_CFunction* function) {
	state->gState->panic = function;
}

int
crs_checkTop(crs_State* state, int top) {
	return !crsC_checkTop(
		state,
		top < CRS_MIN_TOP
			? CRS_MIN_TOP
			: top,
		0
	);
}

int
crs_getTop(crs_State* state) {
	return state->stack.top - state->stack.frame->base;
}

void
crs_setTop(crs_State* state, int top) {
	if (top < 0) {
		top = 0;
	}

	crs_Frame* frame = state->stack.frame;

	crs_Object* object = state->stack.top;
	crs_Object* to     = frame->base + top;

	if (to >= frame->base + frame->top) {
		crsC_setError(state, "stack overflow");
		crsC_throw(state, CRS_STATUS_ERROR);
	}

	if (object < to) {
		while (object < to) {
			(object++)->type = CRS_TYPE_NIL;
		}
	} else {
		while (object > to) {
			crsO_free(--object);
		}
	}

	state->stack.top = to;
}

int
crs_type(crs_State* state, int index) {
	return crs_getIndex(state, index)->type;
}

size_t
crs_length(crs_State* state, int index) {
	return crsV_length(state, crs_getIndex(state, index));
}

void
crs_clone(crs_State* state, int index) {
	crs_Object* from = crs_getIndex(state, index);
	crs_Object* to   = crs_adjustTop(state, 1);

	if (crsO_clone(to, from)) {
		state->stack.top -= 1;

		crsC_memoryError(state);
	}
}

void
crs_deepClone(crs_State* state, int index) {
	crs_Object* from = crs_getIndex(state, index);
	crs_Object* to   = crs_adjustTop(state, 1);

	if (crsO_deepClone(to, from)) {
		state->stack.top -= 1;

		crsC_memoryError(state);
	}
}

int
crs_isNil(crs_State* state, int index) {
	return crs_getIndex(state, index)->type == CRS_TYPE_NIL;
}

int
crs_isBoolean(crs_State* state, int index) {
	return crs_getIndex(state, index)->type == CRS_TYPE_BOOLEAN;
}

int
crs_isInteger(crs_State* state, int index) {
	return crs_getIndex(state, index)->type == CRS_TYPE_INTEGER;
}

int
crs_isFloat(crs_State* state, int index) {
	return crs_getIndex(state, index)->type == CRS_TYPE_FLOAT;
}

int
crs_isNumber(crs_State* state, int index) {
	return obj_isnumber(crs_getIndex(state, index)->type);
}

int
crs_isString(crs_State* state, int index) {
	return crs_getIndex(state, index)->type == CRS_TYPE_STRING;
}

int
crs_isCFunction(crs_State* state, int index) {
	return crs_getIndex(state, index)->type == CRS_TYPE_CFUNCTION;
}

int
crs_toBooleanX(crs_State* state, int index, int* match) {
	return crsO_toBoolean(crs_getIndex(state, index), match);
}

crs_Integer
crs_toIntegerX(crs_State* state, int index, int* match) {
	return crsO_toInteger(crs_getIndex(state, index), match);
}

crs_Float
crs_toFloatX(crs_State* state, int index, int* match) {
	return crsO_toFloat(crs_getIndex(state, index), match);
}

const char*
crs_toStringX(crs_State* state, int index, int* match) {
	return crsO_toString(crs_getIndex(state, index), match);
}

int
crs_toBoolean(crs_State* state, int index) {
	return crsO_toBoolean(crs_getIndex(state, index), NULL);
}

crs_Integer
crs_toInteger(crs_State* state, int index) {
	return crsO_toInteger(crs_getIndex(state, index), NULL);
}

crs_Float
crs_toFloat(crs_State* state, int index) {
	return crsO_toFloat(crs_getIndex(state, index), NULL);
}

const char*
crs_toString(crs_State* state, int index) {
	return crsO_toString(crs_getIndex(state, index), NULL);
}

crs_CFunction*
crs_toCFunction(crs_State* state, int index) {
	crs_Object* object = crs_getIndex(state, index);

	if (object->type == CRS_TYPE_CFUNCTION) {
		return object->value.c;
	}

	return NULL;
}

void
crs_pushNil(crs_State* state) {
	crs_Object* object = crs_adjustTop(state, 1);

	object->type = CRS_TYPE_NIL;
}

void
crs_pushBoolean(crs_State* state, int value) {
	crs_Object* object = crs_adjustTop(state, 1);

	object->type    = CRS_TYPE_BOOLEAN;
	object->value.b = value;
}

void
crs_pushInteger(crs_State* state, crs_Integer value) {
	crs_Object* object = crs_adjustTop(state, 1);

	object->type    = CRS_TYPE_INTEGER;
	object->value.i = value;
}

void
crs_pushFloat(crs_State* state, crs_Float value) {
	crs_Object* object = crs_adjustTop(state, 1);

	object->type    = CRS_TYPE_FLOAT;
	object->value.f = value;
}

void
crs_pushString(crs_State* state, const char* str) {
	crs_Object* object = crs_adjustTop(state, 1);
	crs_String* string = crsS_as((char*)str);

	if (string == NULL) {
		state->stack.top -= 1;

		crsC_memoryError(state);
	}

	object->type    = CRS_TYPE_STRING;
	object->value.s = string;
}

void
crs_pushCFunction(crs_State* state, crs_CFunction* function) {
	crs_Object* object = crs_adjustTop(state, 1);

	object->type    = CRS_TYPE_CFUNCTION;
	object->value.c = function;
}

void
crs_pop(crs_State* state, int amount) {
	if (amount <= 0) {
		return;
	}

	crs_Frame* frame = state->stack.frame;
	int        items = state->stack.top - frame->base;

	if (amount > items) {
		amount = items;
	}

	crs_Object* object = state->stack.top - 1;

	for (int a = 0; a < amount; a++) {
		crsO_free(object--);
	}

	state->stack.top -= amount;
}

void
crs_remove(crs_State* state, int index) {
	if (index == 0) {
		return;
	}

	crs_Frame* frame = state->stack.frame;
	int        items = state->stack.top - frame->base;

	if (index < 0) {
		if (-index > items) {
			return;
		}

		index += items + 1;
	}

	crs_Object* object = frame->base + index - 1;

	crsO_free(object);

	for (int a = 0; a < items; a++) {
		*object = *(object + 1);
		object++;
	}

	object->type = CRS_TYPE_NIL;

	state->stack.top -= 1;
}

int
crs_call(crs_State* state, int index, int args) {
	return crsV_call(state, crs_getIndex(state, index), args, INT_MAX);
}

int
crs_pCall(crs_State* state, int index, int args, int* status) {
	return crsV_pCall(state, crs_getIndex(state, index), args, INT_MAX, status);
}

int
crs_callK(crs_State* state, int index, int args, int maxResults) {
	return crsV_call(state, crs_getIndex(state, index), args, maxResults);
}

int
crs_pCallK(crs_State* state, int index, int args, int maxResults, int* status) {
	return crsV_pCall(state, crs_getIndex(state, index), args, maxResults, status);
}

void __attribute__((noreturn))
crs_error(crs_State* state, const char* error) {
	crsC_setError(state, (char*)error);
	crsC_throw(state, CRS_STATUS_ERROR);
}

void
crs_clearError(crs_State* state) {
	crsC_setError(state, NULL);
}

const char*
crs_getError(crs_State* state) {
	return state->error;
}
