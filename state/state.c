#include "state/stack.h"

typedef struct {
	crescent_Registers* registers;
	crescent_Stack*     stack;
	crescent_Globals*   globals;
} crescent_State;
