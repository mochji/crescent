#include <stdlib.h>
#include <stddef.h>

#include "conf.h"

#include "core/object.h"

struct
crescent_State {
	struct crescent_Stack {
		size_t           size;
		size_t           top;
		crescent_Object* data;
	} stack;
};

typedef struct crescent_State crescent_State;

int
crescent_reallocStack(crescent_State* state, size_t newTop) {
	if (state->stack.size - newTop >= CRESCENT_STACK_MINFREE && state->stack.size <= CRESCENT_STACK_MAXFREE) {
		return 0;
	}

	crescent_Object* newData = realloc(state->stack.data, state->stack.size);

	if (newData == NULL) {
		free(state->stack.data);
		free(state);

		return 1;
	}

	state->stack.size = newTop + CRESCENT_STACK_INITSIZE;
	state->stack.top  = newTop;
	state->stack.data = newData;

	return 0;
}

crescent_State*
crescent_openState() {
	crescent_State* state = malloc(sizeof(crescent_State));

	if (state == NULL) {
		return NULL;
	}

	state->stack.size = CRESCENT_STACK_INITSIZE;
	state->stack.top  = 0;
	state->stack.data = calloc(state->stack.size, sizeof(crescent_Object));

	if (state->stack.data == NULL) {
		free(state);

		return NULL;
	}

	return state;
}

void
crescent_closeState(crescent_State* state) {
	free(state->stack.data);
	free(state);
}
