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
			return (crs_Float)obj_geti(a) == obj_getf(b);
		}

		if (aType == CRS_TYPE_FLOAT && bType == CRS_TYPE_INTEGER) {
			return obj_getf(a) == (crs_Float)obj_geti(b);
		}

		return 0;
	}

	return crsO_compare(a, b);
}

/* TODO: these functions are temporary until string format */

static void
crsV_lengthError(crs_Thread* thread, int type) {
	switch (type) {
		case CRS_TYPE_NIL:
			crsC_setError(thread, "attempt to get length of a nil value");

			break;
		case CRS_TYPE_BOOLEAN:
			crsC_setError(thread, "attempt to get length of a boolean value");

			break;
		case CRS_TYPE_INTEGER:
			crsC_setError(thread, "attempt to get length of a number value");

			break;
		case CRS_TYPE_FLOAT:
			crsC_setError(thread, "attempt to get length of a number value");

			break;
		case CRS_TYPE_CFUNCTION:
			crsC_setError(thread, "attempt to get length of a string value");

			break;
	}
}

static void
crsV_callError(crs_Thread* thread, int type) {
	switch (type) {
		case CRS_TYPE_NIL:
			crsC_setError(thread, "attempt to call a nil value");

			break;
		case CRS_TYPE_BOOLEAN:
			crsC_setError(thread, "attempt to call a boolean value");

			break;
		case CRS_TYPE_INTEGER:
			crsC_setError(thread, "attempt to call a number value");

			break;
		case CRS_TYPE_FLOAT:
			crsC_setError(thread, "attempt to call a number value");

			break;
		case CRS_TYPE_STRING:
			crsC_setError(thread, "attempt to call a string value");

			break;
		case CRS_TYPE_ARRAY:
			crsC_setError(thread, "attempt to call a array value");

			break;
	}
}

size_t
crsV_length(crs_Thread* thread, crs_Object* object) {
	if (object->type == CRS_TYPE_STRING) {
		return obj_gets(object)->length;
	} else if (object->type == CRS_TYPE_ARRAY) {
		return obj_geta(object)->length;
	} else {
		crsV_lengthError(thread, object->type);
		crsC_throw(thread, CRS_STATUS_ERROR);
	}
}

int
crsV_call(crs_Thread* thread, crs_Object* object, int args, int maxResults) {
	if (object->type != CRS_TYPE_CFUNCTION) {
		crsV_callError(thread, object->type);
		crsC_throw(thread, CRS_STATUS_ERROR);
	}

	if (thread->stack.calls >= CRS_MAX_CALLS) {
		crsC_setError(thread, "stack overflow");
		crsC_throw(thread, CRS_STATUS_ERROR);
	} else if (thread->stack.cCalls >= CRS_MAX_CCALLS) {
		crsC_setError(thread, "C stack overflow");
		crsC_throw(thread, CRS_STATUS_ERROR);
	}

	return crsC_callC(thread, obj_getc(object), args, maxResults);
}

int
crsV_pCall(crs_Thread* thread, crs_Object* object, int args, int maxResults, int* status) {
	jmp_buf* handler  = malloc(sizeof(jmp_buf));
	jmp_buf* previous = thread->handler;
	short    calls    = thread->stack.calls;

	thread->handler = handler;

	int results;

	if (setjmp(*handler) == 0) {
		results = crsV_call(thread, object, args, maxResults);
	} else {
		crsC_restoreStack(thread, calls);
		results = 0;
	}

	if (status != NULL) {
		*status = thread->status;
	}

	thread->status  = CRS_STATUS_OK;
	thread->handler = previous;
	free(handler);

	return results;
}
