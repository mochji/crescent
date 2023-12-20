#include <stddef.h>

#define STRING_MAX_FREE_CHARS 64
#define STRING_MIN_FREE_CHARS 8

#define ARRAY_MAX_FREE_ITEMS  32
#define ARRAY_MIN_FREE_ITEMS  8

struct crescent_String {
	size_t length;
	size_t size;
	char*  data;
};

struct crescent_Array {
	size_t           length;
	size_t           size;
	crescent_Object* data
};

typedef crescent_Integer ssize_t;
typedef crescent_Float   double;
typedef crescent_Boolean unsigned char;
typedef crescent_String  struct crescent_String;
typedef crescent_Array   struct crescent_Array;

void crescent_initString(crescent_String* str) {
	str->length = 0;
	str->size   = STRING_MAX_FREE_CHARS;
	str->data   = malloc(STRING_MAX_FREE_CHARS);

	if (!str->data)
		crescent_apiError("malloc failure");

	for (size_t a = 0; a < STRING_MAX_FREE_CHARS; a++)
		str->data[a] = NULL;

	str->data[0] = '\0';
}

void crescent_freeString(crescent_String* str) {
	if (!str->data)
		crescent_apiError("attempt to free null string");

	free(str->data);
	str->data = NULL;
}

void crescent_reallocString(crescent_String* str) {
	if (str->size - str->length < STRING_MIN_FREE_CHARS) {
		str->size += STRING_MAX_FREE_CHARS - STRING_MIN_FREE_CHARS;
		str->data  = realloc(str->data, str->size);

		if (!str->data)
			crescent_apiError("realloc failure");
	}

	if (str->size - str->length > STRING_MAX_FREE_CHARS) {
		str->size -= STRING_MAX_FREE_CHARS + STRING_MIN_FREE_CHARS;
		str->data  = realloc(str->data, str->size);

		if (!str->data)
			crescent_apiError("realloc failure");
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

void crescent_initArray(crescent_Array* array) {
	array->length = 0;
	array->size   = ARRAY_MAX_FREE_ITEMS;
	array->data   = malloc(ARRAY_MAX_FREE_ITEMS * sizeof(crescent_Object));

	if (!array->data)
		crescent_apiError("malloc failure");

	for (size_t a = 0; a < ARRAY_MAX_FREE_ITEMS; a++)
		array->data[a] = NULL;
}

void crescent_freeArray(crescent_Array* array) {
	if (!array->data)
		crescent_apiError("attempt to free null array");

	free(array->data);
	array->data = NULL;
}

void crescent_pushToArray(crescent_Array* array, crescent_Object object) {
}

void crescent_popFromArray(crescent_Array* array, crescent_Object object) {
}

size_t crescent_hash(char* str) {
	size_t hash = 5381;
	char   c;

	size_t a = 0;

	while (c = str[a++])
		hash = c + (hash << 6) + (hash << 16) - hash;

	return hash;
}
