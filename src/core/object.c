/*
 * https://github.com/mochji/crescent
 * core/object.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include <stddef.h>

#include "conf.h"
#include "limit.h"

#include "types/string.h"
#include "types/array.h"
#include "core/format.h"

int
crsO_compare(crs_Object* a, crs_Object* b) {
	crs_byte aType = a->type;
	crs_byte bType = b->type;

	if (aType != bType) {
		if (aType == CRS_TYPE_INTEGER && bType == CRS_TYPE_FLOAT) {
			return (crs_Float)obj_geti(a) == obj_getf(b);
		}

		if (aType == CRS_TYPE_FLOAT && bType == CRS_TYPE_INTEGER) {
			return obj_getf(a) == (crs_Float)obj_geti(b);
		}

		return 0;
	}

	switch (aType) {
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
			return crsF_toInteger(obj_gets(object)->contents, NULL);
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
			return crsF_toFloat(obj_gets(object)->contents, NULL);
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
			return obj_gets(object)->contents;
	}

	return "";
}
