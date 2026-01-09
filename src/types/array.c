/*
 * https://github.com/mochji/crescent
 * types/array.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include <stdlib.h>
#include <stddef.h>

#include "conf.h"
#include "limit.h"

#include "core/object.h"

#include "types/array.h"

crs_Array*
crsA_new(size_t size) {
	crs_Array* array = malloc(sizeof(crs_Array));

	if (array == NULL) {
		return NULL;
	}

	/* TODO: properly set up gc header once gc is implemented */
	array->header.type = CRS_TYPE_ARRAY;

	array->size       = size;
	array->length     = 0;
	array->references = 1;
	array->contents   = malloc(size * sizeof(crs_Object));

	if (array->contents == NULL) {
		free(array);

		return NULL;
	}

	return array;
}

crs_Array*
crsA_clone(crs_Array* array) {
	crs_Array* cloned = malloc(sizeof(crs_Array));

	if (cloned == NULL) {
		return NULL;
	}

	/* TODO: properly set up header once gc is implemented */
	cloned->header.type = CRS_TYPE_ARRAY;

	cloned->size       = array->size;
	cloned->length     = array->length;
	cloned->contents   = malloc(array->size * sizeof(crs_Object));
	cloned->references = 1;

	if (cloned->contents == NULL) {
		free(cloned);

		return NULL;
	}

	for (size_t a = 0; a < array->length; a++) {
		if (crsO_deepClone(&cloned->contents[a], &array->contents[a])) {
			cloned->length = a;

			crsA_free(cloned);

			return NULL;
		}
	}

	return cloned;
}

void
crsA_free(crs_Array* array) {
	if (array == NULL) {
		return;
	}

	for (size_t a = 0; a < array->length; a++) {
		crsO_free(&array->contents[a]);
	}

	free(array->contents);
	free(array);
}

int
crsA_resize(crs_Array* array, size_t newLength) {
	if (newLength >= array->size || array->size - newLength > array->size / 2) {
		size_t      newSize = newLength + CRS_ARRAY_ALLOCSPACE;
		crs_Object* newData = realloc(array->contents, newSize);

		if (newData == NULL) {
			return 1;
		}

		array->size  = newSize;
		array->contents = newData;
	}

	return 0;
}

int
crsA_compare(crs_Array* arrayA, crs_Array* arrayB) {
	if (arrayA == arrayB) {
		return 1;
	}

	if (arrayA->length != arrayB->length) {
		return 0;
	}

	for (size_t a = 0; a < arrayA->length; a++) {
		if (!crsO_compare(&arrayA->contents[a], &arrayB->contents[a])) {
			return 0;
		}
	}

	return 1;
}
