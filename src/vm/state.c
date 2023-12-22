#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "../prefix.h"
#include "../conf.h"

#include "registers.h"
#include "globals.h"

struct
crescent_Stack {
	size_t           top;
	size_t           size;
	crescent_Object* data;
};

typedef struct crescent_Stack crescent_Stack;

struct
crescent_State {
	crescent_Object registers[3];
	crescent_Stack  stack;
	crescent_HM     globals;
}

typedef struct crescent_State crescent_State;

crescent_State
crescent_newState() {
	crescent_State* state;
	state = malloc(sizeof(crescent_State));

	if (!state)
		crescent_apiError("malloc failure");

	state->stack.top  = 0;
	state->stack.size = CRESCENT_STACK_MAX_FREE;
	state->stack.data = malloc(CRESCENT_STACK_MAX_FREE);

	if (!state->stack.data)
		crescent_apiError("malloc failure");
}

void
crescent_closeState(crescent_State* state) {
	if (!state)
		crescent_apiError("attempt to close NULL state");

	free(state->stack.data);
	free(state->stack);
	free(state);

	state = NULL;
}
