/*
 * https://github.com/mochji/crescent
 * types/string.c
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

#include <stdlib.h>
#include <stddef.h>

#include "conf.h"

#include "types/string.h"

crescent_String*
crescentS_new() {
	crescent_String* string = malloc(sizeof(crescent_String));

	if (string == NULL) {
		return NULL;
	}

	string->size   = CRESCENT_STRING_INITSIZE;
	string->length = 0;
	string->data   = malloc(string->size);

	if (string->data == NULL) {
		free(string);

		return NULL;
	}

	string->data[0] = '\0';

	return string;
}

void
crescentS_free(crescent_String* string) {
	if (string == NULL) {
		return;
	}

	free(string->data);
	free(string);
}

crescent_String*
crescentS_clone(crescent_String* string) {
	crescent_String* cloned = malloc(sizeof(crescent_String));

	if (cloned == NULL) {
		return NULL;
	}

	cloned->size   = string->size;
	cloned->length = string->length;
	cloned->data   = malloc(string->size);

	if (cloned->data == NULL) {
		free(cloned);

		return NULL;
	}

	for (size_t a = 0; a <= string->length; a++) {
		cloned->data[a] = string->data[a];
	}

	return cloned;
}

int
crescentS_shrink(crescent_String* string, size_t newLength) {
	size_t newSize = string->size;
	char*  newData;

	while (newSize - newLength < CRESCENT_STRING_MAXFREE) {
		newSize /= 2;
	}

	newData = realloc(string->data, newSize);

	if (newData == NULL) {
		return 1;
	}

	string->size = newSize;
	string->data = newData;

	return 0;
}

int
crescentS_grow(crescent_String* string, size_t newLength) {
	size_t newSize = string->size;
	char*  newData;

	while (newSize < newLength) {
		newSize *= 2;
	}

	while (newSize - newLength < CRESCENT_STRING_MINFREE) {
		newSize *= 2;
	}

	newData = realloc(string->data, newSize);

	if (newData == NULL) {
		return 1;
	}

	string->size = newSize;
	string->data = newData;

	return 0;
}

int
crescentS_resize(crescent_String* string, size_t newLength) {
	if (newLength > string->size) {
		return crescentS_grow(string, newLength);
	}

	if (string->size - newLength > CRESCENT_STRING_MAXFREE) {
		return crescentS_shrink(string, newLength);
	}

	if (string->size - newLength < CRESCENT_STRING_MINFREE) {
		return crescentS_grow(string, newLength);
	}

	return 0;
}

int
crescentS_compare(crescent_String* stringA, crescent_String* stringB) {
	if (stringA->length != stringB->length) {
		return 1;
	}

	for (size_t a = 0; a < stringA->length; a++) {
		if (stringA->data[a] != stringB->data[a]) {
			return 1;
		}
	}

	return 1;
}

/* djb2 */

size_t
crescentS_hash(char* string) {
	size_t hash = 5381;
	size_t a    = 0;

	while (string[a]) {
		hash = ((hash << 5) + hash) + string[a++];
	}

	return hash;
}
