/*
 * https://github.com/mochji/crescent
 * vm/vm.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include <stdlib.h>
#include <stddef.h>
#include <limits.h>
#include <math.h>

#include "conf.h"
#include "limit.h"

#include "types/string.h"
#include "types/table.h"
#include "core/object.h"
#include "core/state.h"
#include "core/call.h"

#include "vm/vm.h"

static void __attribute__((noreturn))
error_unary(crs_Thread* thread, crs_Object* r, char* op) {
	crsC_errorf(thread, "attempt to perform unary '%s' on a %s",
		op, crsO_name(r));
}

static void __attribute__((noreturn))
error_binary(crs_Thread* thread, crs_Object* l, crs_Object* r, char* op) {
	crsC_errorf(thread, "attempt to perform '%s' on a %s and %s",
		op, crsO_name(l), crsO_name(r));
}

static void
error_int(crs_Thread* thread, crs_Object* l, crs_Object* r) {
	if (obj_isnumber(l) && obj_isnumber(r)) {
		crsC_error(thread, "number has no integer representation");
	}
}

static void __attribute__((noreturn))
error_op(crs_Thread* thread, crs_Object* o, char* op) {
	crsC_errorf(thread, "attempt to %s a %s", op, crsO_name(o));
}

/*
 * ===========================
 *  comparisons
 * ===========================
 */

static int
num_equal(crs_Object* l, crs_Object* r) {
	crs_Float fL, fR;
	crsO_toFloat(l, &fL);
	crsO_toFloat(r, &fR);

	return fL == fR;
}

static int
num_less(crs_Object* l, crs_Object* r) {
	crs_Float fL, fR;
	crsO_toFloat(l, &fL);
	crsO_toFloat(r, &fR);

	return fL < fR;
}

static int
num_lessEqual(crs_Object* l, crs_Object* r) {
	crs_Float fL, fR;
	crsO_toFloat(l, &fL);
	crsO_toFloat(r, &fR);

	return fL <= fR;
}

int
crsV_equal(crs_Thread* thread, crs_Object* l, crs_Object* r) {
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
		case CRS_TYPE_THREAD:
			return obj_getx(l) == obj_getx(r);
	}

	(void)thread;

	return 0;
}

int
crsV_less(crs_Thread* thread, crs_Object* l, crs_Object* r) {
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

int
crsV_lessEqual(crs_Thread* thread, crs_Object* l, crs_Object* r) {
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

/*
 * ===========================
 *  operations
 * ===========================
 */

#if CRS_FLOAT_TYPE == CRS_FLOAT_FLOAT
#	define float_pow(l, r) powf(l, r)
#	define float_mod(l, r) fmodf(l, r)
#elif CRS_FLOAT_TYPE == CRS_FLOAT_DOUBLE
#	define float_pow(l, r) pow(l, r)
#	define float_mod(l, r) fmod(l, r)
#elif CRS_FLOAT_TYPE == CRS_FLOAT_LDOUBLE
#	define float_pow(l, r) powl(l, r)
#	define float_mod(l, r) fmodl(l, r)
#endif

crs_Integer
crsV_length(crs_Thread* thread, crs_Object* object) {
	if (object->type == CRS_TYPE_STRING) {
		return obj_gets(object)->length;
	}

	error_op(thread, object, "get length of");
}

static void
error_arith(crs_Thread* thread, crs_Object* l, crs_Object* r, int op) {
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
		case CRS_OP_NOT:
			error_int(thread, r, r);
			error_unary(thread, r, "~");
		case CRS_OP_AND:
			error_int(thread, l, r);
			error_binary(thread, l, r, "&");
		case CRS_OP_OR:
			error_int(thread, l, r);
			error_binary(thread, l, r, "|");
		case CRS_OP_XOR:
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

static crs_Integer
arith_int(crs_Integer l, crs_Integer r, int op) {
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
		case CRS_OP_NOT:
			return ~r;
		case CRS_OP_AND:
			return l & r;
		case CRS_OP_OR:
			return l | r;
		case CRS_OP_XOR:
			return l ^ r;
		case CRS_OP_SHL:
			return l << r;
		case CRS_OP_SHR:
			return l >> r;
	}

	return 0;
}

static crs_Float
arith_float(crs_Float l, crs_Float r, int op) {
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
			return float_mod(l, r);
	}

	return 0;
}

static int
arith_raw(crs_Object* o, crs_Object* l, crs_Object* r, int op) {
	crs_Integer iL, iR;
	crs_Float   fL, fR;

	switch (op) {
		/* integers only */
		case CRS_OP_NOT: case CRS_OP_AND: case CRS_OP_OR:
		case CRS_OP_XOR: case CRS_OP_SHL: case CRS_OP_SHR:
			if (crsO_toInteger(l, &iL) && crsO_toInteger(r, &iR)) {
				crs_Integer result = arith_int(iL, iR, op);
				obj_seti(o, result);

				return 1;
			}

			break;
		/* floats only */
		case CRS_OP_DIV: case CRS_OP_POW:
			if (crsO_toFloat(l, &fL) && crsO_toFloat(r, &fR)) {
				crs_Float result = arith_float(fL, fR, op);
				obj_setf(o, result);

				return 1;
			}

			break;
		/* integers and floats */
		case CRS_OP_UNM: case CRS_OP_ADD: case CRS_OP_SUB:
		case CRS_OP_MUL: case CRS_OP_MOD:
			if (crsO_toInteger(l, &iL) && crsO_toInteger(r, &iR)) {
				crs_Integer result = arith_int(iL, iR, op);
				obj_seti(o, result);

				return 1;
			} else if (crsO_toFloat(l, &fL) && crsO_toFloat(r, &fR)) {
				crs_Float result = arith_float(fL, fR, op);
				obj_setf(o, result);

				return 1;
			}

			break;
	}

	return 0;
}

void
crsV_arith(crs_Thread* thread, crs_Object* o, crs_Object* l, crs_Object* r, int op) {
	if (arith_raw(o, l, r, op)) {
		return;
	}

	error_arith(thread, l, r, op);
}

crs_Object*
crsV_get(crs_Thread* thread, crs_Object* object, crs_Object* key) {
	if (object->type == CRS_TYPE_TABLE) {
		return crsT_get(thread, obj_gett(object), key);
	}

	error_op(thread, object, "index");
}

void
crsV_set(crs_Thread* thread, crs_Object* object, crs_Object* key, crs_Object* value) {
	if (object->type == CRS_TYPE_TABLE) {
		crsT_set(thread, obj_gett(object), key, value);
		return;
	}

	error_op(thread, object, "index");
}

/*
 * ===========================
 *  calling
 * ===========================
 */

typedef struct {
	crs_Object* object;
	int         args;
	int         wanted;
} PCallInfo;

static void*
pcall(crs_Thread* thread, void* data) {
	PCallInfo* info = data;
	crsV_call(thread, info->object, info->args, info->wanted);

	return NULL;
}

void
crsV_call(crs_Thread* thread, crs_Object* object, int args, int wanted) {
	if (object->type == CRS_TYPE_CFUNCTION) {
		crsC_callC(thread, obj_getc(object), args, wanted);
		return;
	}

	error_op(thread, object, "call");
}

int
crsV_pcall(crs_Thread* thread, crs_Object* object, int args, int wanted) {
	PCallInfo info;
	info.object = object;
	info.args   = args;
	info.wanted = wanted;

	short level  = thread->stack.level;
	int   status = crsC_try(thread, pcall, &info, NULL);

	if (status != CRS_STATUS_OK) {
		crsC_restoreStack(thread, level);
	}

	return status;
}
