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
#include "types/table.h"
#include "core/format.h"

char*
crsO_name(crs_Object* object) {
	switch (object->type) {
		case CRS_TYPE_NIL:       return "nil";
		case CRS_TYPE_BOOLEAN:   return "boolean";
		case CRS_TYPE_INTEGER:   return "number";
		case CRS_TYPE_FLOAT:     return "number";
		case CRS_TYPE_CFUNCTION: return "function";
		case CRS_TYPE_STRING:    return "string";
		case CRS_TYPE_TABLE:     return "table";
		case CRS_TYPE_THREAD:    return "thread";
	}

	return NULL;
}

int
crsO_toBoolean(crs_Object* object, int* match) {
	crs_byte type = object->type;

	if (match != NULL) {
		*match = type == CRS_TYPE_BOOLEAN;
	}

	if (type == CRS_TYPE_NIL) {
		return 0;
	} else if (type == CRS_TYPE_BOOLEAN) {
		return obj_getb(object);
	}

	return 1; /* object exists */
}

crs_Integer
crsO_toInteger(crs_Object* object, int* match) {
	crs_byte    type = object->type;
	crs_Integer value;
	int         dummy;

	match = match == NULL ? &dummy : match;

	switch (type) {
		case CRS_TYPE_INTEGER:
			*match = 1;

			return obj_geti(object);
		case CRS_TYPE_FLOAT:
			value  = (crs_Integer)obj_getf(object);
			*match = value == obj_getf(object);

			return value;
		case CRS_TYPE_STRING:
			return crsF_toInteger(obj_gets(object)->contents, match);
	}

	return 0;
}

crs_Float
crsO_toFloat(crs_Object* object, int* match) {
	crs_byte  type = object->type;
	crs_Float value;
	int       dummy;

	match = match == NULL ? &dummy : match;

	switch (type) {
		case CRS_TYPE_INTEGER:
			value  = (crs_Float)obj_geti(object);
			*match = 1;

			return value;
		case CRS_TYPE_FLOAT:
			*match = 1;

			return obj_getf(object);
		case CRS_TYPE_STRING:
			return crsF_toFloat(obj_gets(object)->contents, match);
	}

	return 0;
}

char*
crsO_toString(crs_Object* object, int* match) {
	crs_byte type = object->type;

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

	return NULL;
}
