#include <stdlib.h>
#include <stddef.h>

#include "../conf.h"

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
	if (-addAmount > state->stack.pointer) {
		abort();
	}

	size_t newPointer = state->stack.pointer + addAmount;

	if (newPointer > state->stack.pointer || state->stack.size - newPointer <= CRESCENT_STACK_MINFREE) {
		state->stack.size = newPointer + CRESCENT_STACK_MAXFREE;
		state->stack.data = realloc(state->stack.data, state->stack.size);

		if (state->stack.data == NULL) {
			abort();
		}
	} else if (state->stack.size - newPointer >= CRESCENT_STACK_MAXFREE) {
		state->stack.size = newPointer + CRESCENT_STACK_MINFREE;
		state->stack.data = realloc(state->stack.data, state->stack.size);

		if (state->stack.data == NULL) {
			abort();
		}
	}
}

crescent_State
crescent_openState() {
	crescent_State* state = malloc(sizeof(crescent_State));

	if (state == NULL) {
		abort();
	}

	state->stack.size    = CRESCENT_STACK_INITSIZE;
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
crescent_closeState() {
}
