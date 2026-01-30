/*
 * https://github.com/mochji/crescent
 * types/string.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "conf.h"
#include "limit.h"

#include "core/object.h"
#include "core/state.h"
#include "core/memory.h"
#include "core/gc.h"

#include "types/string.h"

crs_String*
crsS_new(crs_Thread* thread, char* str) {
	size_t length   = strlen(str);
	char*  contents = mem_snew(thread, length);

	if (contents == NULL) {
		return NULL;
	}

	crs_String* string = crsG_new(thread, CRS_TYPE_STRING, sizeof(crs_String));

	if (string == NULL) {
		mem_sfree(thread, contents, length);

		return NULL;
	}

	string->length     = length;
	string->contents   = contents;
	string->references = 1;

	return string;
}

void
crsS_free(crs_Thread* thread, crs_String* string) {
	mem_sfree(thread, string->contents, string->length);
	mem_freeobj(thread, string);
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
