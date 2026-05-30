/*
 * https://github.com/mochji/crescent
 * types/table.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include <stddef.h>

#include "conf.h"
#include "limit.h"

#include "types/string.h"
#include "core/object.h"
#include "core/memory.h"
#include "core/call.h"
#include "core/gc.h"

#include "types/table.h"

#define MIN_ASIZE 16
#define MAX_ASIZE (SIZE_MAX / sizeof(crs_Object))
#define MAX_HSIZE (SIZE_MAX / sizeof(crs_TNode))

/*
 * TODO:
 * - ephemerons/weak tables
 * - indexing tables with any type
 * - less strict arrays
 */

/*
 * ===========================
 *  arrays
 * ===========================
 */

/*
 * FIXME: THESE ARRAYS SUCK BUT I REALLY DON'T WANNA DEAL WITH THEM RIGHT NOW.
 *        LIKE PLEASE I PROMISE THIS ISN'T WHAT THE ARRAYS ARE ACTUALLY GONNA BE
 *        LIKE.
 */

static int
array_resize(crs_Thread* thread, crs_Table* table, size_t size) {
	if (size > MAX_ASIZE) {
		crsC_error(thread, "array too big");
	} else if (size <= MIN_ASIZE) {
		if (table->size == MIN_ASIZE) {
			return 0;
		}

		size = MIN_ASIZE;
	}

	crs_Object* array = mem_vresize(thread, table->array, size, table->size);

	if (array == NULL) {
		return 1;
	}

	table->size  = size;
	table->array = array;

	return 0;
}

static void
array_check(crs_Thread* thread, crs_Table* table, size_t length) {
	size_t size = table->size;

	if (length <= size / 3) {
		array_resize(thread, table, size >> 1);
	} else if (length > size) {
		if (array_resize(thread, table, size << 1)) {
			crsM_error(thread);
		}
	}

	table->length = length;
}

static crs_Object*
array_get(crs_Thread* thread, crs_Table* table, size_t key) {
	if (key >= table->length) {
		crsC_error(thread, "bad index to array");
	}

	return table->array + key;
}

static void
array_delete(crs_Thread* thread, crs_Table* table, size_t key) {
	size_t length = table->length;

	if (key == length) {
		return; /* appending nil does nothing */
	} else if (key > length) {
		crsC_error(thread, "bad index to array");
	}

	crs_Object* object = table->array + key;
	crs_Object* to     = table->array + length - 1;

	/* shift above elements down */
	while (object < to) {
		obj_seto(object, object + 1);
		object++;
	}

	array_check(thread, table, length - 1);
}

static void
array_set(crs_Thread* thread, crs_Table* table, size_t key, crs_Object* value) {
	if (value->type == CRS_TYPE_NIL) {
		array_delete(thread, table, key);

		return;
	}

	size_t length = table->length;

	if (key == length) {
		array_check(thread, table, length + 1);
	} else if (key > length) {
		crsC_error(thread, "bad index to array");
	}

	obj_seto(table->array + key, value);
}

static void
array_init(crs_Thread* thread, crs_Table* table) {
	crs_Object* array = mem_vnew(thread, MIN_ASIZE, crs_Object);

	if (array == NULL) {
		crsM_error(thread);
	}

	table->size   = MIN_ASIZE;
	table->length = 0;
	table->array  = array;
}

static crs_Object*
array_index(crs_Thread* thread, crs_Table* table, crs_Integer key, crs_Object* value) {
	if (table->array == NULL) {
		array_init(thread, table);
	}

	if (key < 0) {
		key += table->length;

		if (key == -1) {
			key = table->length;
		} else if (key < 0) {
			crsC_error(thread, "bad index to array");
		}
	}

	if (value == NULL) {
		return array_get(thread, table, (size_t)key);
	} else {
		array_set(thread, table, (size_t)key, value);

		return NULL;
	}
}

/*
 * ===========================
 *  hashtables
 * ===========================
 */

/*
 * Hashtables utilize a mix of open addressing and chaining.
 *
 * Colliding nodes are linked together in chains, occupying free spots in the
 * table, the root of which is always located at the index given to it by the
 * hash function (root index). In the case that a new chain is created and the
 * root index is occupied, the colliding node (which itself is part of another
 * chain with a root located elsewhere), must be relocated.
 *
 * A chain exists (and thus keys with the same root index) iff a root node is
 * located at its root index.
 *
 * Even as the load factor approaches 100% (and relocations become more common),
 * the free list maintains good insertion performance.
 */

#define rootnode(t, k) ((t)->table + ((k)->hash & table_mask(t)))
#define isabsent(n)    ((n)->value.type == CRS_TYPE_NIL)
#define isroot(n)      ((n)->previous == NULL)

static void
hash_set(crs_Thread* thread, crs_Table* table, crs_String* key, crs_Object* value);

static void
hash_rehash(crs_Table* old, crs_Table* new) {
	crs_TNode* node = old->table;
	crs_TNode* stop = node + table_nodes(old);

	while (node < stop) {
		if (!isabsent(node)) {
			hash_set(NULL, new, node->key, &node->value);
		}

		node++;
	}
}

static void
hash_setNil(crs_TNode* node, crs_TNode* stop) {
	crs_TNode* previous = NULL;

	while (node < stop) {
		obj_setn(&node->value);
		node->previous = previous;
		node->next     = node + 1;

		previous = node;
		node++;
	}

	previous->next = NULL;
}

static void
hash_resize(crs_Thread* thread, crs_Table* table, crs_byte nodes) {
	crs_Table  temp;
	crs_TNode* vector;
	size_t     size = 1 << nodes;

	if (size > CRS_MAX_LENGTH || size > MAX_HSIZE) {
		crsC_error(thread, "hashtable too big");
	}

	vector = mem_vnew(thread, size, crs_TNode);

	if (vector == NULL) {
		crsM_error(thread);
	}

	temp.nodes = nodes;
	temp.table = vector;
	hash_setNil(vector, vector + size);
	hash_rehash(table, &temp);

	mem_vfree(thread, table->table, table_nodes(table));
	table->nodes = nodes;
	table->table = vector;
	table->free  = vector;
}

static crs_TNode*
hash_findFree(crs_Thread* thread, crs_Table* table) {
	crs_TNode* free = table->free;

	if (free == NULL) {
		hash_resize(thread, table, table->nodes + 1);

		/*
		 * the hashtable layout has changed, so the previous result of
		 * hash_search is now likely outdated
		 */
		return NULL;
	}

	table->free = free->next;

	if (free->next != NULL) {
		free->next->previous = NULL;
	}

	return free;
}

static void
hash_removeFree(crs_Table* table, crs_TNode* node) {
	crs_TNode* previous = node->previous;

	if (previous == NULL) {
		table->free = node->next;
	} else {
		node->previous->next = node->next;
	}

	if (node->next != NULL) {
		node->next->previous = previous;
	}
}

#define SEARCH_FREE     0 /* root position is free                         */
#define SEARCH_OCCUPIED 1 /* must move colliding node before setting root  */
#define SEARCH_EXISTS   2 /* key already exists in table                   */
#define SEARCH_CHAIN    3 /* chain was found for index, but the key wasn't */

#include <stdio.h>

static int
hash_search(crs_Table* table, crs_String* key, crs_TNode** location) {
	crs_TNode* node = rootnode(table, key);
	crs_TNode* next = node;
	*location       = node;

	/*
	 * if the node at the root index is not a root node, then no such chain with
	 * the hash index given by the key exists, and therefore, neither does the
	 * key.
	 */

	if (isabsent(node)) {
		return SEARCH_FREE;
	} else if (!isroot(node)) {
		return SEARCH_OCCUPIED;
	}

	do {
		node = next;
		next = node->next;

		if (crsS_compare(key, node->key)) {
			*location = node;
			return SEARCH_EXISTS;
		}
	} while (next != NULL);

	*location = node;
	return SEARCH_CHAIN;
}

static crs_Object*
hash_get(crs_Table* table, crs_String* key) {
	crs_TNode* node;

	return hash_search(table, key, &node) == SEARCH_EXISTS
		? &node->value
		: NULL;
}

static void
hash_delete(crs_Table* table, crs_String* key) {
	crs_TNode* node;

	if (hash_search(table, key, &node) != SEARCH_EXISTS) {
		return;
	}

	if (isroot(node)) {
		crs_TNode* next = node->next;

		if (next != NULL) {
			/* promote second node to root */
			node->key   = next->key;
			node->value = next->value;
			node->next  = next->next;
			node        = next;
		}
	} else {
		node->previous->next = node->next;
	}

	crs_TNode* free = table->free;

	node->next     = free;
	node->previous = NULL;
	table->free    = node;

	if (free != NULL) {
		free->previous = node;
	}

	obj_setn(&node->value);
}

static void
hash_set(crs_Thread* thread, crs_Table* table, crs_String* key, crs_Object* value) {
	if (value->type == CRS_TYPE_NIL) {
		hash_delete(table, key);

		return;
	}

	crs_TNode* node;
	crs_TNode* free;

	retry:

	switch (hash_search(table, key, &node)) {
		case SEARCH_EXISTS:
			obj_seto(&node->value, value);

			return;
		case SEARCH_CHAIN:
			free = hash_findFree(thread, table);

			if (free == NULL) {
				goto retry;
			}

			node->next = free;

			break;
		case SEARCH_OCCUPIED:
			free = hash_findFree(thread, table);

			if (free == NULL) {
				goto retry;
			}

			node->previous->next = free;
			*free                = *node;

			if (node->next != NULL) {
				node->next->previous = free;
			}

			free = node;
			node = NULL;

			break;
		case SEARCH_FREE:
			hash_removeFree(table, node);

			free = node;
			node = NULL;

			break;
	}

	free->key      = key;
	free->next     = NULL;
	free->previous = node;
	obj_seto(&free->value, value);
}

static void
hash_init(crs_Thread* thread, crs_Table* table) {
	crs_TNode* vector = mem_vnew(thread, 16, crs_TNode);

	if (vector == NULL) {
		crsM_error(thread);
	}

	hash_setNil(vector, vector + 16);

	table->nodes = 4; /* 2^4 = 16 */
	table->free  = vector;
	table->table = vector;
}

static crs_Object*
hash_index(crs_Thread* thread, crs_Table* table, crs_String* key, crs_Object* value) {
	if (table->table == NULL) {
		hash_init(thread, table);
	}

	if (value == NULL) {
		return hash_get(table, key);
	} else {
		hash_set(thread, table, key, value);

		return NULL;
	}
}

/*
 * ===========================
 *  tables
 * ===========================
 */

crs_Table*
crsT_new(crs_Thread* thread) {
	crs_Table* table = mem_new(thread, crs_Table);

	if (table == NULL) {
		crsM_error(thread);
	}

	/*
	 * in the pretty common case that a table is used as either an array or a
	 * hashtable--not both--only when a key is first created as a string or an
	 * integer will the appropriate structure be allocated.
	 */

	table->size   = 0;
	table->length = 0;
	table->nodes  = 0;
	table->array  = NULL;
	table->table  = NULL;
	table->free   = NULL;

	return crsG_add(thread, table, CRS_TYPE_TABLE);
}

void
crsT_free(crs_Thread* thread, crs_Table* table) {
	mem_vfree(thread, table->array, table->size);
	mem_vfree(thread, table->table, table_nodes(table));
	mem_free(thread, table);
}

crs_Object*
crsT_get(crs_Thread* thread, crs_Table* table, crs_Object* key) {
	crs_Object* object;

	switch (key->type) {
		case CRS_TYPE_INTEGER:
			object = array_index(thread, table, obj_geti(key), NULL);

			break;
		case CRS_TYPE_STRING:
			object = hash_index(thread, table, obj_gets(key), NULL);

			break;
		default:
			crsC_errorf(thread, "attempt to index table with %s", crsO_name(key));
	}

	return object == NULL
		? &thread->state->nilValue
		: object;
}

void
crsT_set(crs_Thread* thread, crs_Table* table, crs_Object* key, crs_Object* value) {
	switch (key->type) {
		case CRS_TYPE_INTEGER:
			array_index(thread, table, obj_geti(key), value);

			break;
		case CRS_TYPE_STRING:
			hash_index(thread, table, obj_gets(key), value);

			break;
		default:
			crsC_errorf(thread, "attempt to index table with %s", crsO_name(key));
	}

	crsG_barrierB(thread, table, value);
}
