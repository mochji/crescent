/*
 * https://github.com/mochji/crescent
 * core/object.c
 *
 * Copyright (C) 2025 mochji
 * MIT License
 */

#include "conf.h"
#include "limit.h"

#include "types/string.h"
#include "types/array.h"

#include "core/object.h"

/*
 * This function is a strict comparison; see crsV_compare for comparison
 * between integers and floats.
 */

int
crsO_compare(crs_Object* a, crs_Object* b) {
	if (a->type != b->type) {
		return 0;
	}

	switch (a->type) {
		case CRS_TYPE_BOOLEAN:
			return obj_getb(a) == obj_getb(b);
		case CRS_TYPE_INTEGER:
			return obj_geti(a) == obj_geti(b);
		case CRS_TYPE_FLOAT:
			return obj_getf(a) == obj_getf(b);
		case CRS_TYPE_CFUNCTION:
			return obj_getc(a) == obj_getc(b);
		case CRS_TYPE_STRING:
			return crsS_compare(obj_gets(a), obj_gets(b));
		case CRS_TYPE_ARRAY:
			return crsA_compare(obj_geta(a), obj_geta(b));
	}

	return 0;
}

int
crsO_clone(crs_Object* to, crs_Object* from) {
	/* TODO: remove me! */

	int type = from->type;

	if (type == CRS_TYPE_STRING) {
		obj_gets(from)->references += 1;
	} else if (type == CRS_TYPE_ARRAY) {
		obj_geta(from)->references += 1;
	}

	obj_seto(to, from);

	return 0;
}

int
crsO_deepClone(crs_Object* to, crs_Object* from) {
	/* TODO: remove me! */

	void* cloned;

	if (from->type == CRS_TYPE_STRING) {
		cloned = crsS_clone(obj_gets(from));

		if (cloned == NULL) {
			return 1;
		}

		obj_setgc(to, cloned);
	} else if (from->type == CRS_TYPE_ARRAY) {
		cloned = crsA_clone(obj_geta(from));

		if (cloned == NULL) {
			return 1;
		}

		obj_setgc(to, cloned);
	} else {
		obj_seto(to, from);
	}

	return 0;
}

void
crsO_free(crs_Object* object) {
	if (object == NULL) {
		return;
	}

	if (object->type == CRS_TYPE_STRING) {
		crs_String* string  = obj_gets(object);
		string->references -= 1;

		if (string->references == 0) {
			crsS_free(string);
		}
	} else if (object->type == CRS_TYPE_ARRAY) {
		crs_Array* array   = obj_geta(object);
		array->references -= 1;

		if (array->references == 0) {
			crsA_free(array);
		}
	}

	obj_setn(object);
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
		return obj_getb(object);
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
			return obj_geti(object);
		case CRS_TYPE_FLOAT:
			return (crs_Integer)obj_getf(object);
		case CRS_TYPE_STRING:
			return crsS_toInteger(obj_gets(object)->value, NULL);
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
			return (crs_Float)obj_geti(object);
		case CRS_TYPE_FLOAT:
			return obj_getf(object);
		case CRS_TYPE_STRING:
			return crsS_toFloat(obj_gets(object)->value, NULL);
	}

	return 0;
}

char*
crsO_toString(crs_Object* object, int* match) {
	int type = object->type;

	if (match != NULL) {
		*match = type == CRS_TYPE_STRING;
	}

	switch (type) {
		case CRS_TYPE_NIL:
			return "nil";
		case CRS_TYPE_BOOLEAN:
			return obj_getb(object) ? "true" : "false";
		case CRS_TYPE_STRING:
			return obj_gets(object)->value;
	}

	return "";
}
