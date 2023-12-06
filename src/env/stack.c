/*
	env/stack.c
*/

#include <stdio.h>
#include <stddef.h>
#include "config.h"

struct CrescentStackFrame {
	size_t         base;               // Base pointer of this stack frame
	unsigned char  localCount;         // Local variables in this stack frame
	unsigned char* locals[MAX_LOCALS]; // Local variable data
};

struct CrescentStack {
	size_t         top;                               // Index of the next item in the stack
	size_t         maxSize;                           // Total allocated size of the stack
	size_t         topSize;                           // Size of the most recent item pushed onto the stack
	unsigned char* data;                              // Stack data
	CrescentStackFrame stackFrames[MAX_STACK_FRAMES]; //
};

typedef struct CrescentStack CrescentStack;

CrescentStack* crescent_newStack() {
	CrescentStack* stack;

	stack->top  = 0;
	stack->size = STACK_ALLOC_SIZE;
	stack->data = malloc(STACK_ALLOC_SIZE);

	if (!stack->data) {
		printf("Internal Crescent error: Failed allocating initial stack memory (%d).\n\t", STACK_ALLOC_SIZE);
		perror("malloc");

		exit(EXIT_FAILURE);
	}

	return stack;
}

