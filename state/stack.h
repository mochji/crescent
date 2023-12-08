#ifndef STACK_H
#define STACK_H

#define STACK_ALLOC_SIZE     64

#define STACK_MIN_FREE_CHARS 8
#define STACK_MAX_FREE_CHARS 64

typedef struct {
	size_t top;
	size_t size;
	char*  data;
} crescent_Stack;

void crescent_initStack(crescent_Stack* stack);
void crescent_freeStack(crescent_Stack* stack);

void crescent_reallocStack(crescent_String* stack);

void crescent_stackPush(crescent_Stack* stack, unsigned char* data, size_t size);
unsigned char* crescent_stackPop(crescent_Stack* stack, size_t size);

#endif
