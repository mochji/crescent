#include <stdlib.h>
#include <stddef.h>

#include "../prefix.h"
#include "../conf.h"

struct
crescent_String {
	size_t length;
	size_t size;
	char*  data;
};

typedef struct crescent_String crescent_String;

crescent_String*
crescent_newString() {
	crescent_String* str;
	str = malloc(sizeof(crescent_String));

	if (!str)
		crescent_apiError("malloc failure");

	str->length = 0;
	str->size   = CRESCENT_STRING_MAX_FREE;
	str->data   = calloc(str->size, sizeof(char));

	if (!str->data)
		crescent_apiError("calloc failure");

	return str;
}

void
crescent_freeString(crescent_String* str) {
	if (!str)
		crescent_apiError("attempt to free NULL string");

	free(str->data);
	free(str);

	str = NULL;
}

void crescent_reallocString(crescent_String* str, size_t newLength) {
	if (str->size - newLength < CRESCENT_STRING_MIN_FREE) {
		str->size += CRESCENT_STRING_MAX_FREE - CRESCENT_STRING_MIN_FREE;
		str->data  = realloc(str->data, str->size);

		if (!str->data)
			crescent_apiError("realloc failure");
	}

	if (str->size - newLength > CRESCENT_STRING_MAX_FREE) {
		str->size -= CRESCENT_STRING_MAX_FREE + CRESCENT_STRING_MIN_FREE;
		str->data  = realloc(str->data, str->size);

		if (!str->data)
			crescent_apiError("realloc failure");
	}

	str->length = newLength;
}
/*
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
	if (index >= str->length - 1)
		crescent_apiError("string index out of bounds");

	return str->data[index];
}

void crescent_setChar(crescent_String* str, char c, size_t index) {
	if (index >= str->length - 1)
		crescent_apiError("string index out of bounds");

	str->data[index] = c;
}

void crescent_insertChar(crescent_String* str, char c, size_t index) {
	if (index >= str->length)
		crescent_apiError("string index out of bounds");

	str->length++;

	crescent_reallocString(str);

	for (size_t a = index; a < str->length; a++)
		str->data[a + 1] = str->data[a];

	str->data[index] = c;
}

void crescent_pullChar(crescent_String* str, size_t index) {
	if (index >= str->length - 1)
		crescent_apiError("string index out of bounds");

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

void crescent_subString(crescent_String* source, crescent_String* destination, size_t start, size_t end) {
	if (start >= str->length - 1 || end >= str->length - 1)
		crescent_apiError("string index out of bounds");

	if (end > start) {
		crescent_apiError("end cannot be greater than start");

		exit(EXIT_FAILURE);
	}

	crescent_initString(destination);

	for (size_t a = start; a < end; a++)
		crescent_pushChar(destination, str->data[a]);

	return subString;
}

void crescent_lowerString(crescent_String* str) {
	for (size_t a = 0; a < str->length; a++) {
		if (str->data[a] >= 97 && str->data[a] <= 122)
			str->data[a] += 32;
	}
}

void crescent_upperString(crescent_String* str) {
	for (size_t a = 0; a < str->length; a++) {
		if (str->data[a] >= 65 && str->data[a] <= 90)
			str->data[a] -= 32;
	}
}

void crescent_reverseString(crescent_String* str) {
	char* reversedString;
	reversedString = malloc(str->length + 1);

	for (size_t a = 0; a < str->length; a++)
		reversedString[a] = str->data[str->length - a];

	reversedString[str->length + 1] = '\0';

	crescent_setString(str, reversedString);
}
*/
