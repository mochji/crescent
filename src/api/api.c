/*
 * https://github.com/mochji/crescent
 * api/api.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include <stdio.h>
#include <limits.h>

#include "conf.h"
#include "limit.h"

#include "types/string.h"
#include "types/array.h"
#include "core/object.h"
#include "core/state.h"
#include "core/memory.h"
#include "core/gc.h"
#include "core/call.h"
#include "vm/vm.h"

#include "api/api.h"

static int
panic(crs_Thread* thread) {
	crs_Object* error = &thread->error;
	char*       message;

	if (error->type == CRS_TYPE_STRING) {
		message = obj_gets(error)->contents;
	} else {
		message = "error is not a string";
	}

	fprintf(stderr, "PANIC: error within unprotected call to Crescent API (%s)\n", message);

	return 0;
}

static crs_Object*
getIndex(crs_Thread* thread, int index) {
	crs_Object* nil = &thread->state->nilValue;
	crs_Object* object;

	if (index == 0) {
		return nil;
	}

	if (index < 0) {
		object = thread->stack.top + index;

		return object >= thread->stack.frame->base
			? object
			: nil;
	}

	object = thread->stack.frame->base + index - 1;

	return object < thread->stack.top
		? object
		: nil;
}

static crs_Object*
adjustTop(crs_Thread* thread, int amount) {
	crs_Frame* frame = thread->stack.frame;
	int        items = thread->stack.top - frame->base;

	if (-amount > items) {
		amount = -items;
	} else if (items + amount > frame->top) {
		crsC_error(thread, "stack overflow");
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

/*
 * ===========================
 *  state manipulation
 * ===========================
 */

crs_Thread*
crs_open(void) {
	crs_Thread* thread = crsE_open();
	thread->state->panic = &panic;

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

void
crs_error(crs_Thread* thread, int index) {
	crs_Object* object = getIndex(thread, index);

	obj_seto(&thread->error, object);
	crsC_throw(thread);
}

/*
 * ===========================
 *  gc management
 * ===========================
 */

int
crs_gc(crs_Thread* thread, int option) {
	switch (option) {
		case CRS_GC_STEP:
			return crsG_step(thread);
		case CRS_GC_FULL:
			crsG_full(thread, 0); return 1;
		case CRS_GC_USAGE:
			return (int)(thread->state->gc.usage / 1024);
	}

	return -1;
}

int
crs_getGC(crs_Thread* thread, int option) {
	crs_State* state = thread->state;

	switch (option) {
		case CRS_GC_STOP:
			return gc_getstatus(state, STOP);
		case CRS_GC_STOPEM:
			return gc_getstatus(state, STOPEM);
		case CRS_GC_PAUSE:
			return gc_getparam(state, PAUSE);
		case CRS_GC_STEP:
			return gc_getparam(state, STEP);
		case CRS_GC_MULTIPLIER:
			return gc_getparam(state, MULTIPLIER);
	}

	return -1;
}

void
crs_setGC(crs_Thread* thread, int option, unsigned short value) {
	crs_State* state = thread->state;

	switch (option) {
		case CRS_GC_STOP:
			value = value != 0;
			gc_setstatus(state, STOP, value);

			break;
		case CRS_GC_STOPEM:
			value = value != 0;
			gc_setstatus(state, STOPEM, value);

			break;
		case CRS_GC_PAUSE:
			gc_setparam(state, PAUSE, value);

			break;
		case CRS_GC_STEP:
			gc_setparam(state, STEP, value);

			break;
		case CRS_GC_MULTIPLIER:
			gc_setparam(state, MULTIPLIER, value);

			break;
	}
}

/*
 * ===========================
 *  stack manipulation
 * ===========================
 */

int
crs_getTop(crs_Thread* thread) {
	return thread->stack.top - thread->stack.frame->base;
}

int
crs_checkTop(crs_Thread* thread, int top) {
	int result = !crsC_checkTop(thread, top, 0);
	crsG_check(thread);

	return result;
}

void
crs_setTop(crs_Thread* thread, int top) {
	if (top < 0) {
		top = 0;
	} else if (top > CRS_MAX_TOP) {
		top = CRS_MAX_TOP;
	}

	crs_Frame* frame = thread->stack.frame;

	if (top > frame->top) {
		crsC_error(thread, "stack overflow");
	}

	crs_Object* object = thread->stack.top;
	crs_Object* to     = frame->base + top;

	if (object < to) {
		while (object < to) {
			obj_setn(object);
			object++;
		}
	}

	thread->stack.top = to;
}

/* pop a max of 'amount' objects off of the stack */
void
crs_pop(crs_Thread* thread, int amount) {
	if (amount <= 0) {
		return;
	}

	int items = thread->stack.top - thread->stack.frame->base;

	if (amount > items) {
		amount = items;
	}

	thread->stack.top -= amount;
}

/* remove an element from the stack, shifting above elements down */
void
crs_remove(crs_Thread* thread, int index) {
	if (index == 0) {
		return;
	}

	crs_Frame* frame = thread->stack.frame;
	int        items = thread->stack.top - frame->base;

	if (index > items) {
		return;
	} else if (index < 0) {
		if (-index > items) {
			return;
		}

		index += items + 1;
	}

	crs_Object* object = frame->base + index - 1;

	while (object < thread->stack.top) {
		obj_seto(object, object + 1);
		object++;
	}

	thread->stack.top -= 1;
}

/*
 * ===========================
 *  basic object functions
 * ===========================
 */

int
crs_type(crs_Thread* thread, int index) {
	return getIndex(thread, index)->type;
}

size_t
crs_length(crs_Thread* thread, int index) {
	return crsV_length(thread, getIndex(thread, index));
}

void
crs_clone(crs_Thread* thread, int index) {
	crs_Object* from = getIndex(thread, index);
	crs_Object* to   = adjustTop(thread, 1);

	obj_seto(to, from);
}

/*
 * ===========================
 *  is* functions
 * ===========================
 */

int
crs_isNil(crs_Thread* thread, int index) {
	return getIndex(thread, index)->type == CRS_TYPE_NIL;
}

int
crs_isBoolean(crs_Thread* thread, int index) {
	return getIndex(thread, index)->type == CRS_TYPE_BOOLEAN;
}

int
crs_isInteger(crs_Thread* thread, int index) {
	return getIndex(thread, index)->type == CRS_TYPE_INTEGER;
}

int
crs_isFloat(crs_Thread* thread, int index) {
	return getIndex(thread, index)->type == CRS_TYPE_FLOAT;
}

int
crs_isNumber(crs_Thread* thread, int index) {
	return obj_isnumber(getIndex(thread, index));
}

int
crs_isCFunction(crs_Thread* thread, int index) {
	return getIndex(thread, index)->type == CRS_TYPE_CFUNCTION;
}

int
crs_isString(crs_Thread* thread, int index) {
	return getIndex(thread, index)->type == CRS_TYPE_STRING;
}

/*
 * ===========================
 *  to* functions
 * ===========================
 */

int
crs_toBooleanX(crs_Thread* thread, int index, int* match) {
	return crsO_toBoolean(getIndex(thread, index), match);
}

crs_Integer
crs_toIntegerX(crs_Thread* thread, int index, int* match) {
	return crsO_toInteger(getIndex(thread, index), match);
}

crs_Float
crs_toFloatX(crs_Thread* thread, int index, int* match) {
	return crsO_toFloat(getIndex(thread, index), match);
}

const char*
crs_toStringX(crs_Thread* thread, int index, int* match) {
	return crsO_toString(getIndex(thread, index), match);
}

int
crs_toBoolean(crs_Thread* thread, int index) {
	return crsO_toBoolean(getIndex(thread, index), NULL);
}

crs_Integer
crs_toInteger(crs_Thread* thread, int index) {
	return crsO_toInteger(getIndex(thread, index), NULL);
}

crs_Float
crs_toFloat(crs_Thread* thread, int index) {
	return crsO_toFloat(getIndex(thread, index), NULL);
}

crs_CFunction*
crs_toCFunction(crs_Thread* thread, int index) {
	crs_Object* object = getIndex(thread, index);

	if (object->type == CRS_TYPE_CFUNCTION) {
		return obj_getc(object);
	}

	return NULL;
}

const char*
crs_toString(crs_Thread* thread, int index) {
	return crsO_toString(getIndex(thread, index), NULL);
}

/*
 * ===========================
 *  push* functions
 * ===========================
 */

void
crs_pushNil(crs_Thread* thread) {
	crs_Object* object = adjustTop(thread, 1);

	obj_setn(object);
}

void
crs_pushBoolean(crs_Thread* thread, int value) {
	crs_Object* object = adjustTop(thread, 1);

	obj_setb(object, value);
}

void
crs_pushInteger(crs_Thread* thread, crs_Integer value) {
	crs_Object* object = adjustTop(thread, 1);

	obj_seti(object, value);
}

void
crs_pushFloat(crs_Thread* thread, crs_Float value) {
	crs_Object* object = adjustTop(thread, 1);

	obj_setf(object, value);
}

void
crs_pushCFunction(crs_Thread* thread, crs_CFunction* function) {
	crs_Object* object = adjustTop(thread, 1);

	obj_setc(object, function);
}

void
crs_pushString(crs_Thread* thread, const char* str) {
	crs_Object* object = adjustTop(thread, 1);
	crs_String* string = crsS_new(thread, (char*)str);

	if (string == NULL) {
		thread->stack.top -= 1;

		crsM_error(thread);
	}

	obj_setgc(object, string);

	crsG_check(thread);
}

/*
 * ===========================
 *  calling
 * ===========================
 */

int
crs_call(crs_Thread* thread, int index, int args) {
	int results = crsV_call(thread, getIndex(thread, index), args, INT_MAX);
	crsG_check(thread);

	return results;
}

int
crs_pCall(crs_Thread* thread, int index, int args, int* status) {
	int results = crsV_pCall(thread, getIndex(thread, index), args, INT_MAX, status);
	crsG_check(thread);

	return results;
}

int
crs_callX(crs_Thread* thread, int index, int args, int maxResults) {
	int results = crsV_call(thread, getIndex(thread, index), args, maxResults);
	crsG_check(thread);

	return results;
}

int
crs_pCallX(crs_Thread* thread, int index, int args, int maxResults, int* status) {
	int results = crsV_pCall(thread, getIndex(thread, index), args, maxResults, status);
	crsG_check(thread);

	return results;
}
