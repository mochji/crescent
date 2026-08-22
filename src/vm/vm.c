/*
 * https://github.com/mochji/crescent
 * vm/vm.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <limits.h>
#include <math.h>

#include "crescent/conf.h"
#include "limit.h"

#include "types/string.h"
#include "types/table.h"
#include "core/object.h"
#include "core/state.h"
#include "core/format.h"
#include "core/call.h"
#include "core/gc.h"
#include "vm/opcodes.h"

#include "vm/vm.h"

static noret error_unary(crs_Thread* thread, crs_Object* r, char* op) {
    crsC_errorf(thread, "attempt to perform unary '%s' on a %s value",
        op, crsO_name(r));
}

static noret error_binary(crs_Thread* thread, crs_Object* l, crs_Object* r,
                                              char* op) {
    crsC_errorf(thread, "attempt to perform '%s' on %s and %s values",
        op, crsO_name(l), crsO_name(r));
}

static void error_int(crs_Thread* thread, crs_Object* l, crs_Object* r) {
    if (obj_isnumber(l) && obj_isnumber(r)) {
        crsC_error(thread, "number has no integer representation");
    }
}

static noret error_op(crs_Thread* thread, crs_Object* o, char* op) {
    crsC_errorf(thread, "attempt to %s a %s value", op, crsO_name(o));
}

static int num_equal(crs_Object* l, crs_Object* r) {
    crs_Float fL, fR;
    crsO_toFloat(l, &fL, 0);
    crsO_toFloat(r, &fR, 0);

    return fL == fR;
}

static int num_less(crs_Object* l, crs_Object* r) {
    crs_Float fL, fR;
    crsO_toFloat(l, &fL, 0);
    crsO_toFloat(r, &fR, 0);

    return fL < fR;
}

static int num_lessEqual(crs_Object* l, crs_Object* r) {
    crs_Float fL, fR;
    crsO_toFloat(l, &fL, 0);
    crsO_toFloat(r, &fR, 0);

    return fL <= fR;
}

int crsV_equal(crs_Object* l, crs_Object* r) {
    if (l->type != r->type) {
        if (obj_isnumber(l) && obj_isnumber(r)) {
            return num_equal(l, r);
        }

        return 0;
    }

    switch (l->type) {
        case CRS_TYPE_NIL:
            return 1;
        case CRS_TYPE_BOOLEAN:
            return obj_getb(l) == obj_getb(r);
        case CRS_TYPE_INTEGER:
            return obj_geti(l) == obj_geti(r);
        case CRS_TYPE_FLOAT:
            return obj_getf(l) == obj_getf(r);
        case CRS_TYPE_CFUNCTION:
            return obj_getc(l) == obj_getc(r);
        case CRS_TYPE_STRING:
            return crsS_equal(obj_gets(l), obj_gets(r));
        case CRS_TYPE_TABLE:
            return obj_gett(l) == obj_gett(r);
        case CRS_TYPE_FUNCTION:
            return obj_getk(l) == obj_getk(r);
        case CRS_TYPE_THREAD:
            return obj_getx(l) == obj_getx(r);
        default:
            assert(0);
    }

    return 0;
}

int crsV_less(crs_Thread* thread, crs_Object* l, crs_Object* r) {
    if (l->type == CRS_TYPE_INTEGER) {
        if (r->type == CRS_TYPE_INTEGER) {
            return obj_geti(l) < obj_geti(r);
        } else if (r->type == CRS_TYPE_FLOAT) {
            return num_less(l, r);
        }
    } else if (l->type == CRS_TYPE_FLOAT) {
        if (r->type == CRS_TYPE_FLOAT) {
            return obj_getf(l) < obj_getf(r);
        } else if (r->type == CRS_TYPE_INTEGER) {
            return num_less(l, r);
        }
    }

    error_binary(thread, l, r, "<");
}

int crsV_lessEqual(crs_Thread* thread, crs_Object* l, crs_Object* r) {
    if (l->type == CRS_TYPE_INTEGER) {
        if (r->type == CRS_TYPE_INTEGER) {
            return obj_geti(l) <= obj_geti(r);
        } else if (r->type == CRS_TYPE_FLOAT) {
            return num_lessEqual(l, r);
        }
    } else if (l->type == CRS_TYPE_FLOAT) {
        if (r->type == CRS_TYPE_FLOAT) {
            return obj_getf(l) <= obj_getf(r);
        } else if (r->type == CRS_TYPE_INTEGER) {
            return num_lessEqual(l, r);
        }
    }

    error_binary(thread, l, r, "<=");
}

#if CRS_FLOAT_TYPE == CRS_FLOAT_FLOAT
#define float_pow(l, r) powf(l, r)
#define float_mod(l, r) fmodf(l, r)
#elif CRS_FLOAT_TYPE == CRS_FLOAT_DOUBLE
#define float_pow(l, r) pow(l, r)
#define float_mod(l, r) fmod(l, r)
#elif CRS_FLOAT_TYPE == CRS_FLOAT_LDOUBLE
#define float_pow(l, r) powl(l, r)
#define float_mod(l, r) fmodl(l, r)
#endif

crs_Integer crsV_length(crs_Thread* thread, crs_Object* object) {
    if (object->type == CRS_TYPE_STRING) {
        return obj_gets(object)->length;
    }

    error_op(thread, object, "get length of");
}

static void error_arith(crs_Thread* thread, crs_Object* l, crs_Object* r,
                                            int op) {
    switch (op) {
        case CRS_OP_UNM:
            error_unary(thread, r, "-");
        case CRS_OP_ADD:
            error_binary(thread, l, r, "+");
        case CRS_OP_SUB:
            error_binary(thread, l, r, "-");
        case CRS_OP_MUL:
            error_binary(thread, l, r, "*");
        case CRS_OP_DIV:
            error_binary(thread, l, r, "/");
        case CRS_OP_POW:
            error_binary(thread, l, r, "^");
        case CRS_OP_MOD:
            error_binary(thread, l, r, "%");
        case CRS_OP_BNOT:
            error_int(thread, r, r);
            error_unary(thread, r, "~");
        case CRS_OP_BAND:
            error_int(thread, l, r);
            error_binary(thread, l, r, "&");
        case CRS_OP_BOR:
            error_int(thread, l, r);
            error_binary(thread, l, r, "|");
        case CRS_OP_BXOR:
            error_int(thread, l, r);
            error_binary(thread, l, r, "~");
        case CRS_OP_SHL:
            error_int(thread, l, r);
            error_binary(thread, l, r, "<<");
        case CRS_OP_SHR:
            error_int(thread, l, r);
            error_binary(thread, l, r, ">>");
    }
}

static crs_Integer arith_int(crs_Integer l, crs_Integer r, int op) {
    switch (op) {
        case CRS_OP_UNM:
            return -r;
        case CRS_OP_ADD:
            return l + r;
        case CRS_OP_SUB:
            return l - r;
        case CRS_OP_MUL:
            return l * r;
        case CRS_OP_MOD:
            return r == 0 ? 0 : l % r;
        case CRS_OP_BNOT:
            return ~r;
        case CRS_OP_BAND:
            return l & r;
        case CRS_OP_BOR:
            return l | r;
        case CRS_OP_BXOR:
            return l ^ r;
        case CRS_OP_SHL:
            return l << r;
        case CRS_OP_SHR:
            return l >> r;
        default:
            assert(0);
    }

    return 0;
}

static crs_Float arith_float(crs_Float l, crs_Float r, int op) {
    switch (op) {
        case CRS_OP_UNM:
            return -r;
        case CRS_OP_ADD:
            return l + r;
        case CRS_OP_SUB:
            return l - r;
        case CRS_OP_MUL:
            return l * r;
        case CRS_OP_DIV:
            return l / r;
        case CRS_OP_POW:
            return float_pow(l, r);
        case CRS_OP_MOD:
            return r == 0 ? 0 : float_mod(l, r);
        default:
            assert(0);
    }

    return 0;
}

int crsV_rawArith(crs_Object* o, crs_Object* l, crs_Object* r, int op) {
    crs_Integer iL, iR;
    crs_Float   fL, fR;

    switch (op) {
        /* integers only */
        case CRS_OP_BNOT: case CRS_OP_BAND: case CRS_OP_BOR:
        case CRS_OP_BXOR: case CRS_OP_SHL: case CRS_OP_SHR:
            if (obj_cvtint(l, &iL) && obj_cvtint(r, &iR)) {
                crs_Integer result = arith_int(iL, iR, op);
                obj_seti(o, result);

                return 1;
            }

            break;
        /* floats only */
        case CRS_OP_DIV: case CRS_OP_POW:
            if (obj_cvtfloat(l, &fL) && obj_cvtfloat(r, &fR)) {
                crs_Float result = arith_float(fL, fR, op);
                obj_setf(o, result);

                return 1;
            }

            break;
        /* integers and floats */
        case CRS_OP_UNM: case CRS_OP_ADD: case CRS_OP_SUB:
        case CRS_OP_MUL: case CRS_OP_MOD:
            if (obj_cvtint(l, &iL) && obj_cvtint(r, &iR)) {
                crs_Integer result = arith_int(iL, iR, op);
                obj_seti(o, result);

                return 1;
            } else if (obj_cvtfloat(l, &fL) && obj_cvtfloat(r, &fR)) {
                crs_Float result = arith_float(fL, fR, op);
                obj_setf(o, result);

                return 1;
            }

            break;
    }

    return 0;
}

void crsV_arith(crs_Thread* thread, crs_Object* o, crs_Object* l,
                                    crs_Object* r, int op) {
    if (crsV_rawArith(o, l, r, op)) {
        return;
    }

    error_arith(thread, l, r, op);
}

crs_Object* crsV_get(crs_Thread* thread, crs_Object* object, crs_Object* key) {
    if (object->type == CRS_TYPE_TABLE) {
        return crsT_get(thread, obj_gett(object), key);
    }

    error_op(thread, object, "index");
}

void crsV_set(crs_Thread* thread, crs_Object* object, crs_Object* key,
                                  crs_Object* value) {
    if (object->type == CRS_TYPE_TABLE) {
        crsT_set(thread, obj_gett(object), key, value);
        return;
    }

    error_op(thread, object, "index");
}

typedef struct {
    crs_Object* object;
    int         args;
    int         wanted;
} PCallInfo;

static void* pcall(crs_Thread* thread, void* data) {
    PCallInfo* info = data;
    crsV_call(thread, info->object, info->args, info->wanted);

    return NULL;
}

void crsV_call(crs_Thread* thread, crs_Object* object, int args, int wanted) {
    switch (object->type) {
        case CRS_TYPE_FUNCTION:
            crsC_call(thread, obj_getk(object), args, wanted);
            return;
        case CRS_TYPE_CFUNCTION:
            crsC_callC(thread, obj_getc(object), args, wanted);
            return;
    }

    error_op(thread, object, "call");
}

int crsV_pcall(crs_Thread* thread, crs_Object* object, int args, int wanted) {
    PCallInfo info = {
        .object = object,
        .args   = args,
        .wanted = wanted
    };

    short level  = thread->stack.level;
    int   status = crsC_try(thread, pcall, &info, NULL);

    if (status != CRS_STATUS_OK) {
        crsC_restoreStack(thread, level);
    }

    return status;
}

#define reg_A(i) (stack + instr_A(i))
#define reg_B(i) (stack + instr_B(i))
#define reg_C(i) (stack + instr_C(i))

static void checkVList(crs_Thread* thread, int values) {
    if (values > CRS_MAX_TOP) {
        crsC_error(thread, "stack overflow");
    }
}

int crsV_execute(crs_Thread* thread, crs_Function* func) {
    crs_Frame*  frame = thread->stack.frame;
    crs_Object* stack = frame->base;
    crs_instr*  pc    = func->code;

    for (;;) {
        crs_instr i = *pc++;

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
                crsV_arith(thread, reg_A(i), b, b, CRS_OP_UNM);
                break;
            }
            case OP_ADD: {
                crsV_arith(thread, reg_A(i), reg_B(i), reg_C(i), CRS_OP_ADD);
                break;
            }
            case OP_SUB: {
                crsV_arith(thread, reg_A(i), reg_B(i), reg_C(i), CRS_OP_SUB);
                break;
            }
            case OP_MUL: {
                crsV_arith(thread, reg_A(i), reg_B(i), reg_C(i), CRS_OP_MUL);
                break;
            }
            case OP_DIV: {
                crsV_arith(thread, reg_A(i), reg_B(i), reg_C(i), CRS_OP_DIV);
                break;
            }
            case OP_POW: {
                crsV_arith(thread, reg_A(i), reg_B(i), reg_C(i), CRS_OP_POW);
                break;
            }
            case OP_MOD: {
                crsV_arith(thread, reg_A(i), reg_B(i), reg_C(i), CRS_OP_MOD);
                break;
            }
            case OP_BNOT: {
                crsV_arith(thread, reg_A(i), reg_B(i), reg_C(i), CRS_OP_BNOT);
                break;
            }
            case OP_BAND: {
                crsV_arith(thread, reg_A(i), reg_B(i), reg_C(i), CRS_OP_BAND);
                break;
            }
            case OP_BOR: {
                crsV_arith(thread, reg_A(i), reg_B(i), reg_C(i), CRS_OP_BOR);
                break;
            }
            case OP_BXOR: {
                crsV_arith(thread, reg_A(i), reg_B(i), reg_C(i), CRS_OP_BXOR);
                break;
            }
            case OP_SHL: {
                crsV_arith(thread, reg_A(i), reg_B(i), reg_C(i), CRS_OP_SHL);
                break;
            }
            case OP_SHR: {
                crsV_arith(thread, reg_A(i), reg_B(i), reg_C(i), CRS_OP_SHR);
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
                int         v = crsV_equal(reg_B(i), reg_C(i));
                obj_setb(a, v);

                break;
            }
            case OP_LT: {
                crs_Object* a = reg_A(i);
                int         v = crsV_less(thread, reg_B(i), reg_C(i));
                obj_setb(a, v);

                break;
            }
            case OP_LE: {
                crs_Object* a = reg_A(i);
                int         v = crsV_lessEqual(thread, reg_B(i), reg_C(i));
                obj_setb(a, v);

                break;
            }
            case OP_LENGTH: {
                crs_Object* a = reg_A(i);
                crs_Integer v = crsV_length(thread, reg_B(i));
                obj_seti(a, v);

                break;
            }
            case OP_CONCAT: { /* TODO */
                break;
            }
            case OP_GET: {
                crs_Object* a = reg_A(i);
                crs_Object* v = crsV_get(thread, reg_B(i), reg_C(i));
                obj_seto(a, v);

                break;
            }
            case OP_SET: {
                crsV_set(thread, reg_B(i), reg_C(i), reg_A(i));
                break;
            }
            case OP_CALL: {
                crs_Object* a      = reg_A(i);
                int         args   = (int)instr_B(i);
                int         wanted = (int)instr_C(i);

                if (args == MAX_REGS) {
                    args = (int)(thread->stack.top - (a + 1));
                    checkVList(thread, args);
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

                crsV_call(thread, &object, args, wanted);
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
                    checkVList(thread, count);
                    /* a cfunction can return up to CRS_MAX_TOP values */
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
