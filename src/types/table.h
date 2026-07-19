/*
 * https://github.com/mochji/crescent
 * types/table.h
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#ifndef CRS_TYPES_TABLE_H
#define CRS_TYPES_TABLE_H

#include "crescent/conf.h"
#include "limit.h"

#include "core/object.h"

#define table_nodes(t) ((size_t)1 << (t)->nodes)
#define table_mask(t)  (table_nodes(t) - 1)

crs_Table*
crsT_new(crs_Thread* thread);

void
crsT_free(crs_Thread* thread, crs_Table* table);

crs_TNode*
crsT_find(crs_Table* table, crs_Object* key);

crs_Object*
crsT_get(crs_Thread* thread, crs_Table* table, crs_Object* key);

void
crsT_set(crs_Thread* thread, crs_Table* table, crs_Object* key, crs_Object* value);

#endif
