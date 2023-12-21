#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

typedef struct {
	size_t          top;
	size_t          size;
	unsigned char*  data;
} crescent_Stack;

void
crescent_initStack(
	crescent_Stack* stack
);
void
crescent_freeStack(
	crescent_Stack* stack
);
void
crescent_reallocStack(
	crescent_Stack* stack,
	size_t          newTop
);

void
crescent_stackPush(
	crescent_Stack* stack,
	unsigned char*  data,
	size_t          size
);

unsigned char*
crescent_stackPop(
	crescent_Stack* stack,
	size_t          size
);
