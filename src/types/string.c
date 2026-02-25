/*
 * https://github.com/mochji/crescent
 * types/string.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include <stddef.h>
#include <string.h>

#include "conf.h"
#include "limit.h"

#include "core/object.h"
#include "core/state.h"
#include "core/memory.h"
#include "core/gc.h"

#include "types/string.h"

static crs_String*
newString(crs_Thread* thread, size_t length) {
	size_t      size   = sizeof(crs_String) + length;
	crs_String* string = mem_alloc(thread, size);

	if (string == NULL) {
		return NULL;
	}

	string->size = size;

	return string;
}

crs_String*
crsS_new(crs_Thread* thread, char* str) {
	size_t      length = strlen(str);
	crs_String* string = newString(thread, length + 1);

	if (string == NULL) {
		return NULL;
	}

	string->length   = length;
	string->contents = strcpy((char*)(string + 1), str);

	return crsG_add(thread, string, CRS_TYPE_STRING);
}

crs_String*
crsS_external(crs_Thread* thread, char* str) {
	crs_String* string = newString(thread, 0);

	if (string == NULL) {
		return NULL;
	}

	string->length   = strlen(str);
	string->contents = str;

	return crsG_add(thread, string, CRS_TYPE_STRING);
}

void
crsS_free(crs_Thread* thread, crs_String* string) {
	mem_dealloc(thread, string, string->size);
}

int
crsS_compare(crs_String* stringA, crs_String* stringB) {
	if (stringA == stringB) {
		return 1;
	}

	if (stringA->length != stringB->length) {
		return 0;
	}

	return strcmp(stringA->contents, stringB->contents) == 0;
}
