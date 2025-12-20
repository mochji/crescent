/*
 * https://github.com/mochji/crescent
 * core/object.c
 *
 * Copyright (C) 2025 mochji
 * MIT License
 */

#include "conf.h"

#include "types/string.h"
#include "types/array.h"

#include "core/object.h"

/*
 * This function is a strict comparison, see crsV_compare for comparison
 * between integers and floats.
 */

int
crsO_compare(crs_Object* a, crs_Object* b) {
	if (a->type != b->type) {
		return 0;
	}

	switch (a->type) {
		case CRS_TYPE_BOOLEAN:
			return a->value.b == b->value.b;
		case CRS_TYPE_INTEGER:
			return a->value.i == b->value.i;
		case CRS_TYPE_FLOAT:
			return a->value.f == b->value.f;
		case CRS_TYPE_STRING:
			return crsS_compare(a->value.s, b->value.s);
		case CRS_TYPE_ARRAY:
			return crsA_compare(a->value.a, b->value.a);
		case CRS_TYPE_CFUNCTION:
			return a->value.c == b->value.c;
	}

	return 0;
}

int
crsO_clone(crs_Object* to, crs_Object* from) {
	int       type  = from->type;
	crs_Value value = from->value;

	if (type == CRS_TYPE_STRING) {
		value.s->references += 1;
	} else if (type == CRS_TYPE_ARRAY) {
		value.a->references += 1;
	}

	to->type  = type;
	to->value = value;

	return 0;
}

int
crsO_deepClone(crs_Object* to, crs_Object* from) {
	void* cloned;

	if (from->type == CRS_TYPE_STRING) {
		cloned = crsS_clone(from->value.s);

		if (cloned == NULL) {
			return 1;
		}

		to->type    = CRS_TYPE_STRING;
		to->value.s = cloned;
	} else if (from->type == CRS_TYPE_ARRAY) {
		cloned = crsA_clone(from->value.a);

		if (cloned == NULL) {
			return 1;
		}

		to->type    = CRS_TYPE_ARRAY;
		to->value.a = cloned;
	} else {
		*to = *from;
	}

	return 0;
}

void
crsO_free(crs_Object* object) {
	if (object == NULL) {
		return;
	}

	if (object->type == CRS_TYPE_STRING) {
		object->value.s->references -= 1;

		if (object->value.s->references == 0) {
			crsS_free(object->value.s);
		}
	} else if (object->type == CRS_TYPE_ARRAY) {
		object->value.a->references -= 1;

		if (object->value.a->references == 0) {
			crsA_free(object->value.a);
		}
	}

	object->type = CRS_TYPE_NIL;
}

char*
crsO_typeName(int type) {
	switch (type) {
		case CRS_TYPE_NIL:
			return "nil";
		case CRS_TYPE_BOOLEAN:
			return "boolean";
		case CRS_TYPE_INTEGER:
			return "number";
		case CRS_TYPE_FLOAT:
			return "number";
		case CRS_TYPE_STRING:
			return "string";
		case CRS_TYPE_ARRAY:
			return "array";
		case CRS_TYPE_CFUNCTION:
			return "function";
	}

	return NULL;
}

int
crsO_toBoolean(crs_Object* object, int* match) {
	int type = object->type;

	if (match != NULL) {
		*match = type == CRS_TYPE_BOOLEAN;
	}

	if (type == CRS_TYPE_NIL) {
		return 0;
	} else if (type == CRS_TYPE_BOOLEAN) {
		return object->value.b;
	}

	return 1;
}

crs_Integer
crsO_toInteger(crs_Object* object, int* match) {
	int type = object->type;

	if (match != NULL) {
		*match = type == CRS_TYPE_INTEGER;
	}

	switch (type) {
		case CRS_TYPE_INTEGER:
			return object->value.i;
		case CRS_TYPE_FLOAT:
			return (crs_Integer)object->value.f;
		case CRS_TYPE_STRING:
			return crsS_toInteger(object->value.s->value, NULL);
	}

	return 0;
}

crs_Float
crsO_toFloat(crs_Object* object, int* match) {
	int type = object->type;

	if (match != NULL) {
		*match = type == CRS_TYPE_FLOAT;
	}

	switch (type) {
		case CRS_TYPE_INTEGER:
			return (crs_Float)object->value.i;
		case CRS_TYPE_FLOAT:
			return object->value.f;
		case CRS_TYPE_STRING:
			return crsS_toFloat(object->value.s->value, NULL);
	}

	return 0;
}

char*
crsO_toString(crs_Object* object, int* match) {
	int type = object->type;

	if (match != NULL) {
		*match = type == CRS_TYPE_STRING;
	}

	if (object->type == CRS_TYPE_NIL) {
		return "nil";
	} else if (object->type == CRS_TYPE_BOOLEAN) {
		return object->value.b ? "true" : "false";
	}

	return NULL;
}
