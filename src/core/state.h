#ifndef CORE_STATE_H
#define CORE_STATE_H

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
crescent_resizeStack(crescent_State* state, int addAmount);

crescent_State*
crescent_openState();

void
crescent_closeState(crescent_State* state);

int
crescent_isInteger(crescent_State* state, size_t index);

int
crescent_isFloat(crescent_State* state, size_t index);

crescent_Integer
crescent_toInteger(crescent_State* state, size_t index, int* success);

crescent_Float
crescent_toFloat(crescent_State* state, size_t index, int* success);

void
crescent_pushInteger(crescent_State* state, crescent_Integer value);

void
crescent_pushFloat(crescent_State* state, crescent_Float value);

#endif
