/*
 * https://github.com/mochji/crescent
 * core/gc.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include <stdlib.h>
#include <stddef.h>

#include "crescent/conf.h"
#include "limit.h"

#include "types/string.h"
#include "types/table.h"
#include "types/function.h"
#include "types/userdata.h"
#include "core/object.h"
#include "core/state.h"
#include "core/memory.h"

#include "core/gc.h"

/* see type defs in limit.h */
#define istraversable(h) ((h)->type & 0x40)

#define linklist(h, l) {(h)->next = (l); (l) = (h);}
#define linkset(h, s)  {(h)->set = (s); (s) = (h);}

/*
 * During the sweep phase, we don't have to keep the invariant, and as such,
 * write barriers are not activated.
 */
#define keepinvariant(s) ((s)->gc.phase != CRS_GCPHASE_SWEEP)

#define WORK_FINALIZER 20

/*
 * ===========================
 *  generic
 * ===========================
 */

/* return parameter% of value, accounting for overflows */
static crs_mem applyParam(crs_mem value, unsigned short parameter) {
    if (value < 100) {
        return (value * parameter) / 100; /* shouldn't overflow */
    }

    if (parameter > CRS_MAX_MEM / (value / 100)) {
        return CRS_MAX_MEM; /* would overflow; return maximum value */
    }

    return ((value / 100) * parameter) + ((value % 100) * parameter) / 100;
}

static void freeObject(crs_Thread* thread, crs_GCHeader* header) {
    switch (header->type) {
        case CRS_TYPE_STRING:
            crsS_free(thread, obj_tostring(header));
            break;
        case CRS_TYPE_TABLE:
            crsT_free(thread, obj_totable(header));
            break;
        case CRS_TYPE_FUNCTION:
            crsK_free(thread, obj_tofunc(header));
            break;
        case CRS_TYPE_THREAD:
            crsE_freeThread(obj_tothread(header));
            break;
        case CRS_TYPE_USERDATA:
            crsU_free(thread, obj_toudata(header));
            break;
        default:
            assert(0);
    }
}

static int tryFinalizer(crs_Thread* thread, crs_UData* udata) {
    crs_Object obj;
    crs_Object mm;
    obj_setgc(&obj, udata);

    if (!crsM_getMM(thread, &mm, &obj, MT_GC)) {
        return 0;
    }

    obj_seto(thread->stack.top, &mm);
    obj_seto(thread->stack.top + 1, &obj);
    thread->stack.top += 2;
    gc_setfnz(obj_toheader(udata));

    if (crsM_pcall(thread, 1, 0) != CRS_OK) {
        crsE_warn(thread, "error in __gc metamethod");
    }

    return 1;
}

static void setPause(crs_State* state, crs_mem pause) {
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

static void mark_header(crs_State* state, crs_GCHeader* header) {
    if (!gc_iswhite(header)) {
        return;
    }

    if (istraversable(header)) {
        linkset(header, state->gc.gray);
        gc_setgray(header);
    } else {
        gc_setblack(header);
    }
}

#define mark_object(s, o) mark_header((s), obj_toheader(o))
#define mark_value(s, v) \
    {if (obj_iscollectable(v)) mark_header((s), obj_geth(v));}

/*
 * ===========================
 *  traversing
 * ===========================
 */

static crs_mem traverse_table(crs_State* state, crs_Table* table) {
    crs_TNode* node = table->table;
    crs_TNode* stop = node + table_nodes(table);

    while (node < stop) {
        if (node->value.type != CRS_TYPE_NIL) {
            mark_value(state, &node->key);
            mark_value(state, &node->value);
        }

        node++;
    }

    if (table->mt != NULL) {
        mark_object(state, table->mt);
    }

    return 2 + table_nodes(table);
}

static crs_mem traverse_debug(crs_State* state, crs_Function* func) {
    if (!(func->flags & FUNC_DEBUG)) {
        return 1;
    }

    Debug_Var* var = func->debug.vars;

    for (unsigned i = 0; i < func->debug.cV; i++) {
        mark_object(state, var->name);
        var++;
    }

    mark_object(state, func->debug.source);

    return (crs_mem)2 + func->debug.cV;
}

static crs_mem traverse_func(crs_State* state, crs_Function* func) {
    for (unsigned i = 0; i < func->cC; i++) {
        mark_value(state, &func->consts[i]);
    }

    for (unsigned i = 0; i < func->cN; i++) {
        mark_object(state, func->nested[i]);
    }

    return traverse_debug(state, func) + func->cC + func->cN;
}

static crs_mem traverse_thread(crs_State* state, crs_Thread* thread) {
    crs_Object* object = thread->stack.base;

    for (; object < thread->stack.top; object++) {
        mark_value(state, object);
    }

    mark_value(state, &thread->error);

    return 2 + (crs_mem)(thread->stack.top - thread->stack.base);
}

static crs_mem traverse_udata(crs_State* state, crs_UData* udata) {
    if (udata->mt != NULL) {
        mark_object(state, udata->mt);
    }

    return 2;
}

/*
 * ===========================
 *  list iteration
 * ===========================
 */

static crs_mem traverse(crs_State* state, int atomic) {
    crs_GCHeader* header = state->gc.gray;
    state->gc.gray       = header->set;

    switch (header->type) {
        case CRS_TYPE_TABLE:
            gc_setblack(header);
            return traverse_table(state, obj_totable(header));
        case CRS_TYPE_FUNCTION:
            gc_setblack(header);
            return traverse_func(state, obj_tofunc(header));
        case CRS_TYPE_THREAD:
            if (!atomic) {
                /*
                 * threads do not have write barriers, so all reachable threads
                 * must be kept gray and scanned again in the atomic phase.
                 */
                linkset(header, state->gc.grayAgain);
            } else {
                gc_setblack(header);
            }

            return traverse_thread(state, obj_tothread(header));
        case CRS_TYPE_USERDATA:
            gc_setblack(header);
            return traverse_udata(state, obj_toudata(header));
        default:
            assert(0);
    }

    return 1;
}

static crs_mem sweep(crs_Thread* thread) {
    crs_State*    state  = thread->state;
    crs_GCHeader* header = *state->gc.sweep;

    if (gc_iswhite(header)) {
        *state->gc.sweep = header->next; /* remove from list */

        if (header->type == CRS_TYPE_USERDATA && !gc_isfnz(header)) {
            linklist(header, state->gc.finalize);
        } else {
            freeObject(thread, header);
        }

        return 1;
    }

    state->gc.sweep = &header->next; /* advance sweep */
    gc_setwhite(header);

    return 1;
}

static crs_mem finalize(crs_Thread* thread) {
    crs_State*    state  = thread->state;
    crs_GCHeader* header = state->gc.finalize;
    state->gc.finalize   = header->next;

    if (tryFinalizer(thread, obj_toudata(header))) {
        linklist(header, state->gc.all);
        return WORK_FINALIZER;
    } else {
        freeObject(thread, header); /* no finalizer */
        return 1;
    }
}

static void sepToFnz(crs_State* state) {
    crs_GCHeader** list = &state->gc.all;
    crs_GCHeader*  header;

    while ((header = *list) != NULL) {
        if (header->type == CRS_TYPE_USERDATA && !gc_isfnz(header)) {
            *list = header->next;
            linklist(header, state->gc.finalize);
        } else {
            list = &header->next;
        }
    }
}

static void delete(crs_Thread* thread, crs_GCHeader* list, crs_GCHeader* stop) {
    crs_GCHeader* next = list;

    while (next != stop) {
        list = next;
        next = next->next;
        freeObject(thread, list);
    }
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
 *   Reset the sweep pointer and mark all objects in the root set.
 *
 * mark:
 *   Traverse an object in the gray set, marking white objects it references as
 *   gray. If the object doesn't need to be gray again, mark it as black;
 *   otherwise, keep it gray and add it to the grayAgain set for atomic
 *   traversal.
 *
 * atomic (atomic):
 *   Traverse the entire grayAgain set. By now, all reachable objects are marked
 *   as black, but no sweeping has begun yet, so it is safe to inspect
 *   structures.
 *
 * sweep:
 *   Sweep an item in the all list; if it's dead, remove it from the list and
 *   free its memory. Objects awaiting finalization are moved to the finalize
 *   list. After this phase, all remaining objects are white.
 *
 * finalize:
 *   Finalize an object in the finalize list and move it back to the all list.
 *   The finalizer may resurrect the object, so it is not immediately freed;
 *   only once a future cycle detects the object as dead will it be freed,
 *   though the finalizer will not be called again.
 */

static crs_mem step_restart(crs_State* state) {
    /* root set */
    mark_object(state, &state->thread);
    mark_value(state, &state->globals);

    for (int i = 0; i < CRS_TYPECOUNT; i++) {
        crs_Table* mt = state->mt[i];

        if (mt != NULL) {
            mark_object(state, mt);
        }
    }

    state->gc.phase = CRS_GCPHASE_MARK;
    state->gc.sweep = &state->gc.all;

    return CRS_MAX_MEM;
}

static crs_mem step_mark(crs_State* state) {
    if (state->gc.gray == NULL) {
        state->gc.phase = CRS_GCPHASE_ATOMIC;
        return 0;
    }

    return traverse(state, 0);
}

static crs_mem step_atomic(crs_State* state) {
    while (state->gc.gray != NULL) {
        traverse(state, 1);
    }

    state->gc.gray      = state->gc.grayAgain;
    state->gc.grayAgain = NULL;

    while (state->gc.gray != NULL) {
        traverse(state, 1);
    }

    crsS_clearCache(state);

    state->gc.phase = CRS_GCPHASE_SWEEP;
    return CRS_MAX_MEM;
}

static crs_mem step_sweep(crs_Thread* thread) {
    crs_State* state = thread->state;

    if (*state->gc.sweep == NULL) {
        state->gc.phase = CRS_GCPHASE_FINALIZE;
        return 0;
    }

    return sweep(thread);
}

static crs_mem step_finalize(crs_Thread* thread) {
    crs_State* state     = thread->state;
    ptrdiff_t  space     = thread->stack.last - thread->stack.base;
    int        emergency = gc_getstatus(state, EMERGENCY);

    /* finalizers may change the stack in unexpected ways */
    if (state->gc.finalize == NULL || space < CRS_MIN_FREE || emergency) {
        state->gc.phase = CRS_GCPHASE_RESTART;
        return CRS_MAX_MEM; /* end of cycle */
    }

    return finalize(thread);
}

static crs_mem step_single(crs_Thread* thread) {
    crs_State* state = thread->state;

    switch (state->gc.phase) {
        case CRS_GCPHASE_RESTART:
            return step_restart(state);
        case CRS_GCPHASE_MARK:
            return step_mark(state);
        case CRS_GCPHASE_ATOMIC:
            return step_atomic(state);
        case CRS_GCPHASE_SWEEP:
            return step_sweep(thread);
        case CRS_GCPHASE_FINALIZE:
            return step_finalize(thread);
        default:
            assert(0);
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

static void incremental_until(crs_Thread* thread, int phase) {
    do {
        step_single(thread);
    } while (thread->state->gc.phase != phase);
}

static void incremental_full(crs_Thread* thread) {
    crs_State* state = thread->state;

    if (!keepinvariant(state)) {
        /* finish sweep and reset everything back to white */
        incremental_until(thread, CRS_GCPHASE_RESTART);
    }

    incremental_until(thread, CRS_GCPHASE_RESTART);
    setPause(state, applyParam(state->gc.usage, gc_getparam(state, PAUSE)));
}

static int incremental_step(crs_Thread* thread) {
    crs_State*     state      = thread->state;
    unsigned short multiplier = gc_getparam(state, MULTIPLIER);
    crs_mem        work       = (state->gc.usage - state->gc.last) / 1024;

    if (multiplier > CRS_MAX_MEM / work) {
        work = CRS_MAX_MEM;
    } else {
        work *= multiplier;
    }

    while (work) {
        crs_mem done = step_single(thread);
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

void crsG_init(crs_State* state) {
    gc_setstatus(state, STOP, 0);
    gc_setstatus(state, EMERGENCY, 0);
    gc_setstatus(state, STOPEM, 0);

    gc_setparam(state, PAUSE, CRS_GCP_PAUSE);
    gc_setparam(state, STEP, CRS_GCP_STEP);
    gc_setparam(state, MULTIPLIER, CRS_GCP_MULTIPLIER);

    state->gc.usage     = sizeof(crs_State);
    state->gc.phase     = CRS_GCPHASE_RESTART;
    state->gc.all       = NULL;
    state->gc.finalize  = NULL;
    state->gc.immune    = NULL;
    state->gc.gray      = NULL;
    state->gc.grayAgain = NULL;

    setPause(state, applyParam(state->gc.usage, CRS_GCP_PAUSE));
}

void crsG_freeAll(crs_Thread* thread) {
    crs_State* state  = thread->state;
    thread->stack.top = thread->stack.base; /* ensure space for finalizers */

    sepToFnz(state);

    while (state->gc.finalize != NULL) {
        finalize(thread);
    }

    delete(thread, state->gc.all, obj_toheader(&state->thread));
    delete(thread, state->gc.immune, NULL);
}

void* crsG_add_(crs_Thread* thread, crs_GCHeader* header, crs_byte type) {
    crs_State* state = thread->state;

    linklist(header, state->gc.all);
    header->set  = NULL;
    header->type = type;
    header->mark = 0;
    gc_setwhite(header);

    /*
     * if the gc is sweeping, ensure the object doesn't get collected by moving
     * the sweep pointer past it if it isn't already.
     */
    if (!keepinvariant(state) && state->gc.sweep == &state->gc.all) {
        state->gc.sweep = &header->next;
    }

    return header;
}

void crsG_setImmune(crs_Thread* thread) {
    crs_State*    state  = thread->state;
    crs_GCHeader* header = state->gc.all;

    state->gc.all = header->next; /* remove from all list */

    /*
     * Immune objects are kept gray, and they will remain as such. Since they
     * are already marked (non-white), they will not be marked again and added
     * to the gray set or turned black. And since they are not in the all list,
     * they won't be swept and turned white either.
     *
     * This gives immunity to the object itself, but not any it references--
     * they must be referenced by an alive, non-immune object.
     */

    linklist(header, state->gc.immune);
    gc_setgray(header);
}

#define dobarrier(s, b, w) \
    ((gc_isblack(b) && gc_iswhite(w)) && keepinvariant(s))

/* move a white object to gray (or black) */
void crsG_barrierF(crs_Thread* thread, crs_GCHeader* header) {
    /* doesn't really matter if the parent object isn't black */
    mark_header(thread->state, header);
}

/* move a black object back to gray */
void crsG_barrierB(crs_Thread* thread, crs_GCHeader* black,
                                       crs_GCHeader* white) {
    crs_State* state = thread->state;

    if (dobarrier(state, black, white)) {
        linkset(black, state->gc.grayAgain);
        gc_setgray(black);
    }
}

int crsG_step(crs_Thread* thread) {
    crs_State* state = thread->state;

    if (gc_getstatus(state, STOP)) {
        setPause(state, 20000);
        return 0;
    }

    return incremental_step(thread);
}

int crsG_full(crs_Thread* thread, int emergency) {
    crs_State* state = thread->state;

    if (emergency && gc_getstatus(state, EMERGENCY)) {
        return 0;
    }

    gc_setstatus(state, EMERGENCY, emergency);
    incremental_full(thread);
    gc_setstatus(state, EMERGENCY, 0);

    return 1;
}
