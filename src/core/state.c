#include <stdlib.h>
#include <stddef.h>

#include "object.h"

struct crescent_State {
	struct crescent_Stack {
		size_t           size;
		size_t           base;
		size_t           pointer;
		crescent_Object* data;
	} stack;
};

typedef struct crescent_State crescent_State;

static void
crescent_resizeStack(crescent_State* state, int addAmount) {
	size_t newPointer = state->stack.pointer + addAmount;

	if (newPointer > state->stack.size) {
		abort();
	}

	crescent_Object* newData;

	if (newPointer > state->stack.size) {
		state->stack.size    += newPointer - state->stack.size + 16;
		state->stack.pointer += addAmount;

		newData = realloc(state->stack.data, state->stack.size);
	} else if (state->stack.size - newPointer >= 16) {
		state->stack.size    -= newPointer - state->stack.size + 8;
		state->stack.pointer += addAmount;

		newData = realloc(state->stack.data, state->stack.size);
	}

	if (state->stack.data == NULL) {
		free(state->stack.data);
		free(state);
		abort();
	}

	state->stack.data = newData;
}

crescent_State*
crescent_openState() {
	crescent_State* state = malloc(sizeof(crescent_State));

	if (state == NULL) {
		abort();
	}

	state->stack.size    = 64;
	state->stack.base    = 0;
	state->stack.pointer = 0;
	state->stack.data    = calloc(state->stack.size, sizeof(crescent_Object));

	if (state->stack.data == NULL) {
		free(state);
		abort();
	}

	return state;
}

void
crescent_closeState(crescent_State* state) {
	free(state->stack.data);
	free(state);
}

int
crescent_isInteger(crescent_State* state, size_t index) {
	return state->stack.data[index].type == CRESCENT_TINTEGER;
}

int
crescent_isFloat(crescent_State* state, size_t index) {
	return state->stack.data[index].type == CRESCENT_TFLOAT;
}

crescent_Integer
crescent_toInteger(crescent_State* state, size_t index, int* success) {
	if (state->stack.data[index].type != CRESCENT_TINTEGER) {
		*success = 0;
		return 0;
	}

	*success = 1;

	return state->stack.data[index].value.i;
}

crescent_Float
crescent_toFloat(crescent_State* state, size_t index, int* success) {
	if (state->stack.data[index].type != CRESCENT_TFLOAT) {
		*success = 0;
		return 0;
	}

	*success = 1;

	return state->stack.data[index].value.f;
}

void
crescent_pushInteger(crescent_State* state, crescent_Integer value) {
	const size_t stackIndex = state->stack.base + state->stack.pointer;

	crescent_resizeStack(state, 1);

	state->stack.data[stackIndex].type    = CRESCENT_TINTEGER;
	state->stack.data[stackIndex].value.i = value;
}

void
crescent_pushFloat(crescent_State* state, crescent_Float value) {
	const size_t stackIndex = state->stack.base + state->stack.pointer;

	crescent_resizeStack(state, 1);

	state->stack.data[stackIndex].type    = CRESCENT_TFLOAT;
	state->stack.data[stackIndex].value.f = value;
}
