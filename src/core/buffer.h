/*
 * https://github.com/mochji/crescent
 * core/buffer.h
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#ifndef CRS_CORE_BUFFER_H
#define CRS_CORE_BUFFER_H

#include <stdio.h>
#include <stddef.h>

#include "conf.h"
#include "limit.h"

#define CRS_BUF_INITIAL 256
#define CRS_BUF_STREAM  BUFSIZ

typedef struct {
	crs_Thread* thread;
	size_t      size;
	size_t      length;
	char*       buffer;
	char        initial[CRS_BUF_INITIAL];
} crs_Buffer;

void
crsB_init(crs_Thread* thread, crs_Buffer* buffer);

void
crsB_free(crs_Buffer* buffer);

void
crsB_addChar(crs_Buffer* buffer, char c);

void
crsB_addString(crs_Buffer* buffer, char* str, size_t length);

void
crsB_clear(crs_Buffer* buffer);

typedef struct {
	crs_Thread* thread;
	crs_Reader* reader;
	void*       data;
	char        buffer[CRS_BUF_STREAM];
	int         length;
	int         read;
} crs_Stream;

void
crsD_init(crs_Thread* thread, crs_Stream* stream, crs_Reader* reader, void* data);

char
crsD_next(crs_Stream* stream);

#define stream_next(s) \
	((s)->read == (s)->length ? crsD_next(s) : (s)->buffer[(s)->read++])

#endif
