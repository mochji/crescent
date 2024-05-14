/*
 * https://github.com/mochji/crescent
 * types/array.c
 *
 * idrk what this is
 * Copyright (C) 2024 mochji
 *
 * MIT License
 */

#include <stdlib.h>
#include <stddef.h>

#include "conf.h"

#include "core/object.h"

#include "types/array.h"

crescent_Array*
crescentA_new(size_t length) {
	crescent_Array* array = malloc(sizeof(crescent_Array));

	if (array == NULL) {
		return NULL;
	}

	array->size       = length + CRESCENT_ARRAY_ALLOCSPACE;
	array->length     = length;
	array->data       = malloc(array->size * sizeof(crescent_Object));
	array->references = 1;

	if (array->data == NULL) {
		free(array);

		return NULL;
	}

	return array;
}

crescent_Array*
crescentA_clone(crescent_Array* array) {
	crescent_Array* cloned = malloc(sizeof(crescent_Array));

	if (cloned == NULL) {
		return NULL;
	}

	cloned->size       = array->size;
	cloned->length     = array->length;
	cloned->data       = malloc(array->size * sizeof(crescent_Object));
	cloned->references = 1;

	if (cloned->data == NULL) {
		free(cloned);

		return NULL;
	}

	for (size_t a = 0; a < array->length; a++) {
		if (crescentO_deepClone(&cloned->data[a], &array->data[a])) {
			cloned->length = a;

			crescentA_free(cloned);

			return NULL;
		}
	}

	return cloned;
}

void
crescentA_free(crescent_Array* array) {
	if (array == NULL) {
		return;
	}

	for (size_t a = 0; a < array->length; a++) {
		crescentO_free(&array->data[a]);
	}

	free(array->data);
	free(array);
}

int
crescentA_resize(crescent_Array* array, size_t newLength) {
	if (newLength >= array->size || array->size - newLength > array->size / 2) {
		size_t           newSize = newLength + CRESCENT_ARRAY_ALLOCSPACE;
		crescent_Object* newData = realloc(array->data, newSize);

		if (newData == NULL) {
			return 1;
		}

		array->size = newSize;
		array->data = newData;
	}

	return 0;
}

int
crescentA_compare(crescent_Array* arrayA, crescent_Array* arrayB) {
	if (arrayA == arrayB) {
		return 1;
	}

	if (arrayA->length != arrayB->length) {
		return 0;
	}

	for (size_t a = 0; a < arrayA->length; a++) {
		if (!crescentO_compare(&arrayA->data[a], &arrayB->data[a])) {
			return 0;
		}
	}

	return 1;
}
