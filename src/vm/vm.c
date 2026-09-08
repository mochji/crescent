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

#define reg_A(i) (stack + instr_A(i))
#define reg_B(i) (stack + instr_B(i))
#define reg_C(i) (stack + instr_C(i))

int crsV_execute(crs_Thread* thread, crs_Function* func) {
    crs_Frame*  frame = thread->stack.frame;
    crs_Object* stack = frame->base;
    crs_instr*  pc    = func->code;

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
                crs_Object* v = crsT_get(thread,
                    obj_gett(&thread->state->globals), b);
                obj_seto(a, v);

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
                crs_Object* b = reg_B(i);
                crsM_arith(thread, reg_A(i), b, b, CRS_OP_UNM);
                break;
            }
            case OP_ADD: {
                crsM_arith(thread, reg_A(i), reg_B(i), reg_C(i), CRS_OP_ADD);
                break;
            }
            case OP_SUB: {
                crsM_arith(thread, reg_A(i), reg_B(i), reg_C(i), CRS_OP_SUB);
                break;
            }
            case OP_MUL: {
                crsM_arith(thread, reg_A(i), reg_B(i), reg_C(i), CRS_OP_MUL);
                break;
            }
            case OP_DIV: {
                crsM_arith(thread, reg_A(i), reg_B(i), reg_C(i), CRS_OP_DIV);
                break;
            }
            case OP_POW: {
                crsM_arith(thread, reg_A(i), reg_B(i), reg_C(i), CRS_OP_POW);
                break;
            }
            case OP_MOD: {
                crsM_arith(thread, reg_A(i), reg_B(i), reg_C(i), CRS_OP_MOD);
                break;
            }
            case OP_BNOT: {
                crsM_arith(thread, reg_A(i), reg_B(i), reg_C(i), CRS_OP_BNOT);
                break;
            }
            case OP_BAND: {
                crsM_arith(thread, reg_A(i), reg_B(i), reg_C(i), CRS_OP_BAND);
                break;
            }
            case OP_BOR: {
                crsM_arith(thread, reg_A(i), reg_B(i), reg_C(i), CRS_OP_BOR);
                break;
            }
            case OP_BXOR: {
                crsM_arith(thread, reg_A(i), reg_B(i), reg_C(i), CRS_OP_BXOR);
                break;
            }
            case OP_SHL: {
                crsM_arith(thread, reg_A(i), reg_B(i), reg_C(i), CRS_OP_SHL);
                break;
            }
            case OP_SHR: {
                crsM_arith(thread, reg_A(i), reg_B(i), reg_C(i), CRS_OP_SHR);
                break;
            }
            case OP_NOT: {
                crs_Object* a = reg_A(i);
                int         v = !crsO_test(reg_B(i));
                obj_setb(a, v);

                break;
            }
            case OP_EQ: {
                crs_Object* a = reg_A(i);
                int         v = crsM_equal(reg_B(i), reg_C(i));
                obj_setb(a, v);

                break;
            }
            case OP_LT: {
                crs_Object* a = reg_A(i);
                int         v = crsM_compare(thread,
                    reg_B(i), reg_C(i), CRS_OP_LT);
                obj_setb(a, v);

                break;
            }
            case OP_LE: {
                crs_Object* a = reg_A(i);
                int         v = crsM_compare(thread,
                    reg_B(i), reg_C(i), CRS_OP_LE);
                obj_setb(a, v);

                break;
            }
            case OP_GT: {
                crs_Object* a = reg_A(i);
                int         v = crsM_compare(thread,
                    reg_B(i), reg_C(i), CRS_OP_GT);
                obj_setb(a, v);

                break;
            }
            case OP_GE: {
                crs_Object* a = reg_A(i);
                int         v = crsM_compare(thread,
                    reg_B(i), reg_C(i), CRS_OP_GE);
                obj_setb(a, v);

                break;
            }
            case OP_LENGTH: {
                crs_Object* a = reg_A(i);
                crs_Integer v = crsM_length(thread, reg_B(i));
                obj_seti(a, v);

                break;
            }
            case OP_CONCAT: { /* TODO */
                break;
            }
            case OP_GET: {
                crs_Object* a = reg_A(i);
                crs_Object* v = crsM_get(thread, reg_B(i), reg_C(i));
                obj_seto(a, v);

                break;
            }
            case OP_SET: {
                crsM_set(thread, reg_B(i), reg_C(i), reg_A(i));
                break;
            }
            case OP_CALL: {
                crs_Object* a      = reg_A(i);
                int         args   = (int)instr_B(i);
                int         wanted = (int)instr_C(i);

                if (args == MAX_REGS) {
                    args = (int)(thread->stack.top - (a + 1));
                } else {
                    thread->stack.top = a + args + 1;
                }

                if (wanted == MAX_REGS) {
                    wanted = CRS_RETALL;
                }

                /* move arguments down, replacing R[A] */
                crs_Object object;
                obj_seto(&object, a);
                memmove(a, a + 1, (size_t)args * sizeof(crs_Object));
                thread->stack.top--;

                crsM_call(thread, &object, args, wanted);
                stack = frame->base;

                if (wanted != CRS_RETALL) {
                    thread->stack.top = stack + func->top;
                } /* otherwise, top signals end of list for next instruction */

                break;
            }
            case OP_RETURN: {
                crs_Object* a     = reg_A(i);
                int         count = (int)instr_B(i);

                if (count == MAX_REGS) {
                    count = (int)(thread->stack.top - a);
                } else if (count) {
                    thread->stack.top = a + count;
                }

                return count;
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
