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
#include "core/call.h"
#include "core/gc.h"

#include "types/string.h"

#define MAX_SIZE ((SIZE_MAX - sizeof(crs_String)) / sizeof(char))

static crs_String*
newString(crs_Thread* thread, size_t length, size_t after) {
	if (length > CRS_MAX_LENGTH || after > MAX_SIZE) {
		crsC_error(thread, "string too big");
	}

	size_t      size   = sizeof(crs_String) + after;
	crs_String* string = mem_alloc(thread, size);

	if (string == NULL) {
		crsM_error(thread);
	}

	string->size   = size;
	string->length = length;

	return string;
}

crs_String*
crsS_new(crs_Thread* thread, char* str) {
	size_t      length = strlen(str);
	crs_String* string = newString(thread, length, length + 1);

	string->contents = strcpy((char*)(string + 1), str);

	return crsG_add(thread, string, CRS_TYPE_STRING);
}

crs_String*
crsS_literal(crs_Thread* thread, char* str) {
	crs_String* string = newString(thread, strlen(str), 0);
	string->contents   = str;

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
