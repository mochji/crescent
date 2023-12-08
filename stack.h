#ifndef STACK_H
#define STACK_H

#define STACK_ALLOC_SIZE 64

typedef struct {
	size_t         top;
	size_t         size;
	unsigned char* data;
} crescent_Stack;

void crescent_initStack(crescent_Stack* stack);
void crescent_freeStack(crescent_Stack* stack);

void crescent_push(crescent_Stack* stack, const unsigned char* data, const size_t size);
unsigned char* crescent_pop(crescent_Stack* stack, size_t size);

#endif
