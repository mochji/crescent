/*
 * https://github.com/mochji/crescent
 * vm/vm.c
 *
 * Copyright (C) 2025 mochji
 * MIT License
 */

#include <stdlib.h>
#include <stddef.h>
#include <setjmp.h>
#include <limits.h>

#include "conf.h"
#include "limit.h"

#include "core/object.h"
#include "core/state.h"
#include "core/call.h"

#include "vm/vm.h"

/*
 * This function is a more loose comparison compared to crsO_compare.
 * crsO_compare returns false if the 2 objects are of differing types,
 * but this allows comparisons between floats and integers.
 */

int
crsV_compare(crs_Object* a, crs_Object* b) {
	int aType = a->type;
	int bType = b->type;

	if (aType != bType) {
		if (aType == CRS_TYPE_INTEGER && bType == CRS_TYPE_FLOAT) {
			return (crs_Float)a->value.i == b->value.f;
		}

		if (aType == CRS_TYPE_FLOAT && bType == CRS_TYPE_INTEGER) {
			return a->value.f == (crs_Float)b->value.i;
		}

		return 0;
	}

	return crsO_compare(a, b);
}

/* TODO: these functions are temporary until string format */

static void
crsV_lengthError(crs_State* state, int type) {
	switch (type) {
		case CRS_TYPE_NIL:
			crsC_setError(state, "attempt to get length of a nil value");

			break;
		case CRS_TYPE_BOOLEAN:
			crsC_setError(state, "attempt to get length of a boolean value");

			break;
		case CRS_TYPE_INTEGER:
			crsC_setError(state, "attempt to get length of a number value");

			break;
		case CRS_TYPE_FLOAT:
			crsC_setError(state, "attempt to get length of a number value");

			break;
		case CRS_TYPE_CFUNCTION:
			crsC_setError(state, "attempt to get length of a string value");

			break;
	}
}

static void
crsV_callError(crs_State* state, int type) {
	switch (type) {
		case CRS_TYPE_NIL:
			crsC_setError(state, "attempt to call a nil value");

			break;
		case CRS_TYPE_BOOLEAN:
			crsC_setError(state, "attempt to call a boolean value");

			break;
		case CRS_TYPE_INTEGER:
			crsC_setError(state, "attempt to call a number value");

			break;
		case CRS_TYPE_FLOAT:
			crsC_setError(state, "attempt to call a number value");

			break;
		case CRS_TYPE_STRING:
			crsC_setError(state, "attempt to call a string value");

			break;
		case CRS_TYPE_ARRAY:
			crsC_setError(state, "attempt to call a array value");

			break;
	}
}

size_t
crsV_length(crs_State* state, crs_Object* object) {
	if (!obj_haslength(object->type)) {
		crsV_lengthError(state, object->type);
		crsC_throw(state, CRS_STATUS_ERROR);
	}

	if (object->type == CRS_TYPE_STRING) {
		return object->value.s->length;
	}

	return object->value.a->length;
}

int
crsV_call(crs_State* state, crs_Object* object, int args, int maxResults) {
	if (!obj_cancall(object->type)) {
		crsV_callError(state, object->type);
		crsC_throw(state, CRS_STATUS_ERROR);
	}

	if (state->stack.calls >= CRS_MAX_CALLS) {
		crsC_setError(state, "stack overflow");
		crsC_throw(state, CRS_STATUS_ERROR);
	} else if (state->stack.cCalls >= CRS_MAX_CCALLS) {
		crsC_setError(state, "C stack overflow");
		crsC_throw(state, CRS_STATUS_ERROR);
	}

	return crsC_callC(state, object->value.c, args, maxResults);
}

int
crsV_pCall(crs_State* state, crs_Object* object, int args, int maxResults, int* status) {
	jmp_buf* handler  = malloc(sizeof(jmp_buf));
	jmp_buf* previous = state->handler;
	short    calls    = state->stack.calls;

	state->handler = handler;

	int results;

	if (setjmp(*handler) == 0) {
		results = crsV_call(state, object, args, maxResults);
	} else {
		crsC_restoreStack(state, calls);
		results = 0;
	}

	if (status != NULL) {
		*status = state->status;
	}

	state->status  = CRS_STATUS_OK;
	state->handler = previous;
	free(handler);

	return results;
}
