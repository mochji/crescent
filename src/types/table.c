/*
 * https://github.com/mochji/crescent
 * types/table.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include <stddef.h>
#include <limits.h>

#include "conf.h"
#include "limit.h"

#include "types/string.h"
#include "core/object.h"
#include "core/memory.h"
#include "core/call.h"
#include "core/gc.h"
#include "vm/vm.h"

#include "types/table.h"

#define MAX_HSIZE (SIZE_MAX / sizeof(crs_TNode))

/*
 * TODO:
 * - "arrays" (optimizing integer keys, really)
 * - ephemerons/weak tables
 */

/*
 * Hashtables utilize a mix of open addressing and chaining.
 *
 * A key's root index is given by its hash modulo the size of the table. A node
 * with a different key may already exist at this postion. If it is a root node
 * (i.e. it it has the same root index and is located there), then the new node
 * is appended to the end of the chain, occupying a free spot somewhere in the
 * table.
 *
 * If the colliding node is not a root node (is therefore part of another chain
 * whose root is located elsewhere), then it must be relocated before the new
 * node can become a root node (as no root for this index exists yet).
 * Therefore, a chain exists (and thus keys with the same root index) iff a root
 * node is located at its root index.
 *
 * Even as the load factor approaches 100%, retrieving a key maintains good
 * performance (only probing as much as direct chaining). Inserting a key
 * doesn't slow significantly, either, as all free nodes are linked together in
 * a chain of their own.
 */

#define rootnode(t, k) ((t)->table + (hash_key(k) & table_mask(t)))
#define isabsent(n)    ((n)->value.type == CRS_TYPE_NIL)
#define isroot(n)      ((n)->previous == NULL)

static void
hash_set(crs_Thread* thread, crs_Table* table, crs_Object* key, crs_Object* value);

static void
hash_rehash(crs_Table* old, crs_Table* new) {
	crs_TNode* node = old->table;
	crs_TNode* stop = node + table_nodes(old);

	while (node < stop) {
		if (!isabsent(node)) {
			hash_set(NULL, new, &node->key, &node->value);
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
		crsC_error(thread, "table overflow");
	}

	vector = mem_vnew(thread, size, crs_TNode);

	if (vector == NULL) {
		crsM_error(thread);
	}

	temp.nodes = nodes;
	temp.table = vector;
	temp.free  = vector;
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
	crs_TNode* next     = node->next;

	if (previous == NULL) {
		table->free = next;
	} else {
		previous->next = next;
	}

	if (next != NULL) {
		next->previous = previous;
	}
}

#define hash_bool(x)    ((unsigned)(x))
#define hash_int(x)     ((unsigned)(x) * 2654435761)
#define hash_float(x)   ((unsigned)(x) * 2654435761) /* FIXME */
#define hash_pointer(x) ((unsigned)((size_t)(x) & ULONG_MAX))

static unsigned
hash_key(crs_Object* key) {
	switch (key->type) {
		case CRS_TYPE_BOOLEAN:
			return hash_bool(obj_getb(key));
		case CRS_TYPE_INTEGER:
			return hash_int(obj_geti(key));
		case CRS_TYPE_FLOAT:
			/* i'll allow it, but i'm curious about the use case of this */
			return hash_float(obj_getf(key));
		case CRS_TYPE_CFUNCTION:
			return hash_pointer(obj_getc(key));
		case CRS_TYPE_STRING:
			return crsS_hash(obj_gets(key));
		case CRS_TYPE_TABLE:
		case CRS_TYPE_THREAD:
			return hash_pointer(obj_geth(key));
	}

	return 0;
}

#define SEARCH_FREE     0 /* root position is free                         */
#define SEARCH_OCCUPIED 1 /* must move colliding node before setting root  */
#define SEARCH_EXISTS   2 /* key already exists in table                   */
#define SEARCH_CHAIN    3 /* chain was found for index, but the key wasn't */

static int
hash_search(crs_Thread* thread, crs_Table* table, crs_Object* key, crs_TNode** location) {
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

		if (crsV_equal(thread, key, &node->key)) {
			*location = node;
			return SEARCH_EXISTS;
		}
	} while (next != NULL);

	*location = node;
	return SEARCH_CHAIN;
}

static crs_Object*
hash_get(crs_Thread* thread, crs_Table* table, crs_Object* key) {
	crs_TNode* node;

	return hash_search(thread, table, key, &node) == SEARCH_EXISTS
		? &node->value
		: NULL;
}

static void
hash_delete(crs_Thread* thread, crs_Table* table, crs_Object* key) {
	crs_TNode* node;

	if (hash_search(thread, table, key, &node) != SEARCH_EXISTS) {
		return;
	}

	if (isroot(node)) {
		crs_TNode* next = node->next;

		if (next != NULL) {
			/* promote second node to root */
			if (next->next != NULL) {
				next->next->previous = node;
			}

			node->key   = next->key;
			node->value = next->value;
			node->next  = next->next;
			node        = next;
		}
	} else {
		node->previous->next = node->next;

		if (node->next != NULL) {
			node->next->previous = node->previous;
		}
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
hash_set(crs_Thread* thread, crs_Table* table, crs_Object* key, crs_Object* value) {
	if (value->type == CRS_TYPE_NIL) {
		hash_delete(thread, table, key);

		return;
	}

	crs_TNode* node;
	crs_TNode* free;

	retry:

	switch (hash_search(thread, table, key, &node)) {
		case SEARCH_EXISTS:
			obj_seto(&node->value, value);

			return;
		case SEARCH_CHAIN:
			if ((free = hash_findFree(thread, table)) == NULL) {
				goto retry;
			}

			node->next = free;

			break;
		case SEARCH_OCCUPIED:
			if ((free = hash_findFree(thread, table)) == NULL) {
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

	free->key      = *key;
	free->next     = NULL;
	free->previous = node;
	obj_seto(&free->value, value);
}

static crs_Object*
hash_index(crs_Thread* thread, crs_Table* table, crs_Object* key, crs_Object* value) {
	switch (key->type) {
		case CRS_TYPE_NIL:
			crsC_error(thread, "attempt to index table with nil");
		case CRS_TYPE_FLOAT:
			if (obj_getf(key) != obj_getf(key)) {
				crsC_error(thread, "attempt to index table with NaN");
			}
	}

	if (value == NULL) {
		return hash_get(thread, table, key);
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
	crs_TNode* hash  = mem_vnew(thread, 16, crs_TNode);

	if (table == NULL || hash == NULL) {
		mem_free(thread, table);
		mem_vfree(thread, hash, 16);

		crsM_error(thread);
	}

	table->nodes = 4;
	table->free  = hash;
	table->table = hash;
	hash_setNil(hash, hash + 16);

	return crsG_add(thread, table, CRS_TYPE_TABLE);
}

void
crsT_free(crs_Thread* thread, crs_Table* table) {
	mem_vfree(thread, table->table, table_nodes(table));
	mem_free(thread, table);
}

crs_Object*
crsT_get(crs_Thread* thread, crs_Table* table, crs_Object* key) {
	crs_Object* object = hash_index(thread, table, key, NULL);

	return object == NULL
		? &thread->state->nilValue
		: object;
}

void
crsT_set(crs_Thread* thread, crs_Table* table, crs_Object* key, crs_Object* value) {
	hash_index(thread, table, key, value);
	crsG_barrierB(thread, table, key);
	crsG_barrierB(thread, table, value);
}
