/*
 * https://github.com/mochji/crescent
 * core/object.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include <stddef.h>

#include "crescent/conf.h"
#include "limit.h"

#include "types/string.h"
#include "types/table.h"
#include "core/format.h"

crs_Object crsO_nilValue = {.type = CRS_TYPE_NIL};

char* crsO_name(crs_Object* object) {
    switch (object->type) {
        case CRS_TYPE_NIL:       return "nil";
        case CRS_TYPE_BOOLEAN:   return "boolean";
        case CRS_TYPE_INTEGER:   return "number";
        case CRS_TYPE_FLOAT:     return "number";
        case CRS_TYPE_CFUNCTION: return "function";
        case CRS_TYPE_STRING:    return "string";
        case CRS_TYPE_TABLE:     return "table";
        case CRS_TYPE_FUNCTION:  return "function";
        case CRS_TYPE_THREAD:    return "thread";
    }

    return NULL;
}

int crsO_test(crs_Object* object) {
    if (object->type == CRS_TYPE_BOOLEAN) {
        return obj_getb(object) != 0;
    }

    return object->type != CRS_TYPE_NIL;
}

int crsO_toInteger(crs_Object* object, crs_Integer* result, int coerce) {
    crs_Integer value   = 0;
    int         success = 0;

    switch (object->type) {
        case CRS_TYPE_INTEGER:
            value   = obj_geti(object);
            success = 1;

            break;
        case CRS_TYPE_FLOAT:
            value   = (crs_Integer)obj_getf(object);
            success = value == obj_getf(object);

            break;
        case CRS_TYPE_STRING:
            if (coerce) {
                success = crsF_toInteger(obj_gets(object)->contents, &value);
            }

            break;
    }

    *result = value;
    return success;
}

int crsO_toFloat(crs_Object* object, crs_Float* result, int coerce) {
    crs_Float value   = 0;
    int       success = 0;

    switch (object->type) {
        case CRS_TYPE_INTEGER:
            value   = (crs_Float)obj_geti(object);
            success = 1;

            break;
        case CRS_TYPE_FLOAT:
            value   = obj_getf(object);
            success = 1;

            break;
        case CRS_TYPE_STRING:
            if (coerce) {
                success = crsF_toFloat(obj_gets(object)->contents, &value);
            }

            break;
    }

    *result = value;
    return success;
}

char* crsO_toString(crs_Object* object, int* match) {
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
