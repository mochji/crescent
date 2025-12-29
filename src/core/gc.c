/*
 * https://github.com/mochji/crescent
 * core/gc.c
 *
 * Copyright (C) 2025 mochji
 * MIT License
 */

#include "conf.h"
#include "limit.h"

#include "core/object.h"
#include "core/state.h"

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
#define traversable(h) ((h)->type & 4)
#define grayagain(h)   ((h)->type & 8)

#define linklist(h, l) {(h)->next = (l); (l) = (h);}
#define linkset(h, s)  {(h)->set = (s); (s) = (h);}

/*
 * During the sweep phase, we don't have the keep the invariant, and as such,
 * write barriers are not activated.
 */

#define keepinvariant(s) ((s)->gc.phase != CRS_GCPHASE_SWEEP)

void
crsG_new(crs_Thread* thread, crs_GCHeader* header, crs_byte type, int immune) {
	crs_State* state = thread->state;

	if (immune) {
		/*
		 * immune objects are kept gray, and they will remain as such. since
		 * they are already marked (non-white), they will not be marked again
		 * and added to the gray list or turned black. and since they are not
		 * in the all list, they won't be swept and turned white either.
		 *
		 * this gives immunity to the object itself, but not to any it
		 * references--they must be referenced by another non-immune alive
		 * object.
		 */

		linklist(header, state->gc.immune);
		setgray(header);
	} else {
		linklist(header, state->gc.all);
		setwhite(header);

		/*
		 * if the gc is sweeping, move the sweep pointer past the new object if
		 * it isn't already past it.
		 */

		if (!keepinvariant(state) && state->gc.sweep == &state->gc.all) {
			state->gc.sweep = &header->next;
		}
	}

	header->set  = NULL;
	header->type = type;
}
