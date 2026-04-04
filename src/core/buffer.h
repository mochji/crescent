/*
 * https://github.com/mochji/crescent
 * core/buffer.h
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#ifndef CRS_CORE_BUFFER_H
#define CRS_CORE_BUFFER_H

#include <stddef.h>

#include "conf.h"
#include "limit.h"

#define CRS_BUF_INITIAL 256

struct
crs_Buffer {
	crs_Thread* thread;
	size_t      size;
	size_t      length;
	char*       buffer;
	char        initial[CRS_BUF_INITIAL];
};

typedef struct crs_Buffer crs_Buffer;

void
crsB_init(crs_Thread* thread, crs_Buffer* buffer);

void
crsB_free(crs_Buffer* buffer);

void
crsB_addChar(crs_Buffer* buffer, char c);

void
crsB_addString(crs_Buffer* buffer, char* str, size_t length);

#endif
