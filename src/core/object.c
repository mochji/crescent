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

#include "core/object.h"

int
crescentO_clone(crescent_Object* to, crescent_Object* from) {
	*to = *from;

	return 0;
}

void
crescentO_free(crescent_Object* object) {
	if (object == NULL) {
		return;
	}

	object->type = CRESCENT_TYPE_NONE;
}
