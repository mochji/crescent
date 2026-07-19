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

#include "crescent/conf.h"
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
crsR_init(crs_Thread* thread, crs_Stream* stream, crs_Reader* reader, void* data) {
	stream->thread = thread;
	stream->reader = reader;
	stream->data   = data;
	stream->length = 0;
	stream->read   = 0;
}

int
crsR_fill(crs_Stream* stream) {
	int read = stream->reader(stream->thread,
		stream->data, stream->buffer, CRS_BUF_STREAM);

	stream->length = read;
	stream->read   = 0;

	return read;
}

size_t
crsR_read(crs_Stream* stream, char* buffer, size_t count) {
	size_t read = 0;

	while (count) {
		if (stream->read == stream->length && !crsR_fill(stream)) {
			break; /* nothing more to read */
		}

		size_t left = stream->length - stream->read;
		int    copy = count > left ? left : count;

		memcpy(buffer, stream->buffer, copy);
		count        -= copy;
		buffer       += copy;
		stream->read += copy;
	}

	return read;
}

int
crsR_next(crs_Stream* stream) {
	if (stream->read == stream->length && !crsR_fill(stream)) {
		return CRS_EOS;
	}

	return stream->buffer[stream->read++];
}

void
crsW_init(crs_Thread* thread, crs_Dump* dump, crs_Writer* writer, void* data) {
	dump->thread  = thread;
	dump->writer  = writer;
	dump->data    = data;
	dump->written = 0;
}

void
crsW_flush(crs_Dump* dump) {
	if (!dump->written) {
		return;
	}

	dump->writer(dump->thread, dump->data, dump->buffer, dump->written);
	dump->written = 0;
}

void
crsW_write(crs_Dump* dump, char* buffer, size_t length) {
	while (length) {
		if (dump->written == CRS_BUF_STREAM) {
			crsW_flush(dump);
		}

		size_t space = CRS_BUF_STREAM - dump->written;
		int    copy  = length > space ? space : length;

		memcpy(dump->buffer + dump->written, buffer, copy);
		buffer        += copy;
		dump->written += copy;
		length        -= copy;
	}
}
