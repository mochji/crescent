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
#include "core/call.h"

#include "core/methods.h"

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

int crsM_equal(crs_Object* l, crs_Object* r) {
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

int crsM_compare(crs_Thread* thread, crs_Object* l, crs_Object* r, int op) {
    int   success = 0, result;
    char* str     = "";

    switch (op) {
        case CRS_OP_LT:
            success = cmp_less(l, r, &result);
            str     = "<";
            break;
        case CRS_OP_LE:
            success = cmp_lessEqual(l, r, &result);
            str     = "<=";
            break;
        case CRS_OP_GT:
            success = cmp_lessEqual(l, r, &result);
            result  = !result;
            str     = ">";
            break;
        case CRS_OP_GE:
            success = cmp_less(l, r, &result);
            result  = !result;
            str     = ">=";
            break;
        default:
            assert(0);
    }

    if (!success) {
        error_binary(thread, l, r, str);
    }

    return result;
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

int crsM_rawArith(crs_Object* o, crs_Object* l, crs_Object* r, int op) {
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

void crsM_arith(crs_Thread* thread, crs_Object* o, crs_Object* l,
                                    crs_Object* r, int op) {
    if (crsM_rawArith(o, l, r, op)) {
        return;
    }

    error_arith(thread, l, r, op);
}

crs_Integer crsM_length(crs_Thread* thread, crs_Object* object) {
    switch (object->type) {
        case CRS_TYPE_STRING:
            return obj_gets(object)->length;
        case CRS_TYPE_TABLE:
            return obj_gett(object)->length;
    }

    error_op(thread, object, "get length of");
}

crs_Object* crsM_get(crs_Thread* thread, crs_Object* object, crs_Object* key) {
    if (object->type == CRS_TYPE_TABLE) {
        return crsT_get(thread, obj_gett(object), key);
    }

    error_op(thread, object, "index");
}

void crsM_set(crs_Thread* thread, crs_Object* object, crs_Object* key,
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
    crsM_call(thread, info->object, info->args, info->wanted);

    return NULL;
}

void crsM_call(crs_Thread* thread, crs_Object* object, int args, int wanted) {
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

int crsM_pcall(crs_Thread* thread, crs_Object* object, int args, int wanted) {
    PCallInfo info = {
        .object = object,
        .args   = args,
        .wanted = wanted
    };

    short level  = thread->stack.level;
    int   status = crsC_try(thread, pcall, &info, NULL);

    if (status != CRS_OK) {
        crsC_unwind(thread, level);
    }

    return status;
}
