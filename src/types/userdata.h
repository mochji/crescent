/*
 * https://github.com/mochji/crescent
 * types/userdata.h
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#ifndef CRS_TYPES_USERDATA_H
#define CRS_TYPES_USERDATA_H

#include <stddef.h>

#include "crescent/conf.h"
#include "limit.h"

#include "core/object.h"

crs_UData* crsU_new(crs_Thread* thread, size_t size);
void       crsU_free(crs_Thread* thread, crs_UData* udata);

#endif
