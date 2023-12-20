#ifndef STATE_H
#define STATE_H

#define crescent_apiError(msg) fprintf(stderr, "internal error!\n\n%s\n\tin internal function %s\n", msg, __func__); exit(EXIT_FAILURE);

#define STACK_MIN_FREE_BYTES 8
#define STACK_MAX_FREE_BYTES 64

typedef struct {
	size_t top;
	size_t size;
	char*  data;
} crescent_Stack;

void crescent_initStack(crescent_Stack* stack);
void crescent_freeStack(crescent_Stack* stack);

void crescent_reallocStack(crescent_String* stack, size_t newTop);

void crescent_stackPush(crescent_Stack* stack, unsigned char* data, size_t size);
unsigned char* crescent_stackPop(crescent_Stack* stack, size_t size);

#endif
