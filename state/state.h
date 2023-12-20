#ifndef STATE_H
#define STATE_H

typedef struct {
	crescent_Registers* registers;
	crescent_Stack*     stack;
	crescent_Globals*   globals;
} crescent_State;

#endif
