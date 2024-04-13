/*
 * https://github.com/mochji/crescent
 * core/object.c
 *
 * idrk what this is
 * Copyright (C) 2024 mochji
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "conf.h"

#include "types/string.h"
#include "types/array.h"

#include "core/object.h"

int
crescentO_compare(crescent_Object* a, crescent_Object* b) {
	if (a->type != b->type) {
		return 1;
	}

	switch (a->type) {
		case CRESCENT_TYPE_BOOLEAN:
			return a->value.b != b->value.b;

			break;
		case CRESCENT_TYPE_INTEGER:
			return a->value.i != b->value.i;

			break;
		case CRESCENT_TYPE_FLOAT:
			return a->value.f != b->value.f;

			break;
		case CRESCENT_TYPE_STRING:
			return crescentS_compare(a->value.s, b->value.s);

			break;
		case CRESCENT_TYPE_ARRAY:
			return crescentA_compare(a->value.a, b->value.a);

			break;
		case CRESCENT_TYPE_CFUNCTION:
			return a->value.c != b->value.c;

			break;
	}

	return 1;
}

int
crescentO_clone(crescent_Object* to, crescent_Object* from) {
	if (from->type == CRESCENT_TYPE_STRING) {
		crescent_String* cloned = crescentS_clone(from->value.s);

		if (cloned == NULL) {
			return 1;
		}

		to->type    = CRESCENT_TYPE_STRING;
		to->value.s = cloned;
	} else {
		if (from->type == CRESCENT_TYPE_ARRAY) {
			from->value.a->references += 1;
		}

		*to = *from;
	}

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

crescent_Boolean
crescentO_toBoolean(crescent_Object* object, int* isBoolean) {
	if (object->type == CRESCENT_TYPE_BOOLEAN) {
		if (isBoolean != NULL) {
			*isBoolean = 1;
		}

		return object->value.b;
	}

	if (isBoolean != NULL) {
		*isBoolean = 0;
	}

	switch (object->type) {
		case CRESCENT_TYPE_INTEGER:
			return object->value.i != 0;

			break;
		case CRESCENT_TYPE_FLOAT:
			return object->value.f != 0;

			break;
		case CRESCENT_TYPE_STRING:
			return 1;

			break;
		case CRESCENT_TYPE_ARRAY:
			return 1;

			break;
		case CRESCENT_TYPE_CFUNCTION:
			return 1;

			break;
	}

	return 0;
}

crescent_Integer
crescentO_toInteger(crescent_Object* object, int* isInteger) {
	if (object->type == CRESCENT_TYPE_INTEGER) {
		if (isInteger != NULL) {
			*isInteger = 1;
		}

		return object->value.i;
	}

	if (isInteger != NULL) {
		*isInteger = 0;
	}

	if (object->type == CRESCENT_TYPE_BOOLEAN) {
		return (crescent_Integer)object->value.b;
	}

	if (object->type == CRESCENT_TYPE_FLOAT) {
		return (crescent_Float)object->value.f;
	}

	if (object->type == CRESCENT_TYPE_STRING) {
		return crescentS_toInteger(object->value.s->data, NULL);
	}

	return 0;
}

crescent_Float
crescentO_toFloat(crescent_Object* object, int* isFloat) {
	if (object->type == CRESCENT_TYPE_FLOAT) {
		if (isFloat != NULL) {
			*isFloat = 1;
		}

		return object->value.f;
	}

	if (isFloat != NULL) {
		*isFloat = 0;
	}

	if (object->type == CRESCENT_TYPE_BOOLEAN) {
		return (crescent_Float)object->value.b;
	}

	if (object->type == CRESCENT_TYPE_INTEGER) {
		return (crescent_Float)object->value.i;
	}

	if (object->type == CRESCENT_TYPE_STRING) {
		return crescentS_toFloat(object->value.s->data, NULL);
	}

	return 0;
}

char*
crescentO_toString(crescent_Object* object, int* isString) {
	if (object->type == CRESCENT_TYPE_STRING) {
		if (isString != NULL) {
			*isString = 1;
		}

		return object->value.s->data;
	}

	if (isString != NULL) {
		*isString = 0;
	}

	return NULL;
}

crescent_CFunction*
crescentO_toCFunction(crescent_Object* object, int* isCFunction) {
	if (object->type == CRESCENT_TYPE_CFUNCTION) {
		if (isCFunction != NULL) {
			*isCFunction = 1;
		}

		return object->value.c;
	}

	if (isCFunction != NULL) {
		*isCFunction = 0;
	}

	return NULL;
}
