/*
 * https://github.com/mochji/crescent
 * core/call.c
 *
 * idrk what this is
 * Copyright (C) 2024 mochji
 *
 * MIT License
 */

#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "conf.h"

#include "core/object.h"
#include "core/state.h"

#include "core/call.h"

void
crescentC_setError(crescent_State* state, char* error) {
	if (state->error != state->gState->memoryError) {
		free(state->error);
	}

	if (error == NULL) {
		state->error = NULL;

		return;
	}

	state->error = malloc(strlen(error) + 1);

	if (state->error == NULL) {
		crescentC_memoryError(state);
	}

	strcpy(state->error, error);
}

void
crescentC_moveError(crescent_State* to, crescent_State* from) {
	if (from->error == from->gState->memoryError) {
		to->error = to->gState->memoryError;
	} else {
		to->error = from->error;
	}

	from->error = NULL;
}

void
crescentC_panic(crescent_State* state) {
	crescent_GState* gState = state->gState;

	if (gState->panic != NULL) {
		gState->panic(state);
	}

	abort();
}

void
crescentC_throw(crescent_State* state, int status) {
	crescent_GState* gState = state->gState;

	if (state->errorJump != NULL) {
		state->errorJump->status = status;
		longjmp(state->errorJump->buffer, 1);
	}

	if (gState->baseThread->errorJump != NULL) {
		crescentC_throw(gState->baseThread, status);
	}

	if (gState->panic != NULL) {
		gState->panic(state);
	}

	abort();
}

void
crescentC_memoryError(crescent_State* state) {
	char* memoryError = state->gState->memoryError;

	if (state->error != memoryError) {
		free(state->error);
	}

	state->error = memoryError;
	crescentC_throw(state, CRESCENT_STATUS_ERRMEM);
}

void
crescentC_correctStack(crescent_State* state, crescent_Object* stack) {
	ptrdiff_t       offset = stack - state->stack.base;
	crescent_Frame* frame  = state->stack.topFrame;

	state->stack.base = stack;
	state->stack.top += offset;

	while (frame != NULL) {
		frame->base += offset;
		frame        = frame->next;
	}
}

int
crescentC_reallocStack(crescent_State* state, size_t size) {
	crescent_Object* stack = realloc(state->stack.base, size * sizeof(crescent_Object));

	if (stack == NULL) {
		return 1;
	}

	crescentC_correctStack(state, stack);

	state->stack.size = size;

	return 0;
}

/*
 * Ensures the current stack frame has at least top + 2 stack indexes
 * available.
 *
 * If the new absolute top is 1/3 or less of the current stack size, it will
 * attempt to shrink it to half of the size unless the stack size is
 * CRESCENT_STACK_INITSIZE.
 *
 * If the new absolute top is greater than the current stack size - 2, it will
 * attempt to grow it to the new absolute top * 1.5.
 *
 * If it fails to reallocate the stack and throw is set, it will throw
 * an out of memory error.
 */

int
crescentC_resizeStack(crescent_State* state, int top, int throw) {
	size_t absTop = (size_t)(state->stack.topFrame->base - state->stack.base) + top;
	int    failed = 0;

	if (absTop <= state->stack.size / 3) {
		if (state->stack.size == CRESCENT_STACK_INITSIZE) {
			return 0;
		}

		size_t size = state->stack.size / 2;

		if (size <= CRESCENT_STACK_INITSIZE) {
			size = CRESCENT_STACK_INITSIZE;
		}

		/*
		 * even if reallocating the stack fails here, we can just gaslight,
		 * gatekeep and girlboss since it doesnt matter.
		 */

		crescentC_reallocStack(state, size);
	} else if (absTop >= state->stack.size - 2) {
		failed = crescentC_reallocStack(state, absTop + absTop / 2);
	}

	if (failed && throw) {
		crescentC_memoryError(state);
	}

	return failed;
}

void
crescentC_startCall(crescent_State* state, int args, crescent_Frame* newTopFrame) {
	crescent_Frame* oldTopFrame = state->stack.topFrame;

	oldTopFrame->top -= args;
	oldTopFrame->next = newTopFrame;

	newTopFrame->base     = state->stack.top - args;
	newTopFrame->top      = args;
	newTopFrame->next     = NULL;
	newTopFrame->previous = oldTopFrame;

	state->stack.calls   += 1;
	state->stack.cCalls  += 1;
	state->stack.topFrame = newTopFrame;
}

void
crescentC_endCall(crescent_State* state, int results) {
	crescent_Frame* newTopFrame = state->stack.topFrame;
	crescent_Frame* oldTopFrame = newTopFrame->previous;

	if (results != newTopFrame->top) {
		crescent_Object* fromBase;
		crescent_Object* toBase;

		toBase = newTopFrame->base;

		for (int a = 0; a < newTopFrame->top - results; a++) {
			crescentO_free(toBase + a);
		}

		fromBase = state->stack.top - results;
		toBase   = newTopFrame->base;

		for (int a = 0; a < results; a++) {
			*(toBase + a) = *(fromBase + a);
		}
	}

	oldTopFrame->top += results;
	oldTopFrame->next = NULL;

	state->stack.calls   -= 1;
	state->stack.cCalls  -= 1;
	state->stack.topFrame = oldTopFrame;
}
