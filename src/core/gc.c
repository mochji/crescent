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
	 * if the gc is sweeping, move the sweep pointer past the new object if it
	 * isn't already past it.
	 */
	if (!keepinvariant(state) && state->gc.sweep == &state->gc.all) {
		state->gc.sweep = &header->next;
	}

	return header;
}

/* set the newest object in the gc as immune */
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
