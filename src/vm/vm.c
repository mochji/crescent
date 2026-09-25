/*
 * https://github.com/mochji/crescent
 * vm/vm.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include <stddef.h>
#include <string.h>

#include "crescent/conf.h"
#include "limit.h"

#include "types/string.h"
#include "types/table.h"
#include "core/object.h"
#include "core/methods.h"
#include "core/state.h"
#include "core/format.h"
#include "core/call.h"
#include "core/gc.h"
#include "vm/opcodes.h"

#include "vm/vm.h"

/* metamethods and function calls may resize the stack */
#define reg_A(i) (*stack + instr_A(i))
#define reg_B(i) (*stack + instr_B(i))
#define reg_C(i) (*stack + instr_C(i))

static void op_binary(crs_Thread* thread, crs_instr i, int op) {
    crs_Object** stack = &thread->stack.frame->base;
    crs_Object   result;
    crsM_arith(thread, reg_B(i), reg_C(i), op, &result);

    crs_Object* a = reg_A(i);
    obj_seto(a, &result);
}

static void op_unary(crs_Thread* thread, crs_instr i, int op) {
    crs_Object** stack = &thread->stack.frame->base;
    crs_Object*  b     = reg_B(i);
    crs_Object   result;
    crsM_arith(thread, b, b, op, &result);

    crs_Object* a = reg_A(i);
    obj_seto(a, &result);
}

static void op_compare(crs_Thread* thread, crs_instr i, int op) {
    crs_Object** stack  = &thread->stack.frame->base;
    int          result = crsM_compare(thread, reg_B(i), reg_C(i), op);
    crs_Object*  a      = reg_A(i);
    obj_setb(a, result);
}

static void op_length(crs_Thread* thread, crs_instr i) {
    crs_Object** stack  = &thread->stack.frame->base;
    crs_Integer  length = crsM_length(thread, reg_B(i));
    crs_Object*  a      = reg_A(i);
    obj_seti(a, length);
}

static void op_concat(crs_Thread* thread, crs_instr i) {
    crs_Object** stack = &thread->stack.frame->base;
    crs_Object   result;
    crsM_concat(thread, reg_B(i), reg_C(i), &result);

    crs_Object* a = reg_A(i);
    obj_seto(a, &result);
}

static void op_get(crs_Thread* thread, crs_instr i) {
    crs_Object** stack = &thread->stack.frame->base;
    crs_Object   result;
    crsM_get(thread, reg_B(i), reg_C(i), &result, 0);

    crs_Object* a = reg_A(i);
    obj_seto(a, &result);
}

static void op_call(crs_Thread* thread, crs_instr i, crs_Function* func) {
    crs_Frame*   frame  = thread->stack.frame;
    crs_Object** stack  = &frame->base;
    crs_Object*  a      = reg_A(i);
    int          args   = (int)instr_B(i);
    int          wanted = (int)instr_C(i);

    if (args == MAX_REGS) {
        args = (int)(thread->stack.top - (a + 1));
    } else {
        thread->stack.top = a + args + 1;
    }

    if (wanted == MAX_REGS) {
        wanted = CRS_RETALL;
    }

    crsM_call(thread, args, wanted, 0);

    if (wanted != CRS_RETALL) {
        thread->stack.top = *stack + func->top;
    } /* otherwise, top signals end of list for next instruction */
}

static int op_return(crs_Thread* thread, crs_instr i) {
    crs_Object** stack  = &thread->stack.frame->base;
    crs_Object*  a      = reg_A(i);
    int          count  = (int)instr_B(i);

    if (count == MAX_REGS) {
        count = (int)(thread->stack.top - a);
    } else if (count) {
        thread->stack.top = a + count;
    }

    return count;
}

int crsV_execute(crs_Thread* thread, crs_Function* func) {
    crs_Frame*   frame = thread->stack.frame;
    crs_Object** stack = &frame->base;
    crs_instr*   pc    = func->code;

    for (;;) {
        frame->i.v.pc = pc;
        crs_instr i   = *pc++;

        switch (instr_opcode(i)) {
            case OP_MOV: {
                crs_Object* a = reg_A(i);
                crs_Object* b = reg_B(i);
                obj_seto(a, b);

                break;
            }
            case OP_GETG: {
                crs_Object* a = reg_A(i);
                crs_Object* b = &func->consts[instr_Bx(i)];
                crsT_get(thread, obj_gett(&thread->state->globals), b, a);

                break;
            }
            case OP_SETG: {
                crs_Object* a = reg_A(i);
                crs_Object* b = &func->consts[instr_Bx(i)];
                crsT_set(thread, obj_gett(&thread->state->globals), b, a);

                break;
            }
            case OP_LODN: {
                crs_Object* a = reg_A(i);
                obj_setn(a);

                break;
            }
            case OP_LODT: {
                crs_Object* a = reg_A(i);
                obj_setb(a, 1);

                break;
            }
            case OP_LODF: {
                crs_Object* a = reg_A(i);
                obj_setb(a, 0);

                break;
            }
            case OP_LODI: {
                crs_Object* a = reg_A(i);
                crs_Integer b = instr_sBx(i);
                obj_seti(a, b);

                break;
            }
            case OP_LODC: {
                crs_Object* a = reg_A(i);
                crs_Object* b = &func->consts[instr_Bx(i)];
                obj_seto(a, b);

                break;
            }
            case OP_LODK: {
                crs_Object*   a = reg_A(i);
                crs_Function* b = func->nested[instr_Bx(i)];
                obj_setgc(a, b);

                break;
            }
            case OP_NEWT: {
                crs_Object* a = reg_A(i);
                crs_Table*  v = crsT_new(thread);
                obj_setgc(a, v);

                break;
            }
            case OP_UNM: {
                op_unary(thread, i, MT_UNM);
                break;
            }
            case OP_ADD: {
                op_binary(thread, i, MT_ADD);
                break;
            }
            case OP_SUB: {
                op_binary(thread, i, MT_SUB);
                break;
            }
            case OP_MUL: {
                op_binary(thread, i, MT_MUL);
                break;
            }
            case OP_DIV: {
                op_binary(thread, i, MT_DIV);
                break;
            }
            case OP_POW: {
                op_binary(thread, i, MT_POW);
                break;
            }
            case OP_MOD: {
                op_binary(thread, i, MT_MOD);
                break;
            }
            case OP_BNOT: {
                op_binary(thread, i, MT_BNOT);
                break;
            }
            case OP_BAND: {
                op_binary(thread, i, MT_BAND);
                break;
            }
            case OP_BOR: {
                op_binary(thread, i, MT_BOR);
                break;
            }
            case OP_BXOR: {
                op_binary(thread, i, MT_BXOR);
                break;
            }
            case OP_SHL: {
                op_binary(thread, i, MT_SHL);
                break;
            }
            case OP_SHR: {
                op_binary(thread, i, MT_SHR);
                break;
            }
            case OP_NOT: {
                crs_Object* a = reg_A(i);
                int         v = !crsO_test(reg_B(i));
                obj_setb(a, v);

                break;
            }
            case OP_EQ: {
                op_compare(thread, i, MT_EQ);
                break;
            }
            case OP_LT: {
                op_compare(thread, i, MT_LT);
                break;
            }
            case OP_LE: {
                op_compare(thread, i, MT_LE);
                break;
            }
            case OP_GT: {
                op_compare(thread, i, MT_GT);
                break;
            }
            case OP_GE: {
                op_compare(thread, i, MT_GE);
                break;
            }
            case OP_LENGTH: {
                op_length(thread, i);
                break;
            }
            case OP_CONCAT: {
                op_concat(thread, i);
                break;
            }
            case OP_GET: {
                op_get(thread, i);
                break;
            }
            case OP_SET: {
                crsM_set(thread, reg_B(i), reg_C(i), reg_A(i), 0);
                break;
            }
            case OP_GETMT: {
                crs_Object mt;
                crsM_getMT(thread, reg_B(i), &mt);
                crsM_get(thread, &mt, reg_C(i), &mt, 0);
                crs_Object* a = reg_A(i);
                obj_seto(a, &mt);

                break;
            }
            case OP_SETMT: {
                crs_Object mt;
                crsM_getMT(thread, reg_B(i), &mt);
                crsM_set(thread, &mt, reg_C(i), reg_A(i), 0);

                break;
            }
            case OP_CALL: {
                op_call(thread, i, func);
                break;
            }
            case OP_METHOD: {
                crs_Object* a   = reg_A(i);
                crs_Object* obj = reg_B(i);
                crs_Object  mt;
                crs_Object  temp;

                obj_seto(&temp, a);     /* T = R[A] */
                crsM_getMT(thread, obj, &mt);
                crsM_get(thread, &mt, reg_C(i), &mt, 0);
                obj_seto(a, &mt);       /* R[A]     = R[B]:R[C] */
                obj_seto(a + 1, &temp); /* R[A + 1] = T */

                break;
            }
            case OP_RETURN: {
                return op_return(thread, i);
            }
            case OP_TEST: {
                pc += crsO_test(reg_A(i)) == (int)instr_B(i);
                break;
            }
            case OP_JMP: {
                pc += instr_sAxx(i);
                pc--;
                break;
            }
            default:
                assert(0);
        }

        crsG_check(thread);
    }

    return 0;
}
