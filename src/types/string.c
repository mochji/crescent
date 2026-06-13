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

#define MAX_LENGTH ((SIZE_MAX - sizeof(crs_String)) / sizeof(char) - 1)

/* djb2 */
static unsigned
hashString(char* str, size_t length) {
	unsigned hash = 5381;

	while (length--) {
		hash = ((hash << 5) + hash) ^ *str++;
	}

	return hash;
}

crs_String*
crsS_newl(crs_Thread* thread, char* str, size_t length) {
	if (length > CRS_MAX_LENGTH || length > MAX_LENGTH) {
		crsC_error(thread, "string too big");
	}

	crs_String* string = mem_alloc(thread,
		sizeof(crs_String) + (length + 1) * sizeof(char));

	if (string == NULL) {
		crsM_error(thread);
	}

	string->length = length;
	string->hashed = 0;
	string->hash   = 0;
	memcpy(string->contents, str, length * sizeof(char));

	return crsG_add(thread, string, CRS_TYPE_STRING);
}

crs_String*
crsS_new(crs_Thread* thread, char* str) {
	size_t length = strlen(str);

	if (length > 64) {
		return crsS_newl(thread, str, length);
	}

	crs_State* state = thread->state;
	unsigned   hash  = hashString(str, length);
	unsigned   key   = hash % CRS_STRCACHE_SIZE;

	/* search string cache */
	for (int i = 0; i < CRS_STRCACHE_BUCKETS; i++) {
		crs_String* string = state->strings[i][key];

		if (string == NULL) {
			break; /* end of list */
		} else if (!strcmp(str, string->contents)) {
			return string;
		}
	}

	/* string wasn't found in cache */
	crs_String* string = crsS_newl(thread, str, length);
	string->hashed     = 1;
	string->hash       = hash;

	/* add to string cache */
	for (int i = 0; i < CRS_STRCACHE_BUCKETS - 1; i++) {
		state->strings[i + 1][key] = state->strings[i][key];
	}

	state->strings[0][key] = string;

	return string;
}

void
crsS_free(crs_Thread* thread, crs_String* string) {
	mem_dealloc(thread,
		string, sizeof(crs_String) + (string->length + 1) * sizeof(char));
}

int
crsS_equal(crs_String* stringA, crs_String* stringB) {
	if (stringA == stringB) {
		return 1;
	}

	if (stringA->length != stringB->length) {
		return 0;
	}

	return strcmp(stringA->contents, stringB->contents) == 0;
}

unsigned
crsS_hash(crs_String* string) {
	if (!string->hashed) {
		unsigned hash = hashString(string->contents, string->length);

		string->hashed = 1;
		string->hash   = hash;

		return hash;
	} else {
		return string->hash;
	}
}

void
crsS_clearCache(crs_State* state) {
	crs_String* alive[CRS_STRCACHE_BUCKETS];

	for (int j = 0; j < CRS_STRCACHE_SIZE; j++) {
		int count = 0;

		for (int i = 0; i < CRS_STRCACHE_BUCKETS; i++) {
			crs_String* string   = state->strings[i][j];
			state->strings[i][j] = NULL;

			if (string == NULL) {
				break; /* end of list */
			} else if (gc_isblack(obj_toheader(string))) {
				alive[count++] = string;
			}
		}

		for (int i = 0; i < count; i++) {
			state->strings[i][j] = alive[i];
		}
	}
}
