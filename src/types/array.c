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
		crsM_error(thread);
	}

	crs_Array* array = mem_new(thread, crs_Array);

	if (array == NULL) {
		mem_vfree(thread, contents, size, crs_Object);

		crsM_error(thread);
	}

	array->size       = size;
	array->length     = 0;
	array->contents   = contents;

	return crsG_add(thread, array, CRS_TYPE_ARRAY);
}

void
crsA_free(crs_Thread* thread, crs_Array* array) {
	mem_vfree(thread, array->contents, array->size, crs_Object);
	mem_free(thread, array);
}

int
crsA_compare(crs_Array* arrayA, crs_Array* arrayB) {
	return arrayA == arrayB;
}
