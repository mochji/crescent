#include <stdlib.h>
#include <stddef.h>

#include "prefix.h"
#include "conf.h"

// https://gist.github.com/phsym/4605704

typedef struct {
	char*                      key;
	void*                      data;
	struct _crescent_TableKVP* next;
} _crescent_TableKVP;

typedef struct {
	size_t              size;
	size_t              length;
	_crescent_TableKVP* data;
} crescent_Table;

static size_t
_crescent_hash(char* key) {
	size_t hash = 5381;

	while (*(key++))
		hash = ((hash << 5) + hash) + (*key);

	return hash;
}

crescent_
