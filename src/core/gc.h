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

extern int
crsG_init(crs_State* state);

extern void
crsG_freeAll(crs_State* state);

extern void*
crsG_add_(crs_Thread* thread, crs_GCHeader* header, crs_byte type);

#define crsG_add(t, o, b) crsG_add_((t), obj_toheader(o), (b))

extern void
crsG_setImmune(crs_Thread* thread);

extern void
crsG_barrierF(crs_Thread* thread, crs_GCHeader* black, crs_GCHeader* white);

extern void
crsG_barrierB(crs_Thread* thread, crs_GCHeader* black, crs_GCHeader* white);

extern int
crsG_step(crs_Thread* thread);

extern void
crsG_full(crs_Thread* thread, int emergency);

#define crsG_check(t) \
	((t)->state->gc.usage >= (t)->state->gc.next ? crsG_step(t) : 0)

#endif
