#ifndef TYPES_H
#define TYPES_H

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

typedef enum {
	CRESCENT_INTEGER = 0,
	CRESCENT_FLOAT,
	CRESCENT_BOOLEAN,
	CRESCENT_STRING,
	CRESCENT_ARRAY,
} crescent_Type;

typedef union {
	crescent_Integer i;
	crescent_Float   f;
	crescent_Boolean b;
	crescent_String  s;
	crescent_Array   a;
} crescent_Value;

typedef struct {
	crescent_Value value;
	crescent_Type  type;
} crescent_Object;

void crescent_initString(crescent_String* str);
void crescent_freeString(crescent_String* str);

void crescent_reallocString(crescent_String* str);

void crescent_pushChar(crescent_String* str, char c);
void crescent_popChar(crescent_String* str);

char crescent_getChar(crescent_String* str, size_t index);
void crescent_setChar(crescent_String* str, char c, size_t index);

void crescent_insertChar(crescent_String* str, char c, size_t index);
void crescent_pullChar(crescent_String* str, size_t index);

void crescent_setString(crescent_String* str, const char* string);

void crescent_concatString(crescent_String* destination, crescent_String* source);
size_t crescent_compareString(crescent_String* strA, crescent_String* strB);
void crescent_subString(crescent_String* source, crescent_String* destination, size_t start, size_t end);

void crescent_lowerString(crescent_String* str);
void crescent_upperString(crescent_String* str);
void crescent_reverseString(crescent_String* str);

void crescent_initArray(crescent_Array* array);
void crescent_freeArray(crescent_Array* array);

void crescent_pushToArray(crescent_Array* array, crescent_Object object);
void crescent_popFromArray(crescent_Array* array, crescent_Object object);

size_t crescent_hash(char* str);

#endif
