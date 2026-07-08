/*
 * https://github.com/mochji/crescent
 * core/call.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <setjmp.h>

#include "conf.h"
#include "limit.h"

#include "types/string.h"
#include "core/state.h"
#include "core/memory.h"
#include "core/format.h"
#include "vm/vm.h"

#include "core/call.h"

#define STACK_MAX (SIZE_MAX / sizeof(crs_Object))

/*
 * Error throwing
 *
 * When an error is thrown, the error object (usually a string containing the
 * error message) is saved in the thread.
 *
 * If there exists a handler (jmp_buf, created by pcall):
 * - longjmp back; pcall will handle the error.
 * Otherwise:
 * - Call the state's panic function. This is the last chance to jump out.
 * - If it returns, call abort.
 *
 * WARNING: As a pcall might catch and handle any error, be careful to ensure
 * the state of the thread when you throw one.
 */

void
crsC_throw(crs_Thread* thread, int status) {
	crs_Handler* handler = thread->handler;
	crs_State*   state   = thread->state;

	while (handler != NULL) {
		/* error in error handler? */
		if (handler->status != CRS_STATUS_OK) {
			handler         = handler->previous;
			thread->handler = handler;

			continue;
		}

		handler->status = status;
		longjmp(handler->buffer, 1);
	}

	if (state->panic != NULL) {
		state->panic(thread);
	}

	abort();
}

void
crsC_error(crs_Thread* thread, char* message) {
	crs_String* error = crsS_new(thread, message);

	obj_setgc(&thread->error, error);
	crsC_throw(thread, CRS_STATUS_ERROR);
}

void
crsC_errorf(crs_Thread* thread, char* format, ...) {
	crs_String* error;
	va_list     args;

	va_start(args, format);
	error = crsF_vformat(thread, format, args);
	va_end(args);

	obj_setgc(&thread->error, error);
	crsC_throw(thread, CRS_STATUS_ERROR);
}

void
crsC_restoreStack(crs_Thread* thread, short level) {
	while (thread->stack.level > level) {
		crs_Frame* frame    = thread->stack.frame;
		thread->stack.top   = frame->base;
		thread->stack.frame = frame->previous;
		thread->stack.level--;

		mem_free(thread, frame);
	}
}

int
crsC_try(crs_Thread* thread, crs_PFunction* function, void* data, void** result) {
	crs_Handler handler;
	void*       returned = NULL;

	handler.status   = CRS_STATUS_OK;
	handler.previous = thread->handler;
	thread->handler  = &handler;

	if (!setjmp(handler.buffer)) {
		returned = function(thread, data);
	}

	if (result != NULL) {
		*result = returned;
	}

	thread->handler = handler.previous;

	return handler.status;
}

static int
reallocStack(crs_Thread* thread, size_t newSize, int throw) {
	crs_Object* newStack = mem_vresize(thread,
		thread->stack.base, newSize, thread->stack.size);

	if (newStack == NULL) {
		if (throw) {
			crsM_error(thread);
		}

		return 1;
	}

	ptrdiff_t  offset = newStack - thread->stack.base;
	crs_Frame* frame  = thread->stack.frame;

	thread->stack.size = newSize;
	thread->stack.base = newStack;
	thread->stack.top += offset;

	while (frame != NULL) {
		frame->base += offset;
		frame        = frame->previous;
	}

	return 0;
}

int
crsC_resizeStack(crs_Thread* thread, size_t needed, int throw) {
	if (needed > STACK_MAX) {
		if (throw) {
			crsC_error(thread, "stack overflow");
		}

		return 1;
	}

	size_t size    = thread->stack.size;
	size_t newSize = needed + needed / 2;

	if (newSize > STACK_MAX) {
		newSize = needed;
	} else if (newSize < CRS_MIN_STACK) {
		newSize = CRS_MIN_STACK;
	}

	if (needed <= size / 3 && size > CRS_MIN_STACK) {
		reallocStack(thread, newSize, 0);

		return 0;
	} else if (needed > size) {
		return reallocStack(thread, newSize, throw);
	}

	return 0;
}

/* resize the stack frame to have 'top' elements */
int
crsC_checkTop(crs_Thread* thread, int top, int throw) {
	if (top > CRS_MAX_TOP) {
		if (throw) {
			crsC_error(thread, "stack overflow");
		}

		return 1;
	} else if (top < CRS_MIN_TOP) {
		top = CRS_MIN_TOP;
	}

	crs_Object* stack  = thread->stack.base;
	crs_Frame*  frame  = thread->stack.frame;
	size_t      needed = (frame->base - stack) + top;

	frame = frame->previous;

	while (frame != NULL) {
		size_t frameNeeds = (frame->base - stack) + frame->top;

		if (frameNeeds > needed) {
			needed = frameNeeds;
		}

		frame = frame->previous;
	}

	if (crsC_resizeStack(thread, needed, throw)) {
		return 1;
	}

	thread->stack.frame->top = top;

	return 0;
}

/* ensure there is at least 'free' free elements on the stack */
int
crsC_checkFree(crs_Thread* thread, int free, int throw) {
	size_t needed = (thread->stack.top - thread->stack.base) + free;

	if (needed > thread->stack.size) {
		return crsC_resizeStack(thread, needed, throw);
	}

	return 0;
}

/*
 * Calling and stack frames
 *
 * The stack is divided into frames. The base of a frame is where the first
 * object will go, and the top is how many stack spaces are reserved for the
 * frame.
 *
 * When a function is called and a new frame is added, the base is set
 * to the first empty element in the stack (top) offset by how many arguments
 * should be passed to it, such that those objects become part of the new
 * frame.
 *
 * When a function returns, the top 'results' elements are moved to the bottom
 * of the frame, placing them on top of the previous frame's elements. The
 * previous frame's top might need to be adjusted to accommodate the returned
 * objects.
 */

static void
checkResults(crs_Thread* thread, int wanted) {
	crs_Frame* frame    = thread->stack.frame;
	crs_Frame* previous = frame->previous;
	size_t     needed   = (frame->base - thread->stack.base) + wanted - 1;
	int        free     = previous->top - (frame->base - previous->base);

	/* previous frame cannot hold results? */
	if (wanted > CRS_MAX_TOP - free) {
		crsC_error(thread, "stack overflow");
	}

	/* need to grow stack? (only when wanted > results) */
	if (needed > thread->stack.size) {
		crsC_resizeStack(thread, needed, 1);
	}

	if (wanted > free) {
		previous->top += wanted - free;
	}
}

/* create and initialize a new stack frame */
static void
startCall(crs_Thread* thread, int top, int args) {
	if (thread->stack.level >= CRS_MAX_LEVEL) {
		crsC_error(thread, "stack overflow");
	}

	crsC_checkFree(thread, top - args, 1);

	crs_Frame* frame    = mem_new(thread, crs_Frame);
	crs_Frame* previous = thread->stack.frame;

	if (frame == NULL) {
		crsM_error(thread);
	}

	frame->base     = thread->stack.top - args;
	frame->top      = top;
	frame->previous = previous;

	thread->stack.frame = frame;
	thread->stack.level++;
}

/* return 'wanted' elements and pop the top stack frame */
static void
endCall(crs_Thread* thread, int results, int wanted) {
	crs_Frame* frame    = thread->stack.frame;
	crs_Frame* previous = frame->previous;
	int        top      = thread->stack.top - frame->base;

	/* only return as much as the frame has */
	results = results > top ? top : results;
	checkResults(thread, wanted);

	crs_Object* from = thread->stack.top - results;
	crs_Object* to   = frame->base;

	/* move top 'results' elements down to previous frame */
	if (results < top) {
		for (int i = 0; i < results; i++) {
			obj_seto(to, from);
			to++;
			from++;
		}
	}

	/* return nil for missing elements */
	for (int i = results; i < wanted; i++) {
		obj_setn(to);
		to++;
	}

	thread->stack.top  -= top - wanted;
	thread->stack.frame = previous;
	thread->stack.level--;

	mem_free(thread, frame);
}

void
crsC_call(crs_Thread* thread, crs_Function* function, int args, int wanted) {
	startCall(
		thread,
		function->top,
		args > function->args
			? function->args
			: args
	);

	/* fill missing args with nil */
	for (int i = args; i < function->args; i++) {
		obj_setn(thread->stack.top);
		thread->stack.top++;
	}

	endCall(thread, crsV_execute(thread, function), wanted);
}

void
crsC_callC(crs_Thread* thread, crs_CFunction* function, int args, int wanted) {
	startCall(
		thread,
		args < CRS_MIN_TOP
			? CRS_MIN_TOP
			: args,
		args
	);

	endCall(thread, function(thread), wanted);
}
