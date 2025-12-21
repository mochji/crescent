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
crs_panic(crs_Thread* thread) {
	char* error;

	if (thread->error != NULL) {
		error = thread->error;
	} else {
		error = "no error";
	}

	fprintf(stderr, "PANIC: error within unprotected call to Crescent API (%s)\n", error);

	return 0;
}

static crs_Object*
crs_getIndex(crs_Thread* thread, int index) {
	if (index == 0) {
		return &thread->state->nilValue;
	}

	crs_Object* object;

	if (index < 0) {
		object = thread->stack.top + index;

		return object >= thread->stack.frame->base
			? object
			: &thread->state->nilValue;
	}

	object = thread->stack.frame->base + index - 1;

	return object < thread->stack.top
		? object
		: &thread->state->nilValue;
}

static crs_Object*
crs_adjustTop(crs_Thread* thread, int amount) {
	crs_Frame* frame = thread->stack.frame;
	int        items = thread->stack.top - frame->base;

	if (-amount > items) {
		amount = -items;
	}

	if (items + amount > frame->top) {
		crsC_setError(thread, "stack overflow");
		crsC_throw(thread, CRS_STATUS_ERROR);
	}

	thread->stack.top += amount;

	return thread->stack.top - 1;
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

crs_Thread*
crs_open(void) {
	crs_State*  state  = crsE_newState();
	crs_Thread* thread = crsE_newThread(state);

	if (state == NULL || thread == NULL) {
		crsE_freeState(state);
		crsE_freeThread(thread);

		return NULL;
	}

	state->panic = &crs_panic;

	return thread;
}

void
crs_close(crs_Thread* thread) {
	crsE_freeState(thread->state);
}

void
crs_setPanic(crs_Thread* thread, crs_CFunction* function) {
	thread->state->panic = function;
}

int
crs_checkTop(crs_Thread* thread, int top) {
	return !crsC_checkTop(
		thread,
		top < CRS_MIN_TOP
			? CRS_MIN_TOP
			: top,
		0
	);
}

int
crs_getTop(crs_Thread* thread) {
	return thread->stack.top - thread->stack.frame->base;
}

void
crs_setTop(crs_Thread* thread, int top) {
	if (top < 0) {
		top = 0;
	}

	crs_Frame* frame = thread->stack.frame;

	crs_Object* object = thread->stack.top;
	crs_Object* to     = frame->base + top;

	if (to >= frame->base + frame->top) {
		crsC_setError(thread, "stack overflow");
		crsC_throw(thread, CRS_STATUS_ERROR);
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

	thread->stack.top = to;
}

int
crs_type(crs_Thread* thread, int index) {
	return crs_getIndex(thread, index)->type;
}

size_t
crs_length(crs_Thread* thread, int index) {
	return crsV_length(thread, crs_getIndex(thread, index));
}

void
crs_clone(crs_Thread* thread, int index) {
	crs_Object* from = crs_getIndex(thread, index);
	crs_Object* to   = crs_adjustTop(thread, 1);

	if (crsO_clone(to, from)) {
		thread->stack.top -= 1;

		crsC_memoryError(thread);
	}
}

void
crs_deepClone(crs_Thread* thread, int index) {
	crs_Object* from = crs_getIndex(thread, index);
	crs_Object* to   = crs_adjustTop(thread, 1);

	if (crsO_deepClone(to, from)) {
		thread->stack.top -= 1;

		crsC_memoryError(thread);
	}
}

int
crs_isNil(crs_Thread* thread, int index) {
	return crs_getIndex(thread, index)->type == CRS_TYPE_NIL;
}

int
crs_isBoolean(crs_Thread* thread, int index) {
	return crs_getIndex(thread, index)->type == CRS_TYPE_BOOLEAN;
}

int
crs_isInteger(crs_Thread* thread, int index) {
	return crs_getIndex(thread, index)->type == CRS_TYPE_INTEGER;
}

int
crs_isFloat(crs_Thread* thread, int index) {
	return crs_getIndex(thread, index)->type == CRS_TYPE_FLOAT;
}

int
crs_isNumber(crs_Thread* thread, int index) {
	return obj_isnumber(crs_getIndex(thread, index)->type);
}

int
crs_isString(crs_Thread* thread, int index) {
	return crs_getIndex(thread, index)->type == CRS_TYPE_STRING;
}

int
crs_isCFunction(crs_Thread* thread, int index) {
	return crs_getIndex(thread, index)->type == CRS_TYPE_CFUNCTION;
}

int
crs_toBooleanX(crs_Thread* thread, int index, int* match) {
	return crsO_toBoolean(crs_getIndex(thread, index), match);
}

crs_Integer
crs_toIntegerX(crs_Thread* thread, int index, int* match) {
	return crsO_toInteger(crs_getIndex(thread, index), match);
}

crs_Float
crs_toFloatX(crs_Thread* thread, int index, int* match) {
	return crsO_toFloat(crs_getIndex(thread, index), match);
}

const char*
crs_toStringX(crs_Thread* thread, int index, int* match) {
	return crsO_toString(crs_getIndex(thread, index), match);
}

int
crs_toBoolean(crs_Thread* thread, int index) {
	return crsO_toBoolean(crs_getIndex(thread, index), NULL);
}

crs_Integer
crs_toInteger(crs_Thread* thread, int index) {
	return crsO_toInteger(crs_getIndex(thread, index), NULL);
}

crs_Float
crs_toFloat(crs_Thread* thread, int index) {
	return crsO_toFloat(crs_getIndex(thread, index), NULL);
}

const char*
crs_toString(crs_Thread* thread, int index) {
	return crsO_toString(crs_getIndex(thread, index), NULL);
}

crs_CFunction*
crs_toCFunction(crs_Thread* thread, int index) {
	crs_Object* object = crs_getIndex(thread, index);

	if (object->type == CRS_TYPE_CFUNCTION) {
		return object->value.c;
	}

	return NULL;
}

void
crs_pushNil(crs_Thread* thread) {
	crs_Object* object = crs_adjustTop(thread, 1);

	object->type = CRS_TYPE_NIL;
}

void
crs_pushBoolean(crs_Thread* thread, int value) {
	crs_Object* object = crs_adjustTop(thread, 1);

	object->type    = CRS_TYPE_BOOLEAN;
	object->value.b = value;
}

void
crs_pushInteger(crs_Thread* thread, crs_Integer value) {
	crs_Object* object = crs_adjustTop(thread, 1);

	object->type    = CRS_TYPE_INTEGER;
	object->value.i = value;
}

void
crs_pushFloat(crs_Thread* thread, crs_Float value) {
	crs_Object* object = crs_adjustTop(thread, 1);

	object->type    = CRS_TYPE_FLOAT;
	object->value.f = value;
}

void
crs_pushString(crs_Thread* thread, const char* str) {
	crs_Object* object = crs_adjustTop(thread, 1);
	crs_String* string = crsS_as((char*)str);

	if (string == NULL) {
		thread->stack.top -= 1;

		crsC_memoryError(thread);
	}

	object->type    = CRS_TYPE_STRING;
	object->value.s = string;
}

void
crs_pushCFunction(crs_Thread* thread, crs_CFunction* function) {
	crs_Object* object = crs_adjustTop(thread, 1);

	object->type    = CRS_TYPE_CFUNCTION;
	object->value.c = function;
}

void
crs_pop(crs_Thread* thread, int amount) {
	if (amount <= 0) {
		return;
	}

	crs_Frame* frame = thread->stack.frame;
	int        items = thread->stack.top - frame->base;

	if (amount > items) {
		amount = items;
	}

	crs_Object* object = thread->stack.top - 1;

	for (int a = 0; a < amount; a++) {
		crsO_free(object--);
	}

	thread->stack.top -= amount;
}

void
crs_remove(crs_Thread* thread, int index) {
	if (index == 0) {
		return;
	}

	crs_Frame* frame = thread->stack.frame;
	int        items = thread->stack.top - frame->base;

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

	thread->stack.top -= 1;
}

int
crs_call(crs_Thread* thread, int index, int args) {
	return crsV_call(thread, crs_getIndex(thread, index), args, INT_MAX);
}

int
crs_pCall(crs_Thread* thread, int index, int args, int* status) {
	return crsV_pCall(thread, crs_getIndex(thread, index), args, INT_MAX, status);
}

int
crs_callK(crs_Thread* thread, int index, int args, int maxResults) {
	return crsV_call(thread, crs_getIndex(thread, index), args, maxResults);
}

int
crs_pCallK(crs_Thread* thread, int index, int args, int maxResults, int* status) {
	return crsV_pCall(thread, crs_getIndex(thread, index), args, maxResults, status);
}

void __attribute__((noreturn))
crs_error(crs_Thread* thread, const char* error) {
	crsC_setError(thread, (char*)error);
	crsC_throw(thread, CRS_STATUS_ERROR);
}

void
crs_clearError(crs_Thread* thread) {
	crsC_setError(thread, NULL);
}

const char*
crs_getError(crs_Thread* thread) {
	return thread->error;
}
