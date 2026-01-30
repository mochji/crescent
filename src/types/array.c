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
#include "core/state.h"

#include "types/array.h"

crs_Array*
crsA_new(crs_Thread* thread, size_t size) {
	(void)thread;

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

void
crsA_free(crs_Thread* thread, crs_Array* array) {
	if (array == NULL) {
		return;
	}

	for (size_t a = 0; a < array->length; a++) {
		crsO_free(thread, &array->contents[a]);
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
