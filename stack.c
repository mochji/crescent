#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "prefix.h"
#include "conf.h"

typedef struct {
	size_t          top;
	size_t          size;
	int64_t*        data;
} crescent_Stack;

void crescent_initStack(crescent_Stack* stack) {
	stack->top  = 0;
	stack->size = STACK_MAX_FREE_BYTES;
	stack->data = malloc(STACK_MAX_FREE_BYTES);

	if (!stack->data)
		crescent_apiError("malloc failure");
}

void crescent_freeStack(crescent_Stack* stack) {
	free(stack->data);
	stack->data = NULL;
}

void crescent_reallocStack(crescent_Stack* stack, size_t newTop) {
	if (stack->size - newTop < STACK_MIN_FREE_BYTES) {
		stack->size += STACK_MAX_FREE_BYTES - STACK_MIN_FREE_BYTES;
		stack->data  = realloc(stack->data, stack->size);

		if (!stack->data)
			crescent_apiError("realloc failure");
	}

	if (stack->size - newTop > STACK_MAX_FREE_BYTES) {
		stack->size -= STACK_MAX_FREE_BYTES + STACK_MIN_FREE_BYTES;
		stack->data  = realloc(stack->data, stack->size);

		if (!stack->data)
			crescent_apiError("realloc failure");
	}

	stack->top = newTop;
}

void crescent_stackPush(crescent_Stack* stack, unsigned char* data, size_t size) {
	crescent_reallocStack(stack, stack->top + size);

	for (size_t a = stack->top - size; a < stack->top; a++)
		stack->data[a] = data[a];
}

unsigned char* crescent_stackPop(crescent_Stack* stack, size_t size) {
	unsigned char* poppedData;
	poppedData = malloc(size);

	for (size_t a = 0; a < size; a++) {
		poppedData[a] = stack->data[stack->top - 1 + a];
		stack->data[stack->top - 1 + a] = 0;
	}

	crescent_reallocStack(stack, stack->top - size);

	return poppedData;
}
