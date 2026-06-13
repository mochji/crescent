/*
 * https://github.com/mochji/crescent
 * vm/vm.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include <stdlib.h>
#include <stddef.h>
#include <setjmp.h>
#include <limits.h>

#include "conf.h"
#include "limit.h"

#include "types/string.h"
#include "types/table.h"
#include "core/object.h"
#include "core/state.h"
#include "core/call.h"

#include "vm/vm.h"

int
crsV_equal(crs_Thread* thread, crs_Object* a, crs_Object* b) {
	(void)thread;

	int aType = a->type, bType = b->type;

	if (aType != bType) {
		if (aType == CRS_TYPE_INTEGER && bType == CRS_TYPE_FLOAT) {
			return (crs_Float)obj_geti(a) == obj_getf(b);
		}

		if (aType == CRS_TYPE_FLOAT && bType == CRS_TYPE_INTEGER) {
			return obj_getf(a) == (crs_Float)obj_geti(b);
		}

		return 0;
	}

	switch (aType) {
		case CRS_TYPE_NIL:
			return 1;
		case CRS_TYPE_BOOLEAN:
			return obj_getb(a) == obj_getb(b);
		case CRS_TYPE_INTEGER:
			return obj_geti(a) == obj_geti(b);
		case CRS_TYPE_FLOAT:
			return obj_getf(a) == obj_getf(b);
		case CRS_TYPE_CFUNCTION:
			return obj_getc(a) == obj_getc(b);
		case CRS_TYPE_STRING:
			return crsS_equal(obj_gets(a), obj_gets(b));
		case CRS_TYPE_TABLE:
			return obj_gett(a) == obj_gett(b);
		case CRS_TYPE_THREAD:
			return obj_getx(a) == obj_getx(b);
	}

	return 0;
}

crs_Integer
crsV_length(crs_Thread* thread, crs_Object* object) {
	switch (object->type) {
		case CRS_TYPE_STRING:
			return obj_gets(object)->length;
	}

	crsC_errorf(thread, "attempt to get length of a %s value", crsO_name(object));
}

crs_Object*
crsV_get(crs_Thread* thread, crs_Object* object, crs_Object* key) {
	if (object->type != CRS_TYPE_TABLE) {
		crsC_errorf(thread, "attempt to index a %s value", crsO_name(object));
	}

	return crsT_get(thread, obj_gett(object), key);
}

void
crsV_set(crs_Thread* thread, crs_Object* object, crs_Object* key, crs_Object* value) {
	if (object->type != CRS_TYPE_TABLE) {
		crsC_errorf(thread, "attempt to index a %s value", crsO_name(object));
	}

	crsT_set(thread, obj_gett(object), key, value);
}

int
crsV_call(crs_Thread* thread, crs_Object* object, int args, int maxResults) {
	if (object->type != CRS_TYPE_CFUNCTION) {
		crsC_errorf(thread, "attempt to call a %s value", crsO_name(object));
	}

	if (thread->stack.calls >= CRS_MAX_CALLS) {
		crsC_error(thread, "stack overflow");
	} else if (thread->stack.cCalls >= CRS_MAX_CCALLS) {
		crsC_error(thread, "C stack overflow");
	}

	return crsC_callC(thread, obj_getc(object), args, maxResults);
}

static void
pushError(crs_Thread* thread) {
	crs_Frame*  frame = thread->stack.frame;
	crs_Object* top   = thread->stack.top;
	crs_Object* error = &thread->error;
	int         items = top - frame->base;

	if (items >= CRS_MAX_TOP) {
		crsC_error(thread, "stack overflow");
	}

	crsC_checkFree(thread, 1, 1);

	obj_seto(top, error);
	obj_setn(error);

	thread->stack.top++;
}

int
crsV_pCall(crs_Thread* thread, crs_Object* object, int args, int maxResults, int* status) {
	crs_Handler handler;
	int         results;

	call_sethandler(thread, handler);

	if (!setjmp(handler.buffer)) {
		results = crsV_call(thread, object, args, maxResults);
	} else {
		crsC_restoreStack(thread, handler.level);
		pushError(thread);

		results = 0;
	}

	thread->handler = handler.previous;

	if (status != NULL) {
		*status = handler.status;
	}

	return results;
}
