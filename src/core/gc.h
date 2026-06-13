/*
 * https://github.com/mochji/crescent
 * core/gc.h
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#ifndef CRS_CORE_GC_H
#define CRS_CORE_GC_H

#include "conf.h"
#include "limit.h"

#include "core/object.h"
#include "core/state.h"

/*
 * GC status byte
 *
 * - bit 0:    is stopped
 * - bit 1:    is emergency
 * - bit 2:    stop emergency
 * - bits 3-7: unused and reserved
 */

#define CRS_BIT_GCSTOP      0
#define CRS_BIT_GCEMERGENCY 1
#define CRS_BIT_GCSTOPEM    2

#define gc_getstatus(s, n)    (bit_get((s)->gc.status, bit_mask(CRS_BIT_GC##n)) >> CRS_BIT_GC##n)
#define gc_setstatus(s, n, v) ((s)->gc.status = bit_change((s)->gc.status, (v) << CRS_BIT_GC##n, bit_mask(CRS_BIT_GC##n)))

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

#define gc_iswhite(h) bit_get((h)->mark, CRS_MASK_WHITE)
#define gc_isblack(h) bit_get((h)->mark, CRS_MASK_BLACK)
#define gc_isgray(h)  (!bit_get((h)->mark, CRS_MASK_SET))

/* reset all set bits, then set the correct one */
#define gc_setwhite(h) ((h)->mark = bit_reset((h)->mark, CRS_MASK_SET) | CRS_MASK_WHITE)
#define gc_setblack(h) ((h)->mark = bit_reset((h)->mark, CRS_MASK_SET) | CRS_MASK_BLACK)
#define gc_setgray(h)  ((h)->mark = bit_reset((h)->mark, CRS_MASK_SET))

/*
 * GC parameters
 *
 * pause:
 *   Percent that the memory usage must increase for another cycle to begin,
 *   with larger values having longer pauses.
 *
 *   For example:
 *     - A value of 100 means the memory usage must double.
 *     - A value of 200 means the memory usage must triple.
 *     - A value of 0 forces the collector to immediately start another cycle.
 *
 * step:
 *   How many bytes must be allocated for another step to begin in the same
 *   cycle.
 *
 * multiplier:
 *   Amount of objects that should be processed (marked, traversed, or swept)
 *   in a single step per kibibyte of memory allocated since the last step.
 */

#define CRS_INDEX_GCPAUSE      0
#define CRS_INDEX_GCSTEP       1
#define CRS_INDEX_GCMULTIPLIER 2

/* these are only defaults; they can be changed during runtime */
#define CRS_GCP_PAUSE      100
#define CRS_GCP_STEP       4096
#define CRS_GCP_MULTIPLIER 200

#define gc_getparam(s, n)    ((s)->gc.params[CRS_INDEX_GC##n])
#define gc_setparam(s, n, v) ((s)->gc.params[CRS_INDEX_GC##n] = (v))

#define CRS_GCPHASE_RESTART 0
#define CRS_GCPHASE_MARK    1
#define CRS_GCPHASE_ATOMIC  2
#define CRS_GCPHASE_SWEEP   3

void
crsG_init(crs_State* state);

void
crsG_freeAll(crs_State* state);

void*
crsG_add_(crs_Thread* thread, crs_GCHeader* header, crs_byte type);

#define crsG_add(t, o, b) crsG_add_((t), obj_toheader(o), (b))

void
crsG_setImmune(crs_Thread* thread);

void
crsG_barrierF_(crs_Thread* thread, crs_GCHeader* black, crs_GCHeader* white);

void
crsG_barrierB_(crs_Thread* thread, crs_GCHeader* black, crs_GCHeader* white);

#define crsG_barrierF(t, b, w) \
	{if (obj_iscollectable(w)) crsG_barrierF_((t), obj_toheader(b), obj_geth(w));}

#define crsG_barrierB(t, b, w) \
	{if (obj_iscollectable(w)) crsG_barrierB_((t), obj_toheader(b), obj_geth(w));}

int
crsG_step(crs_Thread* thread);

void
crsG_full(crs_Thread* thread, int emergency);

#define crsG_check(t) \
	((t)->state->gc.usage >= (t)->state->gc.next ? crsG_step(t) : 0)

#endif
