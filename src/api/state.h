#ifndef STATE_H
#define STATE_H

#include "../prefix.h"
#include "../conf.h"

#include "object.h"

typedef struct {
	unsigned int magic;
	struct crescent_Stack {
		size_t            size;
		size_t            length;
		crescent_Object** data;
	} stack;
} crescent_State;

crescent_State*
crescentState_open()

void
crescentState_close(crescent_State* state);

void
crescentState_reallocStack(crescent_State* state, size_t newLength);

void
crescentState_pushBoolean(crescent_State* state, crescent_Boolean b);

void
crescentState_pushInteger(crescent_State* state, crescent_Integer i);

void
crescentState_pushFloat(crescent_State* state, crescent_Float f);

void
crescentState_pushString(crescent_State* state, crescent_String* s);

void
crescentState_pushArray(crescent_State* state, crescent_Array* a);

#endif
