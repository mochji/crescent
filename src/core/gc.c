/*
 * https://github.com/mochji/crescent
 * core/gc.c
 *
 * Copyright (C) 2025 mochji
 * MIT License
 */

#include "conf.h"
#include "limit.h"

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

#define iswhite(h) bit_get((h)->mark, CRS_BIT_WHITE)
#define isblack(h) bit_get((h)->mark, CRS_BIT_BLACK)
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
 *
 * TODO: once i get it implemented, explain how new--and therefore white--
 * objects are not collected during the sweep phase
 */

#define keepinvariant(s) ((s)->gc.phase != CRS_GCPHASE_SWEEP)
