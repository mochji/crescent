/*
 * https://github.com/mochji/crescent
 * vm/vm.c
 *
 * Copyright (C) 2024 mochji
 * MIT License
 */

#include <stdlib.h>
#include <stddef.h>
#include <setjmp.h>
#include <limits.h>

#include "conf.h"
#include "limit.h"

#include "types/string.h"
#include "types/array.h"
#include "core/object.h"
#include "core/state.h"
#include "core/call.h"

/*
 * This function is a more loose comparison compared to crescentO_compare, as
 * crescentO_compare returns false if the 2 objects are of differing types
 * while this allows comparisons between floats and integers.
 */

int
crescentV_compare(crescent_Object* a, crescent_Object* b) {
	int aType = a->type;
	int bType = b->type;

	if (aType != bType) {
		if (aType == CRESCENT_TYPE_INTEGER && bType == CRESCENT_TYPE_FLOAT) {
			return (crescent_Float)a->value.i == b->value.f;
		}

		if (aType == CRESCENT_TYPE_FLOAT && bType == CRESCENT_TYPE_INTEGER) {
			return a->value.f == (crescent_Float)b->value.i;
		}

		return 0;
	}

	return crescentO_compare(a, b);
}

/* TODO: these functions are temporary until string format */

static void
crescentV_lengthError(crescent_State* state, int type) {
	switch (type) {
		case CRESCENT_TYPE_NIL:
			crescentC_setError(state, "attempt to get length of a nil value");

			break;
		case CRESCENT_TYPE_BOOLEAN:
			crescentC_setError(state, "attempt to get length of a boolean value");

			break;
		case CRESCENT_TYPE_INTEGER:
			crescentC_setError(state, "attempt to get length of a number value");

			break;
		case CRESCENT_TYPE_FLOAT:
			crescentC_setError(state, "attempt to get length of a number value");

			break;
		case CRESCENT_TYPE_CFUNCTION:
			crescentC_setError(state, "attempt to get length of a string value");

			break;
	}
}

static void
crescentV_callError(crescent_State* state, int type) {
	switch (type) {
		case CRESCENT_TYPE_NIL:
			crescentC_setError(state, "attempt to call a nil value");

			break;
		case CRESCENT_TYPE_BOOLEAN:
			crescentC_setError(state, "attempt to call a boolean value");

			break;
		case CRESCENT_TYPE_INTEGER:
			crescentC_setError(state, "attempt to call a number value");

			break;
		case CRESCENT_TYPE_FLOAT:
			crescentC_setError(state, "attempt to call a number value");

			break;
		case CRESCENT_TYPE_STRING:
			crescentC_setError(state, "attempt to call a string value");

			break;
		case CRESCENT_TYPE_ARRAY:
			crescentC_setError(state, "attempt to call a array value");

			break;
	}
}

size_t
crescentV_length(crescent_State* state, crescent_Object* object) {
	if (!obj_haslength(object->type)) {
		crescentV_lengthError(state, object->type);
		crescentC_throw(state, CRESCENT_STATUS_ERROR);
	}

	if (object->type == CRESCENT_TYPE_STRING) {
		return object->value.s->length;
	}

	return object->value.a->length;
}

int
crescentV_call(crescent_State* state, crescent_Object* object, int args, int maxResults) {
	if (!obj_cancall(object->type)) {
		crescentV_callError(state, object->type);
		crescentC_throw(state, CRESCENT_STATUS_ERROR);
	}

	if (state->stack.calls >= CRESCENT_MAX_CALLS) {
		crescentC_setError(state, "stack overflow");
		crescentC_throw(state, CRESCENT_STATUS_ERROR);
	} else if (state->stack.cCalls >= CRESCENT_MAX_CCALLS) {
		crescentC_setError(state, "C stack overflow");
		crescentC_throw(state, CRESCENT_STATUS_ERROR);
	}

	return crescentC_callC(state, object->value.c, args, maxResults);
}

int
crescentV_pCall(crescent_State* state, crescent_Object* object, int args, int maxResults, int* status) {
	crescent_ErrorJump* errorJump    = malloc(sizeof(crescent_ErrorJump));
	int                 oldCallCount = state->stack.calls;
	int                 results;

	errorJump->previous = state->errorJump;
	state->errorJump    = errorJump;

	if (setjmp(errorJump->buffer) == 0) {
		results = crescentV_call(state, object, args, maxResults);
	} else {
		for (; state->stack.calls > oldCallCount;) {
			crescentC_endCall(state, 0);
		}

		crescentC_resizeStack(state, state->stack.frame->top, 0);

		results = 0;
	}

	state->errorJump = errorJump->previous;

	if (status != NULL) {
		*status = errorJump->status;
	}

	free(errorJump);

	return results;
}
