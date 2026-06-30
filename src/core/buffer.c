/*
 * https://github.com/mochji/crescent
 * core/buffer.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "conf.h"
#include "limit.h"

#include "core/memory.h"
#include "core/call.h"

#include "core/buffer.h"

static void
growBuffer(crs_Buffer* buffer, size_t needed) {
	char*  vector = buffer->buffer;
	size_t size   = buffer->size;

	while (size < needed) {
		size = size > (SIZE_MAX >> 1)
			? SIZE_MAX
			: size << 1;
	}

	if (vector == buffer->initial) {
		vector = mem_vnew(buffer->thread, size, char);

		if (vector != NULL) {
			memcpy(vector, buffer->initial, buffer->length);
		}
	} else {
		vector = mem_vresize(buffer->thread, vector, size, buffer->size);
	}

	if (vector == NULL) {
		crsB_free(buffer);
		crsM_error(buffer->thread);
	}

	buffer->size   = size;
	buffer->buffer = vector;
}

static void
checkBuffer(crs_Buffer* buffer, size_t length) {
	if (length > SIZE_MAX - buffer->length) {
		crsB_free(buffer);
		crsC_error(buffer->thread, "buffer overflow");
	}

	if (buffer->length + length > buffer->size) {
		growBuffer(buffer, buffer->length + length);
	}
}

void
crsB_init(crs_Thread* thread, crs_Buffer* buffer) {
	buffer->thread = thread;
	buffer->size   = CRS_BUF_INITIAL;
	buffer->length = 0;
	buffer->buffer = buffer->initial;
}

void
crsB_free(crs_Buffer* buffer) {
	if (buffer == NULL) {
		return;
	}

	if (buffer->buffer != buffer->initial) {
		mem_vfree(buffer->thread, buffer->buffer, buffer->size);
		buffer->buffer = NULL;
	}
}

void
crsB_addChar(crs_Buffer* buffer, char c) {
	checkBuffer(buffer, 1);
	buffer->buffer[buffer->length++] = c;
}

void
crsB_addString(crs_Buffer* buffer, char* str, size_t length) {
	checkBuffer(buffer, length);

	memcpy(buffer->buffer + buffer->length, str, length);
	buffer->length += length;
}

void
crsB_clear(crs_Buffer* buffer) {
	buffer->length = 0;
}

void
crsD_init(crs_Thread* thread, crs_Stream* stream, crs_Reader* reader, void* data) {
	stream->thread = thread;
	stream->reader = reader;
	stream->data   = data;
	stream->length = 0;
	stream->read   = 0;
}

char
crsD_next(crs_Stream* stream) {
	int read = stream->reader(stream->thread,
		stream->data, stream->buffer, CRS_BUF_STREAM);

	if (read < CRS_BUF_STREAM) {
		stream->buffer[read++] = EOF;
	}

	stream->length = read;
	stream->read   = 1;

	return stream->buffer[0];
}
