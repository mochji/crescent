#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "../utils/apierr.h"

#define STACK_ALLOC_SIZE     64

#define STACK_MIN_FREE_CHARS 8
#define STACK_MAX_FREE_CHARS 64

typedef struct {
	size_t top;
	size_t size;
	char*  data;
} crescent_Stack;

void crescent_initStack(crescent_Stack* stack) {
	stack->top  = 0;
	stack->size = STACK_ALLOC_SIZE;
	stack->data = malloc(STACK_ALLOC_SIZE);

	if (!stack->data) {
		crescent_apiError("malloc failure");

		exit(EXIT_FAILURE);
	}
}

void crescent_freeStack(crescent_Stack* stack) {
	free(stack->data);
	stack->data = NULL;
}

void crescent_reallocStack(crescent_Stack* stack) {
	if (stack->size - stack->top < STACK_MIN_FREE_CHARS) {
		stack->size += STACK_MAX_FREE_CHARS - STACK_MIN_FREE_CHARS;
		stack->data  = realloc(stack->data, stack->size);

		if (!stack->data) {
			crescent_apiError("realloc failure");

			exit(EXIT_FAILURE);
		}
	}

	if (stack->size - stack->top > STACK_MAX_FREE_CHARS) {
		stack->size -= STACK_MAX_FREE_CHARS + STACK_MIN_FREE_CHARS;
		stack->data  = realloc(stack->data, stack->size);

		if (!stack->data) {
			crescent_apiError("realloc failure");

			exit(EXIT_FAILURE);
		}
	}
}

void crescent_stackPush(crescent_Stack* stack, unsigned char* data, size_t size) {
	stack->top += size;

	crescent_reallocStack(stack);

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

	crescent_reallocStack(stack);

	return poppedData;
}
