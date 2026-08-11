/*
 * https://github.com/mochji/crescent
 * types/table.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include <stddef.h>
#include <limits.h>

#include "crescent/conf.h"
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
 * Hashtables utilize a mix of open addressing and chaining. Every key present
 * in the table is either located at its home address or is part of a chain
 * whose root is located at the key's home address. Keys not located at their
 * home address occupy a free spot somewhere else in the table.
 */

#define rootnode(t, h) ((t)->table + ((h) & table_mask(t)))
#define isabsent(n)    ((n)->value.type == CRS_TYPE_NIL)
#define isroot(n)      ((n)->previous == NULL)
#define istail(n)      ((n)->next == NULL)

static int set(crs_Table* table, crs_Object* key, crs_Object* value);

static void setNil(crs_TNode* node, crs_TNode* stop) {
    crs_TNode* previous = NULL;

    while (node < stop) {
        obj_setn(&node->value);
        node->next     = node + 1;
        node->previous = previous;

        previous = node++;
    }

    previous->next = NULL;
}

static void rehash(crs_Table* old, crs_Table* new) {
    crs_TNode* node = old->table;
    crs_TNode* stop = node + table_nodes(old);

    while (node < stop) {
        if (!isabsent(node)) {
            set(new, &node->key, &node->value);
        }

        node++;
    }
}

static void resize(crs_Thread* thread, crs_Table* table, crs_byte nodes) {
    crs_Table  temp;
    crs_TNode* hash;
    size_t     size = (size_t)1 << nodes;

    if (size > CRS_MAX_LENGTH || size > MAX_HSIZE) {
        crsC_error(thread, "table overflow");
    } else if ((hash = mem_vnew(thread, size, crs_TNode)) == NULL) {
        crsM_error(thread);
    }

    temp.nodes = nodes;
    temp.free  = hash;
    temp.table = hash;
    setNil(hash, hash + size);
    rehash(table, &temp);

    mem_vfree(thread, table->table, table_nodes(table));
    table->nodes = nodes;
    table->free  = temp.free;
    table->table = hash;
}

static void free_remove(crs_Table* table, crs_TNode* node) {
    crs_TNode* next     = node->next;
    crs_TNode* previous = node->previous;

    if (previous == NULL) {
        table->free = next;
    } else {
        node->previous->next = next;
    }

    if (next != NULL) {
        next->previous = previous;
    }
}

static void free_add(crs_Table* table, crs_TNode* node) {
    crs_TNode* next = table->free;
    node->next      = next;
    node->previous  = NULL;
    table->free     = node;

    if (next != NULL) {
        next->previous = node;
    }
}

#define hash_bool(x)    ((unsigned)(x))
#define hash_int(x)     ((unsigned)(x) * 2654435761)
#define hash_float(x)   ((unsigned)(x) * 2654435761)
#define hash_pointer(x) ((unsigned)(crs_uptr)(x))

static unsigned hash_obj(crs_Object* key) {
    switch (key->type) {
        case CRS_TYPE_BOOLEAN:
            return hash_bool(obj_getb(key));
        case CRS_TYPE_INTEGER:
            return hash_int(obj_geti(key));
        case CRS_TYPE_FLOAT:
            return hash_float(obj_getf(key));
        case CRS_TYPE_CFUNCTION:
            return hash_pointer(obj_getc(key));
        case CRS_TYPE_STRING:
            return crsS_hash(obj_gets(key));
        case CRS_TYPE_TABLE:
        case CRS_TYPE_FUNCTION:
        case CRS_TYPE_THREAD:
            return hash_pointer(obj_geth(key));
    }

    return 0;
}

#define SEARCH_FREE     0 /* root position is free                         */
#define SEARCH_OCCUPIED 1 /* must move colliding node before setting root  */
#define SEARCH_EXISTS   2 /* key already exists in table                   */
#define SEARCH_CHAIN    3 /* chain was found for index, but the key wasn't */

static int search(crs_Table* table, crs_Object* key, crs_TNode** location) {
    crs_TNode* node = rootnode(table, hash_obj(key));
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

        if (crsV_equal(key, &node->key)) {
            *location = node;
            return SEARCH_EXISTS;
        }
    } while (!istail(node));

    *location = node;
    return SEARCH_CHAIN;
}

static void promote(crs_TNode* root, crs_TNode* next) {
    if (!istail(next)) {
        next->next->previous = root;
    }

    obj_seto(&root->key, &next->key);
    obj_seto(&root->value, &next->value);
    root->next = next->next;
}

static void removeNode(crs_TNode* node) {
    node->previous->next = node->next;

    if (!istail(node)) {
        node->next->previous = node->previous;
    }
}

static void relocate(crs_TNode* old, crs_TNode* new) {
    /* root nodes are never relocated */
    old->previous->next = new;
    *new                = *old;

    if (!istail(old)) {
        old->next->previous = new;
    }
}

static crs_Object* get(crs_Table* table, crs_Object* key) {
    crs_TNode* node;

    return search(table, key, &node) == SEARCH_EXISTS
        ? &node->value
        : NULL;
}

static void delete(crs_Table* table, crs_Object* key) {
    crs_TNode* node;

    if (search(table, key, &node) != SEARCH_EXISTS) {
        return; /* nothing to delete */
    }

    if (isroot(node)) {
        crs_TNode* next = node->next;

        if (next != NULL) {
            promote(node, next);
            node = next; /* now 'next' is to be freed */
        } /* otherwise, no remaining nodes; chain is deleted */
    } else {
        removeNode(node);
    }

    obj_setn(&node->value);
    free_add(table, node);
}

static int set(crs_Table* table, crs_Object* key, crs_Object* value) {
    if (value->type == CRS_TYPE_NIL) {
        delete(table, key);
        return 1;
    }

    /* new nodes are always tail nodes */
    crs_TNode* node; /* to be previous node */
    crs_TNode* free; /* to be new node      */

    switch (search(table, key, &node)) {
        case SEARCH_EXISTS:
            obj_seto(&node->value, value);
            return 1;
        case SEARCH_CHAIN:
            if ((free = table->free) == NULL) {
                return 0;
            }

            free_remove(table, free);

            /* tail node */
            node->next = free;
            break;
        case SEARCH_OCCUPIED:
            if ((free = table->free) == NULL) {
                return 0;
            }

            free_remove(table, free);
            relocate(node, free);

            /* root node */
            free = node;
            node = NULL;
            break;
        case SEARCH_FREE:
            free_remove(table, node);

            /* root node */
            free = node;
            node = NULL;
            break;
    }

    obj_seto(&free->key, key);
    obj_seto(&free->value, value);
    free->previous = node;
    free->next     = NULL;

    return 1;
}

static void checkKey(crs_Thread* thread, crs_Object* key) {
    switch (key->type) {
        case CRS_TYPE_NIL:
            crsC_error(thread, "table index is nil");
        case CRS_TYPE_FLOAT:
            if (obj_getf(key) != obj_getf(key)) {
                crsC_error(thread, "table index is nan");
            }
    }
}

crs_Table* crsT_new(crs_Thread* thread) {
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
    setNil(hash, hash + 16);

    return crsG_add(thread, table, CRS_TYPE_TABLE);
}

void crsT_free(crs_Thread* thread, crs_Table* table) {
    mem_vfree(thread, table->table, table_nodes(table));
    mem_free(thread, table);
}

/* otherwise, we would return NULL for empty keys */
crs_TNode nilKVP = {.value = {.type = CRS_TYPE_NIL}};

/* used for lexer string table (both the key and value are needed) */
crs_TNode* crsT_find(crs_Table* table, crs_Object* key) {
    crs_TNode* node;

    return search(table, key, &node) == SEARCH_EXISTS
        ? node
        : &nilKVP;
}

crs_Object* crsT_get(crs_Thread* thread, crs_Table* table, crs_Object* key) {
    checkKey(thread, key);
    crs_Object* object = get(table, key);

    return object == NULL
        ? &crsO_nilValue
        : object;
}

void crsT_set(crs_Thread* thread, crs_Table* table, crs_Object* key,
                                  crs_Object* value) {
    checkKey(thread, key);

    if (!set(table, key, value)) {
        resize(thread, table, table->nodes + 1);
        set(table, key, value); /* should always succeed */
    }

    crsG_barrierB(thread, table, key);
    crsG_barrierB(thread, table, value);
}
