/*
 * https://github.com/mochji/crescent
 * core/methods.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include <stddef.h>
#include <math.h>

#include "crescent/conf.h"
#include "limit.h"

#include "types/string.h"
#include "types/table.h"
#include "core/object.h"
#include "core/state.h"
#include "core/call.h"
#include "core/gc.h"

#include "core/methods.h"

/* order MT */
char* crsM_names[MT_COUNT] = {
    "__unm", "__add", "__sub", "__mul", "__div", "__pow", "__mod",
    "__bnot", "__band", "__bor", "__bxor", "__shl", "__shr",
    "__eq", "__lt", "__le", "__gt", "__ge",
    "__len", "__concat",
    "__get", "__set", "__call"
};

void crsM_init(crs_Thread* thread) {
    crs_State* state = thread->state;

    for (int i = 0; i < MT_COUNT; i++) {
        state->mtKeys[i] = crsS_new(thread, crsM_names[i]);
        crsG_setImmune(thread);
    }

    for (int i = 0; i < CRS_TYPECOUNT; i++) {
        state->mt[i] = NULL;
    }
}

/*
 * ===========================
 *  netatables & metamethods
 * ===========================
 */

crs_Table** crsM_getMT(crs_Thread* thread, crs_Object* obj) {
    return obj->type == CRS_TYPE_TABLE
        ? &obj_gett(obj)->mt
        : &thread->state->mt[obj_apitype(obj)];
}

void crsM_setMT(crs_Thread* thread, crs_Object* obj, crs_Table* mt) {
    crs_Table** mtP = crsM_getMT(thread, obj);
    *mtP            = mt;
    crsG_barrierF(thread, obj_toheader(mt));
}

int crsM_getMM(crs_Thread* thread, crs_Object* mm, crs_Object* obj, int op) {
    crs_Table* mt = *crsM_getMT(thread, obj);
    obj_setn(mm);

    if (mt != NULL) {
        crs_Object key;
        obj_setgc(&key, thread->state->mtKeys[op]);
        crsT_get(thread, mt, &key, mm);
    }

    return mm->type != CRS_TYPE_NIL;
}

/*
 * ===========================
 *  raw operations
 * ===========================
 */

static CRS_NORET void error_unary(crs_Thread* thread, crs_Object* r, char* op) {
    crsC_errorf(thread, "attempt to perform unary '%s' on a %s value",
        op, crsO_name(r));
}

static CRS_NORET void error_binary(crs_Thread* thread, crs_Object* l,
                                               crs_Object* r, char* op) {
    crsC_errorf(thread, "attempt to perform '%s' on %s and %s values",
        op, crsO_name(l), crsO_name(r));
}

static void error_int(crs_Thread* thread, crs_Object* l, crs_Object* r) {
    if (obj_isnumber(l) && obj_isnumber(r)) {
        crsC_error(thread, "number has no integer representation");
    }
}

static CRS_NORET void error_op(crs_Thread* thread, crs_Object* o, char* op) {
    crsC_errorf(thread, "attempt to %s a %s value", op, crsO_name(o));
}

static void error_compare(crs_Thread* thread, crs_Object* l, crs_Object* r,
                                              int op) {
    switch (op) {
        case MT_LT:
            error_binary(thread, l, r, "<");
        case MT_LE:
            error_binary(thread, l, r, "<=");
        case MT_GT:
            error_binary(thread, l, r, ">");
        case MT_GE:
            error_binary(thread, l, r, ">=");
    }
}

static void error_arith(crs_Thread* thread, crs_Object* l, crs_Object* r,
                                            int op) {
    switch (op) {
        case MT_UNM:
            error_unary(thread, r, "-");
        case MT_ADD:
            error_binary(thread, l, r, "+");
        case MT_SUB:
            error_binary(thread, l, r, "-");
        case MT_MUL:
            error_binary(thread, l, r, "*");
        case MT_DIV:
            error_binary(thread, l, r, "/");
        case MT_POW:
            error_binary(thread, l, r, "^");
        case MT_MOD:
            error_binary(thread, l, r, "%");
        case MT_BNOT:
            error_int(thread, r, r);
            error_unary(thread, r, "~");
        case MT_BAND:
            error_int(thread, l, r);
            error_binary(thread, l, r, "&");
        case MT_BOR:
            error_int(thread, l, r);
            error_binary(thread, l, r, "|");
        case MT_BXOR:
            error_int(thread, l, r);
            error_binary(thread, l, r, "~");
        case MT_SHL:
            error_int(thread, l, r);
            error_binary(thread, l, r, "<<");
        case MT_SHR:
            error_int(thread, l, r);
            error_binary(thread, l, r, ">>");
    }
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

static int cmp_equal(crs_Object* l, crs_Object* r) {
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

static int cmp_less(crs_Object* l, crs_Object* r, int* result) {
    if (l->type == CRS_TYPE_INTEGER) {
        if (r->type == CRS_TYPE_INTEGER) {
            *result = obj_geti(l) < obj_geti(r);
            return 1;
        } else if (r->type == CRS_TYPE_FLOAT) {
            *result = num_less(l, r);
            return 1;
        }
    } else if (l->type == CRS_TYPE_FLOAT) {
        if (r->type == CRS_TYPE_FLOAT) {
            *result = obj_getf(l) < obj_getf(r);
            return 1;
        } else if (r->type == CRS_TYPE_INTEGER) {
            *result = num_less(l, r);
            return 1;
        }
    }

    return 0;
}

static int cmp_lessEqual(crs_Object* l, crs_Object* r, int* result) {
    if (l->type == CRS_TYPE_INTEGER) {
        if (r->type == CRS_TYPE_INTEGER) {
            *result = obj_geti(l) <= obj_geti(r);
            return 1;
        } else if (r->type == CRS_TYPE_FLOAT) {
            *result = num_lessEqual(l, r);
            return 1;
        }
    } else if (l->type == CRS_TYPE_FLOAT) {
        if (r->type == CRS_TYPE_FLOAT) {
            *result = obj_getf(l) <= obj_getf(r);
            return 1;
        } else if (r->type == CRS_TYPE_INTEGER) {
            *result = num_lessEqual(l, r);
            return 1;
        }
    }

    return 0;
}

static crs_Integer arith_int(crs_Integer l, crs_Integer r, int op) {
    switch (op) {
        case MT_UNM:
            return -r;
        case MT_ADD:
            return l + r;
        case MT_SUB:
            return l - r;
        case MT_MUL:
            return l * r;
        case MT_MOD:
            return r == 0 ? 0 : l % r;
        case MT_BNOT:
            return ~r;
        case MT_BAND:
            return l & r;
        case MT_BOR:
            return l | r;
        case MT_BXOR:
            return l ^ r;
        case MT_SHL:
            return l << r;
        case MT_SHR:
            return l >> r;
        default:
            assert(0);
    }

    return 0;
}

static crs_Float arith_float(crs_Float l, crs_Float r, int op) {
    switch (op) {
        case MT_UNM:
            return -r;
        case MT_ADD:
            return l + r;
        case MT_SUB:
            return l - r;
        case MT_MUL:
            return l * r;
        case MT_DIV:
            return l / r;
        case MT_POW:
            return float_pow(l, r);
        case MT_MOD:
            return r == 0 ? 0 : float_mod(l, r);
        default:
            assert(0);
    }

    return 0;
}

static int raw_length(crs_Object* obj, crs_Integer* result) {
    switch (obj->type) {
        case CRS_TYPE_STRING:
            *result = obj_gets(obj)->length;
            return 1;
        case CRS_TYPE_TABLE:
            *result = obj_gett(obj)->length;
            return 1;
    }

    return 0;
}

static int raw_compare(crs_Object* l, crs_Object* r, int op, int* result) {
    int success = 0;

    switch (op) {
        case MT_EQ:
            *result = cmp_equal(l, r);
            success = 1;
            break;
        case MT_LT:
            return cmp_less(l, r, result);
        case MT_LE:
            return cmp_lessEqual(l, r, result);
        case MT_GT:
            success = cmp_lessEqual(l, r, result);
            *result = !(*result);
            break;
        case MT_GE:
            success = cmp_less(l, r, result);
            *result = !(*result);
            break;
    }

    return success;
}

static int raw_arith(crs_Object* l, crs_Object* r, int op, crs_Object* result) {
    crs_Integer iL, iR;
    crs_Float   fL, fR;

    switch (op) {
        /* integers only */
        case MT_BNOT: case MT_BAND: case MT_BOR:
        case MT_BXOR: case MT_SHL: case MT_SHR:
            if (obj_cvtint(l, &iL) && obj_cvtint(r, &iR)) {
                crs_Integer value = arith_int(iL, iR, op);
                obj_seti(result, value);

                return 1;
            }

            break;
        /* floats only */
        case MT_DIV: case MT_POW:
            if (obj_cvtfloat(l, &fL) && obj_cvtfloat(r, &fR)) {
                crs_Float value = arith_float(fL, fR, op);
                obj_setf(result, value);

                return 1;
            }

            break;
        /* integers and floats */
        case MT_UNM: case MT_ADD: case MT_SUB:
        case MT_MUL: case MT_MOD:
            if (obj_cvtint(l, &iL) && obj_cvtint(r, &iR)) {
                crs_Integer value = arith_int(iL, iR, op);
                obj_seti(result, value);

                return 1;
            } else if (obj_cvtfloat(l, &fL) && obj_cvtfloat(r, &fR)) {
                crs_Float value = arith_float(fL, fR, op);
                obj_setf(result, value);

                return 1;
            }

            break;
    }

    return 0;
}

static int raw_get(crs_Thread* thread, crs_Object* obj, crs_Object* key,
                                       crs_Object* value) {
    if (obj->type == CRS_TYPE_TABLE) {
        crsT_get(thread, obj_gett(obj), key, value);
        return 1;
    }

    return 0;
}

static int raw_set(crs_Thread* thread, crs_Object* obj, crs_Object* key,
                                       crs_Object* value) {
    if (obj->type == CRS_TYPE_TABLE) {
        crsT_set(thread, obj_gett(obj), key, value);
        return 1;
    }

    return 0;
}

/*
 * ===========================
 *  methods
 * ===========================
 */

static int tryUnaryMM(crs_Thread* thread, crs_Object* obj, int op) {
    crs_Object mm;

    if (!crsM_getMM(thread, &mm, obj, op)) {
        return 0;
    }

    obj_seto(thread->stack.top, &mm);
    obj_seto(thread->stack.top + 1, obj);
    thread->stack.top += 2;
    crsM_call(thread, 1, 1);

    return 1;
}

static int tryBinaryMM(crs_Thread* thread, crs_Object* l, crs_Object* r,
                                          int op) {
    crs_Object mm;

    if (!crsM_getMM(thread, &mm, l, op) && !crsM_getMM(thread, &mm, r, op)) {
        return 0;
    }

    obj_seto(thread->stack.top, &mm);
    obj_seto(thread->stack.top + 1, l);
    obj_seto(thread->stack.top + 2, r);
    thread->stack.top += 3;
    crsM_call(thread, 2, 1);

    return 1;
}

static int tryGetMM(crs_Thread* thread, crs_Object* obj, crs_Object* key) {
    crs_Object mm;

    if (!crsM_getMM(thread, &mm, obj, MT_GET)) {
        return 0;
    }

    obj_seto(thread->stack.top, &mm);
    obj_seto(thread->stack.top + 1, obj);
    obj_seto(thread->stack.top + 2, key);
    thread->stack.top += 3;
    crsM_call(thread, 2, 1);

    return 1;
}

static int trySetMM(crs_Thread* thread, crs_Object* obj, crs_Object* key,
                                        crs_Object* value) {
    crs_Object mm;

    if (!crsM_getMM(thread, &mm, obj, MT_SET)) {
        return 0;
    }

    obj_seto(thread->stack.top, &mm);
    obj_seto(thread->stack.top + 1, obj);
    obj_seto(thread->stack.top + 2, key);
    obj_seto(thread->stack.top + 3, value);
    thread->stack.top += 4;
    crsM_call(thread, 3, 0);

    return 1;
}

crs_Integer crsM_length(crs_Thread* thread, crs_Object* obj) {
    crs_Integer result = 0;

    if (thread != NULL && tryUnaryMM(thread, obj, MT_LEN)) {
        thread->stack.top--;
        result = obj_geti(thread->stack.top);
    } else if (!raw_length(obj, &result) && thread != NULL) {
        error_op(thread, obj, "length");
    }

    return result;
}

int crsM_compare(crs_Thread* thread, crs_Object* l, crs_Object* r, int op) {
    int result = 0;

    if (thread != NULL && tryBinaryMM(thread, l, r, op)) {
        thread->stack.top--;
        result = obj_getb(thread->stack.top);
    } else if (!raw_compare(l, r, op, &result) && thread != NULL) {
        error_compare(thread, l, r, op);
    }

    return result;
}

int crsM_rawArith(crs_Object* l, crs_Object* r, int op, crs_Object* result) {
    return raw_arith(l, r, op, result);
}

void crsM_arith(crs_Thread* thread, crs_Object* l, crs_Object* r, int op,
                                    crs_Object* result) {
    if (thread != NULL && tryBinaryMM(thread, l, r, op)) {
        thread->stack.top--;
        obj_seto(result, thread->stack.top);
    } else if (!raw_arith(l, r, op, result) && thread != NULL) {
        error_arith(thread, l, r, op);
    }
}

void crsM_get(crs_Thread* thread, crs_Object* obj, crs_Object* key,
                                  crs_Object* value, int raw) {
    if (!raw && tryGetMM(thread, obj, key)) {
        thread->stack.top--;
        obj_seto(value, thread->stack.top);
    } else if (!raw_get(thread, obj, key, value)) {
        error_op(thread, obj, "index");
    }
}

void crsM_set(crs_Thread* thread, crs_Object* obj, crs_Object* key,
                                  crs_Object* value, int raw) {
    if (!raw && trySetMM(thread, obj, key, value)) {
        return;
    } else if (!raw_set(thread, obj, key, value)) {
        error_op(thread, obj, "index");
    }
}

typedef struct {
    int args;
    int wanted;
} PCallInfo;

static void* pcall(crs_Thread* thread, void* data) {
    PCallInfo* info = data;
    crsM_call(thread, info->args, info->wanted);

    return NULL;
}

void crsM_call(crs_Thread* thread, int args, int wanted) {
    crs_Object* obj = thread->stack.top - args - 1;

    switch (obj->type) {
        case CRS_TYPE_FUNCTION:
            crsC_call(thread, obj_getk(obj), args, wanted);
            return;
        case CRS_TYPE_CFUNCTION:
            crsC_callC(thread, obj_getc(obj), args, wanted);
            return;
    }

    error_op(thread, obj, "call");
}

int crsM_pcall(crs_Thread* thread, int args, int wanted) {
    PCallInfo info = {
        .args   = args,
        .wanted = wanted
    };

    short level  = thread->stack.level;
    int   status = crsC_try(thread, pcall, &info, NULL);

    if (status != CRS_OK) {
        crsC_unwind(thread, level, args);
    }

    return status;
}
