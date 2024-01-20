#ifndef CORE_STATE_H
#define CORE_STATE_H

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

extern int*
crescent_reallocStack(crescent_State* state, size_t newTop);

extern crescent_State*
crescent_openState();

extern void
crescent_closeState(crescent_State* state);

#endif
