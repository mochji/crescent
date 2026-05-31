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
#include "core/buffer.h"
#include "core/call.h"
#include "core/gc.h"

#include "types/string.h"

#define MAX_LENGTH ((SIZE_MAX - sizeof(crs_String)) / sizeof(char) - 1)

static crs_String*
newString(crs_Thread* thread, size_t length) {
	if (length > CRS_MAX_LENGTH || length > MAX_LENGTH) {
		crsC_error(thread, "string too big");
	}

	crs_String* string = mem_alloc(thread,
		sizeof(crs_String) + (length + 1) * sizeof(char));

	if (string == NULL) {
		crsM_error(thread);
	}

	string->length = length;

	return string;
}

/* djb2 hash */
static void
copyString(crs_String* string, char* source, size_t length) {
	char*  destination = string->contents;
	size_t hash        = 5381;

	while (length--) {
		char c         = *source++;
		*destination++ = c;
		hash           = ((hash << 5) + hash) + c;
	}

	*destination = '\0';
	string->hash = hash;
}

crs_String*
crsS_newl(crs_Thread* thread, char* str, size_t length) {
	crs_String* string = newString(thread, length);
	copyString(string, str, length);

	return crsG_add(thread, string, CRS_TYPE_STRING);
}

crs_String*
crsS_new(crs_Thread* thread, char* str) {
	return crsS_newl(thread, str, strlen(str));
}

void
crsS_free(crs_Thread* thread, crs_String* string) {
	mem_dealloc(thread,
		string, sizeof(crs_String) + (string->length + 1) * sizeof(char));
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
