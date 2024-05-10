/*
 * https://github.com/mochji/crescent
 * vm/vm.c
 *
 * idrk what this is
 * Copyright (C) 2024 mochji
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "conf.h"

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
	}

	return crescentO_compare(a, b);
}

/* TODO: this is temporary until string format */

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

int
crescentV_call(crescent_State* state, crescent_Object* object, int argCount, int maxResults) {
	if (object->type != CRESCENT_TYPE_CFUNCTION) {
		crescentV_callError(state, object->type);
		crescentC_throw(state, CRESCENT_STATUS_ERROR);
	}

	if (maxResults < 0) {
		maxResults = 0;
	}

	crescentC_startCall(state, argCount);

	int results = object->value.c(state);

	if (results < 0) {
		results = 0;
	} else if (results > maxResults) {
		results = maxResults;
	}

	crescentC_endCall(state, results);

	return results;
}

int
crescentV_pCall(crescent_State* state, crescent_Object* object, int argCount, int maxResults, int* status) {
	crescent_ErrorJump* oldErrorJump  = state->errorJump;
	crescent_ErrorJump  newErrorJump  = {.status = CRESCENT_STATUS_OK};
	size_t              oldFrameIndex = state->stack.frameCount - 1;
	int                 results;

	state->errorJump = &newErrorJump;

	if (setjmp(newErrorJump.buffer) == 0) {
		results = crescentV_call(state, object, argCount, maxResults);
	} else {
		for (size_t a = oldFrameIndex; a < state->stack.frameCount; a++) {
			crescentC_endCall(state, 0);
		}

		results = 0;
	}

	state->errorJump = oldErrorJump;

	if (status != NULL) {
		*status = newErrorJump.status;
	}

	return results;
}
