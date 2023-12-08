#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>

#include "../utils/apierr.h"

#define STRING_ALLOC_SIZE     64

#define STRING_MIN_FREE_CHARS 8
#define STRING_MAX_FREE_CHARS 64

typedef struct {
	size_t length;
	size_t size;
	char*  data;
} crescent_String;

void crescent_initString(crescent_String* str) {
	str->length = 0;
	str->size   = STRING_ALLOC_SIZE;
	str->data   = malloc(STRING_ALLOC_SIZE);

	if (!str->data) {
		crescent_apiError("malloc failure");

		exit(EXIT_FAILURE);
	}

	str->data[0] = '\0';
}

void crescent_freeString(crescent_String* str) {
	free(str->data);
	str->data = NULL;
}

void crescent_reallocString(crescent_String* str) {
	if (str->size - str->length < STRING_MIN_FREE_CHARS) {
		str->size += STRING_MAX_FREE_CHARS - STRING_MIN_FREE_CHARS;
		str->data  = realloc(str->data, str->size);

		if (!str->data) {
			crescent_apiError("realloc failure");

			exit(EXIT_FAILURE);
		}
	}

	if (str->size - str->length > STRING_MAX_FREE_CHARS) {
		str->size -= STRING_MAX_FREE_CHARS + STRING_MIN_FREE_CHARS;
		str->data  = realloc(str->data, str->size);

		if (!str->data) {
			crescent_apiError("realloc failure");

			exit(EXIT_FAILURE);
		}
	}
}

void crescent_pushChar(crescent_String* str, char c) {
	crescent_reallocString(str);

	str->data[str->length++] = c;
	str->data[str->length]   = '\0';
}

void crescent_popChar(crescent_String* str) {
	str->data[--str->length] = '\0';

	crescent_reallocString(str);
}

char crescent_getChar(crescent_String* str, size_t index) {
	if (index >= str->length - 1) {
		crescent_apiError("string index out of bounds");

		exit(EXIT_FAILURE);
	}

	return str->data[index];
}

void crescent_setChar(crescent_String* str, char c, size_t index) {
	if (index >= str->length - 1) {
		crescent_apiError("string index out of bounds");

		exit(EXIT_FAILURE);
	}

	str->data[index] = c;
}

void crescent_insertChar(crescent_String* str, char c, size_t index) {
	if (index >= str->length) {
		crescent_apiError("string index out of bounds");

		exit(EXIT_FAILURE);
	}

	str->length++;

	crescent_reallocString(str);

	for (size_t a = index; a < str->length; a++)
		str->data[a + 1] = str->data[a];

	str->data[index] = c;
}

void crescent_pullChar(crescent_String* str, size_t index) {
	if (index >= str->length - 1) {
		crescent_apiError("string index out of bounds");

		exit(EXIT_FAILURE);
	}

	for (size_t a = index; a < str->length; a++)
		str->data[a] = str->data[a + 1];

	str->data[str->length--] = '\0';

	crescent_reallocString(str);
}

void crescent_setString(crescent_String* str, const char* string) {
	if (str->length) {
		for (size_t a = 0; a < str->length; a++)
			crescent_popChar(str);
	}

	size_t a = 0;

	while (string[a])
		crescent_pushChar(str, string[a++]);
}

void crescent_concatString(crescent_String* destination, crescent_String* source) {
	for (size_t a = 0; a < source->length; a++)
		crescent_pushChar(destination, source->data[a]);
}

size_t crescent_compareString(crescent_String* strA, crescent_String* strB) {
	size_t distance, limit;

	if (strA->length > strB->length) {
		distance = strA->length - strB->length;
		limit = strB->length;
	} else {
		distance = strB->length - strA->length;
		limit = strA->length;
	}

	for (size_t a = 0; a < limit; a++) {
		if (strA->data[a] != strB->data[a])
			distance++;
	}

	return distance;
}

crescent_String* crescent_subString(crescent_String* str, size_t start, size_t end) {
	crescent_String* subString;
	crescent_initString(subString);

	if (start >= str->length - 1 || end >= str->length - 1) {
		crescent_apiError("str index out of bounds");

		exit(EXIT_FAILURE);
	}

	if (end > start) {
		crescent_apiError("end cannot be greater than start");

		exit(EXIT_FAILURE);
	}

	for (size_t a = start; a < end; a++)
		crescent_pushChar(subString, str->data[a]);

	return subString;
}

void crescent_lowerString(crescent_String* str) {
	for (size_t a = 0; a < str->length; a++)
		str->data[a] = tolower(str->data[a]);
}

void crescent_upperString(crescent_String* str) {
	for (size_t a = 0; a < str->length; a++)
		str->data[a] = toupper(str->data[a]);
}

void crescent_reverseString(crescent_String* str) {
	char* reversedString;
	reversedString = malloc(str->length + 1);

	for (size_t a = 0; a < str->length; a++)
		reversedString[a] = str->data[str->length - a];

	reversedString[str->length + 1] = '\0';

	crescent_setString(str, reversedString);
}

