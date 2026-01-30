/*
 * https://github.com/mochji/crescent
 * types/array.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include <stddef.h>

#include "conf.h"
#include "limit.h"

#include "core/object.h"
#include "core/state.h"
#include "core/memory.h"
#include "core/gc.h"

#include "types/array.h"

crs_Array*
crsA_new(crs_Thread* thread, size_t size) {
	crs_Object* contents = mem_vnew(thread, size, crs_Object);

	if (contents == NULL) {
		return NULL;
	}

	crs_Array* array = crsG_new(thread, CRS_TYPE_ARRAY, sizeof(crs_Array));

	if (array == NULL) {
		mem_vfree(thread, contents, size, crs_Object);

		return NULL;
	}

	array->size       = size;
	array->length     = 0;
	array->contents   = contents;
	array->references = 1;

	return array;
}

void
crsA_free(crs_Thread* thread, crs_Array* array) {
	for (size_t a = 0; a < array->length; a++) {
		crsO_free(thread, &array->contents[a]);
	}

	mem_vfree(thread, array->contents, array->size, crs_Object);
	mem_freeobj(thread, array);
}

int
crsA_compare(crs_Array* arrayA, crs_Array* arrayB) {
	return arrayA == arrayB;
}
