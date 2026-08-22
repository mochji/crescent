/*
 * https://github.com/mochji/crescent
 * compiler/chunk.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <limits.h>

#include "crescent/conf.h"
#include "limit.h"

#include "types/table.h"
#include "types/function.h"
#include "core/state.h"
#include "core/memory.h"
#include "core/format.h"
#include "vm/opcodes.h"
#include "vm/vm.h"

#include "compiler/chunk.h"

static void limitError(Chunk* chunk, unsigned max, char* what) {
    crsI_error(chunk, "too many %s (limit is %u)", what, max);
}

static void data_init(Data* data, void** vec, unsigned* size,
                                  unsigned max, size_t type) {
    max = max > SIZE_MAX / type
        ? (unsigned)(SIZE_MAX / type)
        : max;

    data->vec   = vec;
    data->size  = size;
    data->count = 0;
    data->max   = max;
    data->type  = type;
}

static void data_resize(Chunk* chunk, Data* data, unsigned size) {
    void* vec = mem_realloc(chunk->thread, *data->vec,
        size * data->type, *data->size * data->type);

    if (vec == NULL) {
        crsM_error(chunk->thread);
    }

    *data->vec  = vec;
    *data->size = size;
}

/* ensure space for at least one more element */
static unsigned data_check(Chunk* chunk, Data* data, char* what) {
    unsigned size = *data->size;
    unsigned max  = data->max;

    if (data->count < size) {
        return data->count++;
    }

    if (size == max) {
        limitError(chunk, max, what);
    } else if (size > max / 2) {
        size = max;
    } else {
        size *= 2;
    }

    data_resize(chunk, data, size);
    return data->count++;
}

/* finalize a function vector */
static void data_shrink(Chunk* chunk, Data* data) {
    if (data->count) {
        data_resize(chunk, data, data->count);
    } else {
        mem_dealloc(chunk->thread, *data->vec, *data->size * data->type);
        *data->vec  = NULL;
        *data->size = 0;
    }
}

static void data_free(Chunk* chunk, Data* data) {
    if (data->vec != NULL) {
        mem_dealloc(chunk->thread, *data->vec, *data->size * data->type);
    }
}

noret crsI_error(Chunk* chunk, char* format, ...) {
    crs_Thread* thread = chunk->thread;
    crs_String* error;
    va_list     args;

    va_start(args, format);
    error = crsF_vformat(thread, format, args);
    va_end(args);

    obj_setgc(&thread->error, error);
    crsC_throw(thread, CRS_STATUS_CODEERR);
}

void crsI_init(crs_Thread* thread, Parser* parser) {
    Variable* vars   = mem_vnew(thread, 32, Variable);
    Label*    labels = mem_vnew(thread, 16, Label);

    if (vars == NULL || labels == NULL) {
        parser->vars.vec   = NULL;
        parser->labels.vec = NULL;
        crsM_error(thread);
    }

    parser->vecs.vars   = vars;
    parser->vecs.labels = labels;
    data_init(&parser->vars, (void**)&parser->vecs.vars, &parser->vecs.sV,
        UINT_MAX, sizeof(Variable));
    data_init(&parser->labels, (void**)&parser->vecs.labels, &parser->vecs.sL,
        UINT_MAX, sizeof(Label));
}

void crsI_free(crs_Thread* thread, Parser* parser) {
    Chunk temp = {.thread = thread};
    data_free(&temp, &parser->vars);
    data_free(&temp, &parser->labels);
}

/*
 * ===========================
 *  chunk
 * ===========================
 */

crs_Function* crsI_newChunk(Chunk* chunk, crs_Thread* thread, Parser* parser) {
    crs_Function* func = crsK_new(thread, 32, 8, 8);

    chunk->thread = thread;
    chunk->parser = parser;
    chunk->scope  = NULL;
    chunk->regs   = 0;
    chunk->locals = 0;
    chunk->fV     = parser->vars.count;
    chunk->func   = func;
    data_init(&chunk->code, (void**)&func->code, &func->nI,
        UINT_MAX, sizeof(crs_instr));
    data_init(&chunk->consts, (void**)&func->consts, &func->nC,
        UINT_MAX, sizeof(crs_Object));
    data_init(&chunk->nested, (void**)&func->nested, &func->nN,
        UINT_MAX, sizeof(crs_Function*));

    return func;
}

void crsI_finish(Chunk* chunk) {
    data_shrink(chunk, &chunk->code);
    data_shrink(chunk, &chunk->consts);
    data_shrink(chunk, &chunk->nested);
}

unsigned crsI_nested(Chunk* parent, Chunk* chunk) {
    Data*         nested = &parent->nested;
    unsigned      index  = data_check(chunk, nested, "nested functions");
    crs_Function* func   = crsI_newChunk(chunk, parent->thread, parent->parser);

    parent->func->nested[parent->func->cN++] = func;
    return index;
}

/*
 * ===========================
 *  scope
 * ===========================
 */

void crsI_enter(Chunk* chunk, Scope* scope, int loop) {
    Parser* parser   = chunk->parser;
    Scope*  previous = chunk->scope;
    chunk->scope     = scope;

    loop            = loop || (previous != NULL ? previous->inLoop : 0);
    scope->previous = previous;
    scope->inLoop   = loop;
    scope->fL       = loop ? parser->labels.count : previous->fL;
    scope->nV       = 0;
    /* only expose labels for current loop */
}

void crsI_leave(Chunk* chunk) {
    Parser* parser   = chunk->parser;
    Scope*  scope    = chunk->scope;
    Scope*  previous = scope->previous;
    chunk->scope     = previous;

    parser->labels.count = scope->fL;
    parser->vars.count  -= scope->nV;
    chunk->regs         -= scope->nV;
}

/*
 * ===========================
 *  code
 * ===========================
 */

unsigned crsI_emit(Chunk* chunk, crs_instr i) {
    unsigned pc = data_check(chunk, &chunk->code, "instructions");

    chunk->func->code[pc] = i;
    return pc;
}

static crs_instr createJump(Chunk* chunk, unsigned pc, unsigned target) {
    unsigned sign;
    unsigned magnitude;

    if (target > pc) {
        sign      = 0;
        magnitude = target - pc;
    } else {
        sign      = 1;
        magnitude = pc - target;
    }

    if (magnitude > MAX_sAxx) {
        crsI_error(chunk, "control structure too long");
    }

    return instr_newisAxx(OP_JMP, magnitude, sign);
}

/*
 * ===========================
 *  backpatching
 * ===========================
 */

/* find or create a new label */
static Label* findLabel(Chunk* chunk, crs_String* name) {
    Parser*  parser = chunk->parser;
    unsigned count  = parser->labels.count - chunk->scope->fL;
    Label*   label  = parser->vecs.labels + chunk->scope->fL;

    while (count--) {
        if (label->name == name) {
            return label;
        }

        label++;
    }

    unsigned index = data_check(chunk, &parser->labels, "labels");
    label          = &parser->vecs.labels[index];
    label->list    = PATCH_NONE;
    label->pc      = PATCH_NONE;

    return label;
}

/* add an instruction to a backpatch list */
void crsI_jump(Chunk* chunk, unsigned* list) {
    /* 'crs_instr' is at least 'unsigned' */
    *list = crsI_emit(chunk, (crs_instr)*list);
}

/* fix all instructions in list to jump to 'target' */
void crsI_backpatch(Chunk* chunk, unsigned list, unsigned target) {
    crs_instr* code = chunk->func->code;

    while (list != PATCH_NONE) {
        unsigned pc = list;
        list        = (unsigned)code[pc];
        code[pc]    = createJump(chunk, pc, target);
    }
}

/* create new label 'name' */
void crsI_label(Chunk* chunk, crs_String* name) {
    Label* label = findLabel(chunk, name);
    label->pc    = crsI_nextPC(chunk);
}

/* add instruction to jump to label 'name' */
void crsI_goto(Chunk* chunk, crs_String* name) {
    Label* label = findLabel(chunk, name);
    crsI_jump(chunk, &label->list);
}

/* fix all jumps to labels in current scope (innermost loop) */
void crsI_patchAll(Chunk* chunk) {
    Parser*  parser = chunk->parser;
    unsigned count  = parser->labels.count - chunk->scope->fL;
    Label*   label  = parser->vecs.labels + chunk->scope->fL;

    while (count--) {
        assert(label->pc != PATCH_NONE); /* label should exist */
        crsI_backpatch(chunk, label->list, label->pc);
        label++;
    }
}

/*
 * ===========================
 *  constants
 * ===========================
 */

static unsigned const_obj(Chunk* chunk, crs_Object* value) {
    crs_Object* consts = chunk->func->consts;
    unsigned    index;

    for (unsigned i = 0; i < chunk->consts.count; i++) {
        crs_Object* object = &consts[i];

        if (crsV_equal(object, value) && object->type == value->type) {
            return i;
        }
    }

    index = data_check(chunk, &chunk->consts, "constants");
    obj_seto(&chunk->func->consts[index], value);
    chunk->func->cC++;

    return index;
}

static unsigned const_int(Chunk* chunk, crs_Integer value) {
    crs_Object object;
    obj_seti(&object, value);
    return const_obj(chunk, &object);
}

static unsigned const_float(Chunk* chunk, crs_Float value) {
    crs_Object object;
    obj_setf(&object, value);
    return const_obj(chunk, &object);
}

static unsigned const_str(Chunk* chunk, crs_String* value) {
    crs_Object object;
    obj_setgc(&object, value);
    return const_obj(chunk, &object);
}

/*
 * ===========================
 *  expressions
 * ===========================
 */

/*
 * register allocation is only used for expressions; locals survive from their
 * declaration to the end of their scope and occupy the bottom registers.
 */

static crs_byte reg_new(Chunk* chunk) {
    if (chunk->regs == MAX_REGS) {
        crsI_error(chunk, "expression too complex");
    } else if (chunk->regs == chunk->func->top) {
        chunk->func->top++;
    }

    return chunk->regs++;
}

static void reg_free(Chunk* chunk, crs_byte reg) {
    if (reg + 1 == chunk->regs && reg >= chunk->locals) {
        chunk->regs--;
    }
}

static void reg_free2(Chunk* chunk, crs_byte reg1, crs_byte reg2) {
    if (reg1 > reg2) {
        reg_free(chunk, reg1);
        reg_free(chunk, reg2);
    } else {
        reg_free(chunk, reg2);
        reg_free(chunk, reg1);
    }
}

static void reg_freeExp(Chunk* chunk, Expression* exp) {
    if (exp->type == EXP_TEMP) {
        reg_free(chunk, (crs_byte)exp->value.v);
    }
}

static void reg_free2Exp(Chunk* chunk, Expression* exp1, Expression* exp2) {
    if (exp1->type != EXP_TEMP) {
        reg_freeExp(chunk, exp2);
    } else if (exp2->type != EXP_TEMP) {
        reg_free(chunk, (crs_byte)exp1->value.v);
    } else {
        reg_free2(chunk, (crs_byte)exp1->value.v, (crs_byte)exp2->value.v);
    }
}

static void reg_freeList(Chunk* chunk, Expression* exp) {
    assert(exp->type == EXP_LIST || exp->type == EXP_VLIST);
    chunk->regs -= (crs_byte)exp->value.v;
}

void crsI_freeExp(Chunk* chunk, Expression* exp) {
    reg_freeExp(chunk, exp);
}

/*
 * subscripted expressions need to be flattened before any more registers are
 * allocated, as 'obj' and 'key' would become trapped and unable to be freed
 * otherwise.
 */
static void flatten_index(Chunk* chunk, Expression* exp, crs_byte dest) {
    crs_byte obj = exp->value.x.obj;
    crs_byte key = exp->value.x.key;
    reg_free2(chunk, obj, key);

    if (dest == MAX_REGS) {
        dest = reg_new(chunk);
    }

    crsI_iABC(chunk, OP_GET, dest, obj, key);
    exp->type    = EXP_TEMP;
    exp->value.v = dest;
}

static void flatten_call(Chunk* chunk, Expression* exp, crs_byte dest) {
    crs_instr* i = &chunk->func->code[exp->value.v];
    *i          |= instr_setC(1);

    if (dest == MAX_REGS) {
        dest = reg_new(chunk);
    }

    exp->type    = EXP_TEMP;
    exp->value.v = dest;
}

int crsI_flatten(Chunk* chunk, Expression* exp) {
    switch (exp->type) {
        case EXP_INDEX:
            flatten_index(chunk, exp, MAX_REGS);
            return 1;
        case EXP_CALL:
            flatten_call(chunk, exp, MAX_REGS);
            return 1;
    }

    return 0;
}

static void load_const(Chunk* chunk, unsigned index, crs_byte reg) {
    crsI_iABx(chunk, OP_LODC, reg, index);
}

static void load_int(Chunk* chunk, crs_Integer value, crs_byte reg) {
    unsigned sign;
    unsigned magnitude;

    if (value >= MAX_sBx || value <= -MAX_sBx) {
        load_const(chunk, const_int(chunk, value), reg);
        return;
    } else if (value < 0) {
        sign      = 1;
        magnitude = (unsigned)-value;
    } else {
        sign      = 0;
        magnitude = (unsigned)value;
    }

    crsI_iAsBx(chunk, OP_LODI, reg, magnitude, sign);
}

static void load_global(Chunk* chunk, crs_String* value, crs_byte reg) {
    unsigned index = const_str(chunk, value);
    crsI_iABx(chunk, OP_GETG, reg, index);
}

/* 'exp' is potentially invalid after return */
crs_byte crsI_store(Chunk* chunk, Expression* exp, crs_byte reg) {
    switch (exp->type) {
        case EXP_NIL:
            crsI_iABC(chunk, OP_LODN, reg, 0, 0);
            break;
        case EXP_TRUE:
            crsI_iABC(chunk, OP_LODT, reg, 0, 0);
            break;
        case EXP_FALSE:
            crsI_iABC(chunk, OP_LODF, reg, 0, 0);
            break;
        case EXP_INT:
            load_int(chunk, exp->value.i, reg);
            break;
        case EXP_FLOAT:
            load_const(chunk, const_float(chunk, exp->value.f), reg);
            break;
        case EXP_STRING:
            load_const(chunk, const_str(chunk, exp->value.s), reg);
            break;
        case EXP_GLOBAL:
            load_global(chunk, exp->value.s, reg);
            break;
        case EXP_LOCAL:
        case EXP_TEMP:
            crsI_iABC(chunk, OP_MOV, reg, exp->value.v, 0);
            break;
        case EXP_FUNC:
            crsI_iABx(chunk, OP_LODK, reg, exp->value.v);
            break;
        case EXP_INDEX:
            flatten_index(chunk, exp, reg);
            break;
        case EXP_CALL:
            flatten_call(chunk, exp, reg);
            break;
        default:
            assert(0);
    }

    return reg;
}

/* ensure 'exp' is in a register */
void crsI_toAny(Chunk* chunk, Expression* exp) {
    if (!exp_inreg(exp) && !crsI_flatten(chunk, exp)) {
        exp->type    = EXP_TEMP;
        exp->value.v = crsI_store(chunk, exp, reg_new(chunk));
    }
}

/* ensure 'exp' is in the top temporary register */
void crsI_toTop(Chunk* chunk, Expression* exp) {
    assert(exp->type != EXP_TEMP || exp->value.v + 1 == chunk->regs);

    if (exp->type != EXP_TEMP && !crsI_flatten(chunk, exp)) {
        exp->type    = EXP_TEMP;
        exp->value.v = crsI_store(chunk, exp, reg_new(chunk));
    }
}

static int fold_toObject(Expression* exp, crs_Object* object) {
    if (!exp_canfold(exp)) {
        return 0;
    }

    switch (exp->type) {
        case EXP_NIL:
            obj_setn(object);
            break;
        case EXP_TRUE:
            obj_setb(object, 1);
            break;
        case EXP_FALSE:
            obj_setb(object, 0);
            break;
        case EXP_INT:
            obj_seti(object, exp->value.i);
            break;
        case EXP_FLOAT:
            obj_setf(object, exp->value.f);
            break;
    }

    return 1;
}

static int fold_fromObject(Expression* exp, crs_Object* object) {
    switch (object->type) {
        case CRS_TYPE_NIL:
            exp->type = EXP_NIL;
            return 1;
        case CRS_TYPE_BOOLEAN:
            exp->type = obj_getb(object) ? EXP_TRUE : EXP_FALSE;
            return 1;
        case CRS_TYPE_INTEGER:
            exp->type    = EXP_INT;
            exp->value.i = obj_geti(object);
            return 1;
        case CRS_TYPE_FLOAT:
            if (obj_getf(object) != obj_getf(object)) {
                return 0;
            }

            exp->type    = EXP_FLOAT;
            exp->value.f = obj_getf(object);
            return 1;
    }

    return 0;
}

static int fold_unary(Expression* exp, int uop) {
    crs_Object object;
    int        success = 0;

    if (!fold_toObject(exp, &object)) {
        return 0;
    }

    switch (uop) {
        case UOP_UNM:
            success = crsV_rawArith(&object, &object, &object, CRS_OP_UNM);
            break;
        case UOP_BNOT:
            success = crsV_rawArith(&object, &object, &object, CRS_OP_BNOT);
            break;
        case UOP_NOT: {
            int value = !crsO_test(&object);
            obj_setb(&object, value);

            success = 1;
            break;
        }
    }

    return success ? fold_fromObject(exp, &object) : 0;
}

static int fold_binary(Expression* rhs, Expression* lhs, int bop) {
    crs_Object lObj, rObj;
    int        success = 0;

    if (!fold_toObject(lhs, &lObj) || !fold_toObject(rhs, &rObj)) {
        return 0;
    }

    switch (bop) {
        case BOP_ADD:
            success = crsV_rawArith(&lObj, &lObj, &rObj, CRS_OP_ADD);
            break;
        case BOP_SUB:
            success = crsV_rawArith(&lObj, &lObj, &rObj, CRS_OP_SUB);
            break;
        case BOP_MUL:
            success = crsV_rawArith(&lObj, &lObj, &rObj, CRS_OP_MUL);
            break;
        case BOP_DIV:
            success = crsV_rawArith(&lObj, &lObj, &rObj, CRS_OP_DIV);
            break;
        case BOP_POW:
            success = crsV_rawArith(&lObj, &lObj, &rObj, CRS_OP_POW);
            break;
        case BOP_MOD:
            success = crsV_rawArith(&lObj, &lObj, &rObj, CRS_OP_MOD);
            break;
        case BOP_SHL:
            success = crsV_rawArith(&lObj, &lObj, &rObj, CRS_OP_SHL);
            break;
        case BOP_SHR:
            success = crsV_rawArith(&lObj, &lObj, &rObj, CRS_OP_SHR);
            break;
        case BOP_BAND:
            success = crsV_rawArith(&lObj, &lObj, &rObj, CRS_OP_BAND);
            break;
        case BOP_BXOR:
            success = crsV_rawArith(&lObj, &lObj, &rObj, CRS_OP_BXOR);
            break;
        case BOP_BOR:
            success = crsV_rawArith(&lObj, &lObj, &rObj, CRS_OP_BOR);
            break;
        case BOP_EQ: {
            int value = crsV_equal(&lObj, &rObj);
            obj_setb(&lObj, value);

            success = 1;
            break;
        }
        case BOP_NE: {
            int value = !crsV_equal(&lObj, &rObj);
            obj_setb(&lObj, value);

            success = 1;
            break;
        }
    }

    return success ? fold_fromObject(lhs, &lObj) : 0;
}

/*
 * Prepare a boolean operation. Both the left and right hand sides must evaluate
 * to the same register, such a register being the top temporary register. 'lhs'
 * is freed such that code evaluting the right side stores the result in the
 * same register.
 */
static unsigned boolean_prep(Chunk* chunk, Expression* lhs, int test) {
    crsI_toTop(chunk, lhs);
    reg_freeExp(chunk, lhs);
    crsI_iABC(chunk, OP_TEST, lhs->value.v, test, 0);
    return crsI_emit(chunk, 0); /* placeholder */
}

/*
 * Finish a boolean operation. 'rhs' has now been evaluated to the same register
 * as the left hand side.
 */
static void boolean_finish(Chunk* chunk, Expression* rhs, unsigned pc) {
    crsI_toTop(chunk, rhs);
    chunk->func->code[pc] = createJump(chunk, pc, crsI_nextPC(chunk));
}

/* emit code for a unary operation */
void crsI_unary(Chunk* chunk, Expression* exp, int uop) {
    if (fold_unary(exp, uop)) {
        return;
    }

    crsI_flatten(chunk, exp);
    crsI_toAny(chunk, exp);
    reg_freeExp(chunk, exp);

    crs_byte src  = (crs_byte)exp->value.v;
    crs_byte dest = reg_new(chunk);

    switch (uop) {
        case UOP_UNM:
            crsI_iABC(chunk, OP_UNM, dest, src, 0);
            break;
        case UOP_LEN:
            crsI_iABC(chunk, OP_LENGTH, dest, src, 0);
            break;
        case UOP_BNOT:
            crsI_iABC(chunk, OP_BNOT, dest, src, 0);
            break;
        case UOP_NOT:
            crsI_iABC(chunk, OP_NOT, dest, src, 0);
            break;
        default:
            assert(0);
    }

    exp->type    = EXP_TEMP;
    exp->value.v = dest;
}

/* prepare a binary operation */
unsigned crsI_infix(Chunk* chunk, Expression* lhs, int bop) {
    crsI_flatten(chunk, lhs);

    switch (bop) {
        case BOP_AND:
            return boolean_prep(chunk, lhs, 1);
        case BOP_OR:
            return boolean_prep(chunk, lhs, 0);
    }

    return 0;
}

/* emit code for a binary operation */
void crsI_binary(Chunk* chunk, Expression* lhs, Expression* rhs,
                               int bop, unsigned pc) {
    if (fold_binary(lhs, rhs, bop)) {
        return;
    }

    crsI_flatten(chunk, rhs);
    crsI_toAny(chunk, lhs);
    crsI_toAny(chunk, rhs);
    reg_free2Exp(chunk, lhs, rhs);

    crs_byte lReg = (crs_byte)lhs->value.v;
    crs_byte rReg = (crs_byte)rhs->value.v;
    crs_byte dest = reg_new(chunk);
    lhs->type     = EXP_TEMP;
    lhs->value.v  = dest;

    switch (bop) {
        case BOP_ADD:
            crsI_iABC(chunk, OP_ADD, dest, lReg, rReg);
            break;
        case BOP_SUB:
            crsI_iABC(chunk, OP_SUB, dest, lReg, rReg);
            break;
        case BOP_MUL:
            crsI_iABC(chunk, OP_MUL, dest, lReg, rReg);
            break;
        case BOP_DIV:
            crsI_iABC(chunk, OP_DIV, dest, lReg, rReg);
            break;
        case BOP_POW:
            crsI_iABC(chunk, OP_POW, dest, lReg, rReg);
            break;
        case BOP_MOD:
            crsI_iABC(chunk, OP_MOD, dest, lReg, rReg);
            break;
        case BOP_CONCAT:
            crsI_iABC(chunk, OP_CONCAT, dest, lReg, rReg);
            break;
        case BOP_SHL:
            crsI_iABC(chunk, OP_SHL, dest, lReg, rReg);
            break;
        case BOP_SHR:
            crsI_iABC(chunk, OP_SHR, dest, lReg, rReg);
            break;
        case BOP_BAND:
            crsI_iABC(chunk, OP_BAND, dest, lReg, rReg);
            break;
        case BOP_BXOR:
            crsI_iABC(chunk, OP_BXOR, dest, lReg, rReg);
            break;
        case BOP_BOR:
            crsI_iABC(chunk, OP_BOR, dest, lReg, rReg);
            break;
        case BOP_EQ:
            crsI_iABC(chunk, OP_EQ, dest, lReg, rReg);
            break;
        case BOP_NE:
            crsI_iABC(chunk, OP_EQ, dest, lReg, rReg);
            crsI_iABC(chunk, OP_NOT, dest, dest, 0);
            break;
        case BOP_GT:
            crsI_iABC(chunk, OP_LE, dest, lReg, rReg);
            crsI_iABC(chunk, OP_NOT, dest, dest, 0);
            break;
        case BOP_GE:
            crsI_iABC(chunk, OP_LT, dest, lReg, rReg);
            crsI_iABC(chunk, OP_NOT, dest, dest, 0);
            break;
        case BOP_LT:
            crsI_iABC(chunk, OP_LT, dest, lReg, rReg);
            break;
        case BOP_LE:
            crsI_iABC(chunk, OP_LE, dest, lReg, rReg);
            break;
        case BOP_AND:
        case BOP_OR:
            boolean_finish(chunk, rhs, pc);
            break;
        default:
            assert(0);
    }
}

void crsI_getValues(Chunk* chunk, Expression* exp, int count) {
    assert(exp->type == EXP_CALL);
    crs_instr* i = &chunk->func->code[exp->value.v];
    *i          |= instr_setC(count);

    if (count == MAX_REGS) {
        exp->type    = EXP_VLIST;
        exp->value.v = 0;
        /* it is expected that the next instruction handles the VLIST */
    } else {
        exp->type    = EXP_LIST;
        exp->value.v = (crs_byte)count;
        chunk->regs += (crs_byte)count;
    }
}

void crsI_index(Chunk* chunk, Expression* obj, Expression* key) {
    crsI_flatten(chunk, key);
    crsI_toAny(chunk, obj);
    crsI_toAny(chunk, key);

    crs_byte objReg = (crs_byte)obj->value.v;
    crs_byte keyReg = (crs_byte)key->value.v;

    obj->type        = EXP_INDEX;
    obj->value.x.obj = objReg;
    obj->value.x.key = keyReg;
}

void crsI_call(Chunk* chunk, Expression* obj, Expression* args) {
    unsigned argn;

    switch (args->type) {
        case EXP_CALL:
            /* all return values passed as arguments */
            crsI_getValues(chunk, obj, MAX_REGS);
            /* fallthrough */
        case EXP_VLIST:
            /* stack top signals end of list */
            argn = MAX_REGS;
            break;
        case EXP_LIST:
            argn = args->value.v;
            break;
        case EXP_VOID:
            argn = 0;
            break;
        default:
            /* all other expression types evaluate to one value */
            crsI_toTop(chunk, args);
            reg_freeExp(chunk, args);
            argn = 1;
    }

    reg_freeExp(chunk, obj); /* 'obj' no longer exists after the call */
    obj->type    = EXP_CALL;
    obj->value.v = crsI_iABC(chunk, OP_CALL, obj->value.v, argn, 0);
    /* no return values unless required */
}

void crsI_test(Chunk* chunk, Expression* exp, int test) {
    crsI_toAny(chunk, exp);
    crsI_iABC(chunk, OP_TEST, exp->value.v, test, 0);
}

/*
 * ===========================
 *  variables
 * ===========================
 */

void crsI_var(Chunk* chunk, crs_String* name, Expression* exp) {
    Parser*   parser = chunk->parser;
    Variable* vars   = parser->vecs.vars;

    for (unsigned i = parser->vars.count; i < chunk->fV;) {
        Variable* var = &vars[--i];

        if (var->name == name) {
            exp->type    = EXP_LOCAL;
            exp->value.v = var->reg;
        }
    }

    exp->type    = EXP_GLOBAL;
    exp->value.s = name;
}

void crsI_local(Chunk* chunk, crs_String* name) {
    if (chunk->locals == MAX_LOCALS) {
        limitError(chunk, MAX_LOCALS, "local variables");
    }

    Parser*   parser = chunk->parser;
    unsigned  index  = data_check(chunk, &parser->vars, "variables");
    Variable* var    = &parser->vecs.vars[index];

    /* registers aren't allocated yet */
    var->name = name;
    var->reg  = chunk->locals++;
    chunk->scope->nV++;
}

/*
 * ===========================
 *  statements
 * ===========================
 */

void crsI_return(Chunk* chunk, Expression* exp) {
    unsigned reg;
    unsigned count;

    switch (exp->type) {
        case EXP_CALL:
            /* return all return values */
            crsI_getValues(chunk, exp, MAX_REGS);
            /* fallthrough */
        case EXP_VLIST:
            /* stack top signals end of list */
            reg   = 0; /* doesn't matter */
            count = MAX_REGS;
            reg_freeList(chunk, exp);
            break;
        case EXP_LIST:
            reg   = chunk->regs - exp->value.v;
            count = exp->value.v;
            reg_freeList(chunk, exp);
            break;
        case EXP_VOID:
            reg   = 0; /* doesn't matter */
            count = 0;
            break;
        default:
            /* all other expression types evaluate to one value */
            crsI_toAny(chunk, exp);
            reg   = exp->value.v;
            count = 1;
            reg_freeExp(chunk, exp);
    }

    crsI_iABC(chunk, OP_RETURN, reg, count, 0);
}
