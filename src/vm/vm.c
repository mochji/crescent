/*
 * https://github.com/mochji/crescent
 * vm/vm.c
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

#include "core/object.h"

/*
 * This function is a more loose comparison compared to crescentO_compare, as
 * crescentO_compare returns false if the 2 objects are of differing types
 * while this allows comparisons between floats and integers.
 */

int
crescentV_compare(crescent_Object* a, crescent_Object* b) {
	int aType = a->type;
	int bType = b->type;

	if (aType != bType) {
		if (aType == CRESCENT_TYPE_INTEGER && bType == CRESCENT_TYPE_FLOAT) {
			return (crescent_Float)b->value.i == b->value.f;
		}

		if (aType == CRESCENT_TYPE_FLOAT && bType == CRESCENT_TYPE_INTEGER) {
			return a->value.f == (crescent_Float)b->value.i;
		}
	}

	return crescentO_compare(a, b);
}
