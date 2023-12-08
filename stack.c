#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "apierr.h"

#define STACK_ALLOC_SIZE 64

#define STACK_MAX_FREE_BYTES 64
#define STACK_MIN_FREE_BYTES 8

typedef struct {
	size_t         top;
	size_t         size;
	unsigned char* data;
} crescent_Stack;

void crescent_initStack(crescent_Stack* stack) {
	stack->top = 0;
	stack->size = STACK_ALLOC_SIZE;
	stack->data = malloc(STACK_ALLOC_SIZE);

	if (!stack->data) {
		crescent_apiError("malloc failure");

		exit(EXIT_FAILURE);
	}
}

void crescent_reallocStack(crescent_Stack* stack) {
	if (stack->size - stack->top < STACK_MIN_FREE_BYTES) {
		stack->size += STACK_MAX_FREE_BYTES - STACK_MIN_FREE_CHARS;
		stack->data  = realloc(stack->data, stack->size);

		if (!stack->data) {
			crescent_apiError("malloc failure");

			exit(EXIT_FAILURE);
		}
	}

	if (stack->size - stack->top > STACK_MAX_FREE_BYTES * 2) {
		stack->size -= STACK_MAX_FREE_BYTES;
		stack->data  = realloc(stack->data, stack->size);

		if (!stack->data) {
			crescent_apiError("realloc failure");

			exit(EXIT_FAILURE);
		}
	}
}

void crescent_pushStack(crescent_Stack* stack, unsigned char* bytes, size_t size) {
	stack->top += size;

	crescent_reallocStack(stack);

	for (size_t a = 0; a < size; a++)
		stack->data[a + stack->top - size] = bytes[a];
}

unsigned char* crescent_popStack(crescent_Stack* stack, size_t size) {
	if (stack->top < size) {
		crescent_apiError("attempt to pop more than stack top");

		exit(EXIT_FAILURE);
	}

	unsigned char* poppedBytes;
	poppedBytes = malloc(size);

	if (!poppedBytes) {
		crescent_apiError("malloc failure");

		exit(EXIT_FAILURE);
	}

	for (size_t a = 0; a < size; a++)
		poppedBytes[a] = stack->data[stack->top - 1 + a];

	stack->top -= size;
	stack->data[stack->top] = '\0';

	crescent_reallocStack(stack);
}
