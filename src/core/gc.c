/*
 * https://github.com/mochji/crescent
 * core/gc.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include <stdlib.h>

#include "conf.h"
#include "limit.h"

#include "types/string.h"
#include "types/array.h"
#include "core/object.h"
#include "core/state.h"
#include "core/memory.h"

#include "core/gc.h"

/*
 * Header mark byte
 *
 * - bit 0:    is white
 * - bit 1:    is black
 * - bits 2-7: unused and reserved
 *
 * An object is gray if it is neither white nor black. However, an object
 * cannot be both white and black.
 */

#define CRS_MASK_WHITE bit_mask(0)
#define CRS_MASK_BLACK bit_mask(1)
#define CRS_MASK_SET   (CRS_MASK_WHITE | CRS_MASK_BLACK)

#define iswhite(h) bit_get((h)->mark, CRS_MASK_WHITE)
#define isblack(h) bit_get((h)->mark, CRS_MASK_BLACK)
#define isgray(h)  (!bit_get((h)->mark, CRS_MASK_SET))

/* reset all set bits, then set the correct one */
#define setwhite(h) ((h)->mark = bit_reset((h)->mark, CRS_MASK_SET) | CRS_MASK_WHITE)
#define setblack(h) ((h)->mark = bit_reset((h)->mark, CRS_MASK_SET) | CRS_MASK_BLACK)
#define setgray(h)  ((h)->mark = bit_reset((h)->mark, CRS_MASK_SET))

/* see type enums in conf.h */
#define istraversable(h) ((h)->type & 4)
#define isgrayagain(h)   ((h)->type & 8)

#define linklist(h, l) {(h)->next = (l); (l) = (h);}
#define linkset(h, s)  {(h)->set = (s); (s) = (h);}

/*
 * During the sweep phase, we don't have to keep the invariant, and as such,
 * write barriers are not activated.
 */

#define keepinvariant(s) ((s)->gc.phase != CRS_GCPHASE_SWEEP)

/*
 * ===========================
 *  generic
 * ===========================
 */

/* return parameter% of value, accounting for overflows */
static crs_mem
applyParam(crs_mem value, unsigned short parameter) {
	if (value < 100) {
		return (value * parameter) / 100; /* shouldn't overflow */
	}

	if (parameter > CRS_MAX_MEM / (value / 100)) {
		return CRS_MAX_MEM; /* would overflow; return maximum value */
	}

	return ((value / 100) * parameter) + ((value % 100) * parameter) / 100;
}

static void
freeObject(crs_State* state, crs_GCHeader* header) {
	crs_Thread* thread = state->thread;

	switch (header->type) {
		case CRS_TYPE_STRING:
			crsS_free(thread, obj_tostring(header)); break;
		case CRS_TYPE_ARRAY:
			crsA_free(thread, obj_toarray(header)); break;
		case CRS_TYPE_THREAD:
			crsE_freeThread(obj_tothread(header)); break;
	}
}

static void
setPause(crs_State* state, crs_mem pause) {
	crs_mem usage = state->gc.usage;

	if (pause > CRS_MAX_MEM - usage) {
		state->gc.next = CRS_MAX_MEM;
	} else {
		state->gc.next = usage + pause;
	}

	state->gc.last = usage;
}

/*
 * ===========================
 *  marking
 * ===========================
 */

static void
mark_header(crs_State* state, crs_GCHeader* header) {
	if (!iswhite(header)) {
		return;
	}

	if (istraversable(header)) {
		linkset(header, state->gc.gray);
		setgray(header);
	} else {
		setblack(header);
	}
}

#define mark_object(s, i) mark_header((s), obj_toheader(i))
#define mark_value(s, o)  mark_header((s), obj_geth(o))

/*
 * ===========================
 *  traversing
 * ===========================
 */

static crs_mem
traverse_array(crs_State* state, crs_Array* array) {
	crs_Object* object = array->contents;

	for (size_t a = 0; a < array->length; a++) {
		mark_value(state, object);
		object++;
	}

	return 1 + array->length;
}

static crs_mem
traverse_thread(crs_State* state, crs_Thread* thread) {
	crs_Object* object = thread->stack.base;

	for (; object < thread->stack.top; object++) {
		mark_value(state, object);
	}

	return 1 + (object - thread->stack.base);
}

/*
 * ===========================
 *  list iteration
 * ===========================
 */

static crs_mem
traverse(crs_State* state, int atomic) {
	crs_GCHeader* header = state->gc.gray;
	state->gc.gray       = header->set;

	if (isgrayagain(header) && !atomic) {
		linkset(header, state->gc.grayAgain);
	} else {
		setblack(header);
	}

	switch (header->type) {
		case CRS_TYPE_ARRAY:
			return traverse_array(state, obj_toarray(header));
		case CRS_TYPE_THREAD:
			return traverse_thread(state, obj_tothread(header));
	}

	return 1;
}

static crs_mem
sweep(crs_State* state) {
	crs_GCHeader* header = *state->gc.sweep;

	if (iswhite(header)) {
		*state->gc.sweep = header->next; /* remove from list */
		freeObject(state, header);
	} else {
		state->gc.sweep = &header->next; /* advance sweep */
		setwhite(header);
	}

	return 1;
}

/*
 * ===========================
 *  steps
 * ===========================
 */

/*
 * GC phases
 *
 * restart (atomic):
 *   Reset the sweep pointer and mark the root objects (main thread). There is
 *   no need to reset the gray and grayAgain sets as it becomes NULL once the
 *   last element is removed.
 *
 * mark:
 *   Traverse an object in the gray set, marking white objects it references as
 *   gray. If the object doesn't need to be gray again, mark it as black;
 *   otherwise, keep it gray and add it to the grayAgain set for atomic
 *   traversal.
 *
 * atomic (atomic):
 *   Traverse the entire grayAgain list. By now, all reachable objects should
 *   be marked as black, so we can begin sweeping.
 *
 * sweep:
 *   Sweep an item in the all list; if it's dead, remove it from the list and
 *   free its memory.
 */

static crs_mem
step_restart(crs_State* state) {
	state->gc.sweep = &state->gc.all; /* reset sweep */
	mark_object(state, state->thread);

	state->gc.phase = CRS_GCPHASE_MARK;

	return CRS_MAX_MEM;
}

static crs_mem
step_mark(crs_State* state) {
	if (state->gc.gray == NULL) {
		state->gc.phase = CRS_GCPHASE_ATOMIC;

		return 0;
	}

	return traverse(state, 0);
}

static crs_mem
step_atomic(crs_State* state) {
	while (state->gc.gray != NULL) {
		traverse(state, 1);
	}

	state->gc.gray      = state->gc.grayAgain;
	state->gc.grayAgain = NULL;

	while (state->gc.gray != NULL) {
		traverse(state, 1);
	}

	state->gc.phase = CRS_GCPHASE_SWEEP;

	return CRS_MAX_MEM;
}

static crs_mem
step_sweep(crs_State* state) {
	if (*state->gc.sweep == NULL) {
		state->gc.phase = CRS_GCPHASE_RESTART;

		return CRS_MAX_MEM; /* end of cycle; don't do any more */
	}

	return sweep(state);
}

static crs_mem
step_single(crs_State* state) {
	switch (state->gc.phase) {
		case CRS_GCPHASE_RESTART:
			return step_restart(state);
		case CRS_GCPHASE_MARK:
			return step_mark(state);
		case CRS_GCPHASE_ATOMIC:
			return step_atomic(state);
		case CRS_GCPHASE_SWEEP:
			return step_sweep(state);
	}

	return 0;
}

/*
 * ===========================
 *  incremental gc
 * ===========================
 */

/*
 * GC work
 *
 * How much work is performed in a step is determined by units of work. One
 * work unit is equal to one object processed (marked, traversed, or swept).
 */

static void
incremental_until(crs_State* state, int phase) {
	do {
		step_single(state);
	} while (state->gc.phase != phase);
}

static void
incremental_full(crs_State* state) {
	if (!keepinvariant(state)) {
		/* finish sweep; reset everything back to white */
		incremental_until(state, CRS_GCPHASE_RESTART);
	}

	incremental_until(state, CRS_GCPHASE_RESTART);
	setPause(state, applyParam(state->gc.usage, gc_getparam(state, PAUSE)));
}

static int
incremental_step(crs_State* state) {
	unsigned short multiplier = gc_getparam(state, MULTIPLIER);
	crs_mem        work       = state->gc.last - state->gc.usage;

	work = applyParam(work / 1024, multiplier);

	while (work) {
		crs_mem done = step_single(state);
		work         = done > work ? 0 : work - done;
	}

	if (state->gc.phase == CRS_GCPHASE_RESTART) {
		setPause(state, applyParam(state->gc.usage, gc_getparam(state, PAUSE)));

		return 1;
	} else {
		setPause(state, gc_getparam(state, STEP));

		return 0;
	}
}

/*
 * ===========================
 *  control
 * ===========================
 */

/* returning as void removes the need to cast the type */
void*
crsG_new(crs_Thread* thread, crs_byte type, size_t size) {
	crs_State*    state  = thread->state;
	crs_GCHeader* header = mem_new(thread, size);

	linklist(header, state->gc.all);
	setwhite(header);
	header->set  = NULL;
	header->type = type;

	/*
	 * if the gc is sweeping, ensure the object doesn't get collected by moving
	 * the sweep pointer past it if it isn't already.
	 */
	if (!keepinvariant(state) && state->gc.sweep == &state->gc.all) {
		state->gc.sweep = &header->next;
	}

	return header;
}

void
crsG_setImmune(crs_Thread* thread) {
	crs_State*    state  = thread->state;
	crs_GCHeader* header = state->gc.all;

	state->gc.all = header->next; /* remove from all list */

	/*
	 * Immune objects are kept gray, and they will remain as such. Since they
	 * are already marked (non-white), they will not be marked again and added
	 * to the gray list or turned black. And since they are not in the all
	 * list, they won't be swept and turned white either.
	 *
	 * This gives immunity to the object itself, but not any it references--
	 * they must be referenced by an alive, non-immune object.
	 */

	linklist(header, state->gc.immune);
	setgray(header);
}

#define dobarrier(s, b, w) ((isblack(b) && iswhite(w)) && keepinvariant(s))

void
crsG_barrierF(crs_Thread* thread, crs_GCHeader* black, crs_GCHeader* white) {
	crs_State* state = thread->state;

	if (dobarrier(state, black, white)) {
		mark_header(state, white);
	}
}

void
crsG_barrierB(crs_Thread* thread, crs_GCHeader* black, crs_GCHeader* white) {
	crs_State* state = thread->state;

	if (dobarrier(state, black, white)) {
		linkset(black, state->gc.grayAgain);
		setgray(black);
	}
}

int
crsG_step(crs_Thread* thread) {
	crs_State* state = thread->state;

	if (gc_getstatus(state, STOP)) {
		setPause(state, 20000);

		return 0;
	}

	return incremental_step(state);
}

void
crsG_full(crs_Thread* thread, int emergency) {
	crs_State* state = thread->state;

	gc_setstatus(state, EMERGENCY, emergency);
	incremental_full(state);
	gc_setstatus(state, EMERGENCY, 0);
}
