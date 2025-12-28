/*
 * https://github.com/mochji/crescent
 * core/gc.h
 *
 * Copyright (C) 2025 mochji
 * MIT License
 */

#ifndef CRS_CORE_GC_H
#define CRS_CORE_GC_H

#include "conf.h"
#include "limit.h"

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
#define CRS_BIT_STOPEM      2

#define gc_getstatus(s, n)    (bit_get((s)->gc.status, bit_mask(CRS_BIT_GC##n)) >> CRS_BIT_GC##n)
#define gc_setstatus(s, n, v) bit_change((s)->gc.status, (v) << CRS_BIT_GC##n, bit_mask(CRS_BIT_GC##n))

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
 *   How many bytes must be allocated for another step to begin; specifically
 *   2^step bytes.
 *
 * multiplier:
 *   Percent of the memory allocated since the last step (which may be more
 *   than the step parameter) that should be marked or swept in this step.
 *
 *   For example:
 *     - A value of 800 means it works at eight times the rate of allocation.
 *     - A value of 200 means it works at twice the rate of allocation.
 *     - A value of 100 means it works at the rate of allocation.
 *     - A value of 0 means no work is ever done in a step.
 *
 *   Values less than or equal to 100 will most likely cause the collector to
 *   never finish a cycle, as it cannot keep up with the rate of allocation.
 *   Or (to use a metaphor) the outflow of memory is less than or equal to the
 *   inflow, never completely draining.
 */

#define CRS_INDEX_GCPAUSE      0
#define CRS_INDEX_GCSTEP       1
#define CRS_INDEX_GCMULTIPLIER 2

/* these are only defaults; they can be changed during runtime */
#define CRS_GCP_PAUSE      100
#define CRS_GCP_STEP       12
#define CRS_GCP_MULTIPLIER 800

#define gc_getparam(s, n)    ((s)->gc.params[CRS_INDEX_GC##n])
#define gc_setparam(s, n, v) ((s)->gc.params[CRS_INDEX_GC##n] = (v))

#define CRS_GCPHASE_RESTART 0
#define CRS_GCPHASE_MARK    1
#define CRS_GCPHASE_ATOMIC  2
#define CRS_GCPHASE_SWEEP   3

#endif
