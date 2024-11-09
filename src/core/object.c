/*
 * https://github.com/mochji/crescent
 * core/object.c
 *
 * Copyright (C) 2024 mochji
 * MIT License
 */

#include "conf.h"

#include "types/string.h"
#include "types/array.h"

#include "core/object.h"

/*
 * This function is a strict comparison, see crescentV_compare for comparison
 * between integers and floats.
 */

int
crescentO_compare(crescent_Object* a, crescent_Object* b) {
	if (a->type != b->type) {
		return 0;
	}

	switch (a->type) {
		case CRESCENT_TYPE_BOOLEAN:
			return a->value.b == b->value.b;

			break;
		case CRESCENT_TYPE_INTEGER:
			return a->value.i == b->value.i;

			break;
		case CRESCENT_TYPE_FLOAT:
			return a->value.f == b->value.f;

			break;
		case CRESCENT_TYPE_STRING:
			return crescentS_compare(a->value.s, b->value.s);

			break;
		case CRESCENT_TYPE_ARRAY:
			return crescentA_compare(a->value.a, b->value.a);

			break;
		case CRESCENT_TYPE_CFUNCTION:
			return a->value.c == b->value.c;

			break;
	}

	return 0;
}

int
crescentO_clone(crescent_Object* to, crescent_Object* from) {
	int            type  = from->type;
	crescent_Value value = from->value;

	if (type == CRESCENT_TYPE_STRING) {
		value.s->references += 1;
	} else if (type == CRESCENT_TYPE_ARRAY) {
		value.a->references += 1;
	}

	to->type  = type;
	to->value = value;

	return 0;
}

int
crescentO_deepClone(crescent_Object* to, crescent_Object* from) {
	void* cloned;

	if (from->type == CRESCENT_TYPE_STRING) {
		cloned = crescentS_clone(from->value.s);

		if (cloned == NULL) {
			return 1;
		}

		to->type    = CRESCENT_TYPE_STRING;
		to->value.s = cloned;
	} else if (from->type == CRESCENT_TYPE_ARRAY) {
		cloned = crescentA_clone(from->value.a);

		if (cloned == NULL) {
			return 1;
		}

		to->type    = CRESCENT_TYPE_ARRAY;
		to->value.a = cloned;
	} else {
		*to = *from;
	}

	return 0;
}

void
crescentO_free(crescent_Object* object) {
	if (object == NULL) {
		return;
	}

	if (object->type == CRESCENT_TYPE_STRING) {
		object->value.s->references -= 1;

		if (object->value.s->references == 0) {
			crescentS_free(object->value.s);
		}
	} else if (object->type == CRESCENT_TYPE_ARRAY) {
		object->value.a->references -= 1;

		if (object->value.a->references == 0) {
			crescentA_free(object->value.a);
		}
	}

	object->type = CRESCENT_TYPE_NIL;
}

char*
crescentO_typeName(int type) {
	switch (type) {
		case CRESCENT_TYPE_NIL:
			return "nil";

			break;
		case CRESCENT_TYPE_BOOLEAN:
			return "boolean";

			break;
		case CRESCENT_TYPE_INTEGER:
			return "number";

			break;
		case CRESCENT_TYPE_FLOAT:
			return "number";

			break;
		case CRESCENT_TYPE_STRING:
			return "string";

			break;
		case CRESCENT_TYPE_ARRAY:
			return "array";

			break;
		case CRESCENT_TYPE_CFUNCTION:
			return "function";

			break;
	}

	return NULL;
}

int
crescentO_toBoolean(crescent_Object* object, int* match) {
	int type = object->type;

	if (match != NULL) {
		*match = type == CRESCENT_TYPE_BOOLEAN;
	}

	switch (type) {
		case CRESCENT_TYPE_NIL:
			return 0;

			break;
		case CRESCENT_TYPE_BOOLEAN:
			return object->value.b;

			break;
		case CRESCENT_TYPE_INTEGER:
			return object->value.i != 0;

			break;
		case CRESCENT_TYPE_FLOAT:
			return object->value.f != 0;

			break;
	}

	return 1;
}

crescent_Integer
crescentO_toInteger(crescent_Object* object, int* match) {
	int type = object->type;

	if (match != NULL) {
		*match = type == CRESCENT_TYPE_INTEGER;
	}

	switch (type) {
		case CRESCENT_TYPE_INTEGER:
			return object->value.i;

			break;
		case CRESCENT_TYPE_FLOAT:
			return (crescent_Integer)object->value.f;

			break;
		case CRESCENT_TYPE_STRING:
			return crescentS_toInteger(object->value.s->value, NULL);

			break;
	}

	return 0;
}

crescent_Float
crescentO_toFloat(crescent_Object* object, int* match) {
	int type = object->type;

	if (match != NULL) {
		*match = type == CRESCENT_TYPE_FLOAT;
	}

	switch (type) {
		case CRESCENT_TYPE_INTEGER:
			return (crescent_Float)object->value.i;

			break;
		case CRESCENT_TYPE_FLOAT:
			return object->value.f;

			break;
		case CRESCENT_TYPE_STRING:
			return crescentS_toFloat(object->value.s->value, NULL);

			break;
	}

	return 0;
}

char*
crescentO_toString(crescent_Object* object, int* match) {
	int type = object->type;

	if (match != NULL) {
		*match = type == CRESCENT_TYPE_STRING;
	}

	if (object->type == CRESCENT_TYPE_NIL) {
		return "nil";
	} else if (object->type == CRESCENT_TYPE_BOOLEAN) {
		return object->value.b ? "true" : "false";
	}

	return NULL;
}
