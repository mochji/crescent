#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#define STACK_ALLOC_SIZE 64

typedef struct {
	size_t         top;
	size_t         size;
	unsigned char* data;
} crescent_Stack;

void crescent_initStack(crescent_Stack* stack) {
	stack->top  = 0;
	stack->size = STACK_ALLOC_SIZE;
	stack->data = malloc(STACK_ALLOC_SIZE);

	if (!stack->data) {
		perror("malloc");

		exit(EXIT_FAILURE);
	}
}

void crescent_freeStack(crescent_Stack* stack) {
	free(stack->data);
}

void crescent_push(crescent_Stack* stack, const unsigned char* data, const size_t size) {
	stack->top += size;

	if (stack->top >= stack->size) {
		stack->size = stack->top + STACK_ALLOC_SIZE;

		stack->data = realloc(stack->data, stack->size);

		if (!stack->data) {
			perror("realloc");

			exit(EXIT_FAILURE);
		}
	}

	for (size_t a = 0; a < size; a++)
		stack->data[stack->top - size + a] = data[a];
}

unsigned char* crescent_pop(crescent_Stack* stack, size_t size) {
	if (stack->top == 0) {
		fprintf(stderr, "attempt to pop empty stack\n");

		exit(EXIT_FAILURE);
	}

	unsigned char* poppedData;
	poppedData = malloc(size);

	if (!poppedData) {
		perror("malloc");

		exit(EXIT_FAILURE);
	}

	for (size_t a = 0; a < size; a++)
		poppedData[a] = stack->data[stack->top--];

	if (stack->size - stack->top > STACK_ALLOC_SIZE * 2) {
		stack->size = stack->top - STACK_ALLOC_SIZE;

		stack->data = realloc(stack->data, stack->size);

		if (!stack->data) {
			perror("malloc");

			exit(EXIT_FAILURE);
		}
	}

	return poppedData;
}
