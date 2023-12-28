#include <stddef.h>

#include "../prefix.h"
#include "../conf.h"

#include "object.h"

struct crescent_Stack {
	size_t                size;
	size_t                pointer;
	size_t                base;
	struct crescent_Item* data;
};

struct crescent_State {
	struct crescent_Stack stack;
	// hashmap globals
	// status
};

struct crescent_State*
crescentState_open() {
	struct crescent_State* state;
	state = malloc(sizeof(struct crescent_State));

	if (!state)
		exit(1);

	state->stack.size    = 0;
	state->stack.base    = 0;
	state->stack.pointer = 0;
	state->stack.data    = calloc(CRESCENT_STACK_MAXFREE);

	if (!state->stack.data)
		exit(1);

	return state;
}

void
crescentState_resize(struct crescent_State* state, int amount) {
}

void crescentState_pushBoolean(struct crescent_State* state, crescent_Boolean value) {
	crescentState_resize(state, 1);

	struct crescent_Item* item;
	item = state->stack.data[state->stack.base + state->stack.pointer];

	item->type    = CRESCENT_TBOOLEAN;
	item->value.b = value;
}
