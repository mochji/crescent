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

/*
 * crs_String->size signals if the string stored is contained within the same
 * block as the header or if it's externally allocated. A value of 0 indicates
 * that the string is stored directly after the crs_String struct, and therefore
 * the size of it is sizeof(crs_String) + string->length + 1; any other value is
 * the size of the block that string->contents points to.
 *
 * IMPORTANT: When the string is freed, so will the external block (if it
 * exists), so the one string that is using that block should be the only thing
 * doing so.
 */

static crs_String*
newString(crs_Thread* thread, size_t length, size_t after) {
	if (length > CRS_MAX_LENGTH || after > MAX_SIZE) {
		crsC_error(thread, "string too big");
	}

	crs_String* string = mem_alloc(thread, sizeof(crs_String) + after);

	if (string == NULL) {
		crsM_error(thread);
	}

	string->length = length;

	return string;
}

crs_String*
crsS_new(crs_Thread* thread, char* str) {
	size_t      length = strlen(str);
	crs_String* string = newString(thread, length, length + 1);

	string->size     = 0;
	string->contents = strcpy((char*)(string + 1), str);

	return string;
}

crs_String*
crsS_external(crs_Thread* thread, char* str, size_t size) {
	crs_String* string = newString(thread, strlen(str), 0);

	string->size       = size;
	string->contents   = str;

	return string;
}

void
crsS_free(crs_Thread* thread, crs_String* string) {
	size_t size = sizeof(crs_String);

	if (string->size) {
		mem_vfree(thread, string->contents, string->size);
	} else {
		size += string->length + 1;
	}

	mem_dealloc(thread, string, size);
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
