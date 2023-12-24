#include "../prefix.h"
#include "../conf.h"

#include "object.h"

typedef struct {
	struct crescent_Stack {
		size_t            size;
		size_t            length;
		crescent_Object** data;
	} stack;
} crescent_State;

crescent_State*
crescentState_open() {
	crescent_State* state;
	state = malloc(sizeof(crescent_State));

	if (!state)
		abort();

	state->stack.size   = CRESCENT_STACK_MINFREE;
	state->stack.length = 0;
	state->stack.base   = 0; state->stack.data       = calloc(state->stack.size, sizeof(crescent_Object));

	if (!state->stack.data)
		abort();

	return state;
}

void
crescentState_close(crescent_State* state) {
	free(state->stack.data);
	free(state);
}

void
crescentState_reallocStack(crescent_State* state, size_t newLength) {
	if (newLength - state->stack.size < CRESCENT_STACK_MINFREE) {
		state->stack.size *= CRESCENT_STACK_GROWTHFACTOR;
		state->stack.data  = realloc(state->stack.data, state->stack.size);

		if (!state->stack.data)
			abort();
	}

	if (newLength - state->stack.size > CRESCENT_STACK_MINFREE) {
		state->stack.size /= CRESCENT_STACK_GROWTHFACTOR;
		state->stack.data  = realloc(state->stack.data, state->stack.size);

		if (!state->stack.data)
			abort();
	}

	state->stack.length = newLength;
}

void
crescentState_pushBoolean(crescent_State* state, crescent_Boolean b) {
	crescentState_reallocStack(state, state->stack.length + 1);

	crescent_Object* object;
	object = malloc(sizeof(crescent_Object));

	if (!object)
		abort();

	object->type    = TYPE_BOOLEAN;
	object->value.i = b;

	state->stack.data[state->stack.length - 1] = object;
}

void
crescentState_pushInteger(crescent_State* state, crescent_Integer i) {
	crescentState_reallocStack(state, state->stack.length + 1);

	crescent_Object* object;
	object = malloc(sizeof(crescent_Object));

	if (!object)
		abort();

	object->type    = TYPE_INTEGER;
	object->value.i = i;

	state->stack.data[state->stack.length - 1] = object;
}

void
crescentState_pushFloat(crescent_State* state, crescent_Float f) {
	crescentState_reallocStack(state, state->stack.length + 1);

	crescent_Object* object;
	object = malloc(sizeof(crescent_Object));

	if (!object)
		abort();

	object->type    = TYPE_FLOAT;
	object->value.f = f;

	state->stack.data[state->stack.length - 1] = object;
}

void
crescentState_pushString(crescent_State* state, crescent_String* s) {
	crescentState_reallocStack(state, state->stack.length + 1);

	crescent_Object* object;
	object = malloc(sizeof(crescent_Object));

	if (!object)
		abort();

	object->type    = TYPE_STRING;
	object->value.s = malloc(sizeof(crescent_String));

	if (!object->value.s)
		abort();

	object->value.s->size   = s->size;
	object->value.s->length = s->length;
	object->value.s->data   = calloc(s->size, sizeof(char));

	if (!object->value.s->data)
		abort();

	for (size_t a = 0; a < object->value.s->length; a++)
		object->value.s->data[a] = s->data[a];

	state->stack.data[state->stack.length - 1] = object;
}

void
crescentState_pushArray(crescent_State* state, crescent_Array* a) {
	crescentState_reallocStack(state, state->stack.length + 1);

	crescent_Object* object;
	object = malloc(sizeof(crescent_Object));

	if (!object)
		abort();

	object->type    = TYPE_ARRAY;
	object->value.a = malloc(sizeof(crescent_Array));

	if (!object->value.a)
		abort();

	object->value.a->size   = a->size;
	object->value.a->length = a->length;
	object->value.a->data   = calloc(a->size, sizeof(crescent_Object));

	if (!object->value.a->data)
		abort();

	for (size_t b = 0; b < a->length; b++)
		object->value.a->data[b] = a->data[b];

	state->stack.data[state->stack.length - 1] = object;
}
