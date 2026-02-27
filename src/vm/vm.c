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

#include "core/object.h"
#include "core/state.h"
#include "core/call.h"

#include "vm/vm.h"

size_t
crsV_length(crs_Thread* thread, crs_Object* object) {
	switch (object->type) {
		case CRS_TYPE_STRING:
			return obj_gets(object)->length;
		case CRS_TYPE_ARRAY:
			return obj_geta(object)->length;
	}

	/* TODO: proper error message */
	crsC_error(thread, "cannot get length");
}

int
crsV_call(crs_Thread* thread, crs_Object* object, int args, int maxResults) {
	if (object->type != CRS_TYPE_CFUNCTION) {
		/* TODO: proper error message */
		crsC_error(thread, "cannot call");
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
