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

#include <stddef.h>

#include "conf.h"

#include "types/string.h"

#include "core/object.h"

int
crescentO_compare(crescent_Object* a, crescent_Object* b) {
	if (a->type != b->type) {
		return 1;
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
		case CRESCENT_TYPE_CFUNCTION:
			return a->value.cFunc != b->value.cFunc;

			break;
	}

	return 1;
}

size_t
crescentO_hash(crescent_Object* object) {
	switch (object->type) {
		case CRESCENT_TYPE_BOOLEAN:
			return object->value.b;

			break;
		case CRESCENT_TYPE_INTEGER:
			return object->value.i;

			break;
		case CRESCENT_TYPE_FLOAT:
			return object->value.f;

			break;
		case CRESCENT_TYPE_STRING:
			return crescentS_hash(object->value.s);

			break;
		case CRESCENT_TYPE_CFUNCTION:
			return (size_t)object->value.cFunc;

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

		to->value.s = cloned;
		to->type    = CRESCENT_TYPE_STRING;
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
		crescentS_free(object->value.s);
	}

	object->type = CRESCENT_TYPE_NONE;
}
