#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "apierr.h"
#include "types.h"

#define HASHTABLE_CAPACITY 0xFFFFFF

typedef struct {
	size_t length;
	size_t size;
	crescent_Object* data;
} crescent_HashTable;

size_t crescent_hash(char* str) {
	size_t hash = 0;
	char   c;

	while (c = str[a++])
		hash = c + (hash << 6) + (hash << 16) - hash;

	return hash;
}
