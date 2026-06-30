/*
 * https://github.com/mochji/crescent
 * core/format.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include <stddef.h>
#include <string.h>
#include <ctype.h>

#include "conf.h"
#include "limit.h"

#include "types/string.h"
#include "core/call.h"
#include "core/buffer.h"

#include "core/format.h"

#define hexdigit(c) (isdigit(c) ? c - '0' : (tolower(c) - 'a') + 10)

crs_String*
crsF_format(crs_Thread* thread, char* format, ...) {
	crs_String* string;
	va_list     args;

	va_start(args, format);
	string = crsF_vformat(thread, format, args);
	va_end(args);

	return string;
}

static void*
createString(crs_Thread* thread, void* data) {
	crs_Buffer* buffer = data;

	return crsS_newl(thread, buffer->buffer, buffer->length);
}

crs_String*
crsF_vformat(crs_Thread* thread, char* format, va_list args) {
	crs_Buffer buffer;
	char*      next;
	char       numBuffer[CRS_MAX_FMTNUM];

	crsB_init(thread, &buffer);

	while ((next = strchr(format, '%')) != NULL) {
		crsB_addString(&buffer, format, next - format);

		switch (*(next + 1)) {
			case 'c':
				crsB_addChar(&buffer, va_arg(args, int));

				break;
			case 's': {
				char* str = va_arg(args, char*);
				crsB_addString(&buffer, str, strlen(str));

				break;
			}
			case 'd':
				crsB_addString(&buffer, numBuffer, fmt_int(
					numBuffer, CRS_MAX_FMTNUM, va_arg(args, int)));

				break;
			case 'p':
				crsB_addString(&buffer, numBuffer, fmt_pointer(
					numBuffer, CRS_MAX_FMTNUM, va_arg(args, void*)));

				break;
			case 'I':
				crsB_addString(&buffer, numBuffer, fmt_integer(
					numBuffer, CRS_MAX_FMTNUM, va_arg(args, crs_Integer)));

				break;
			case 'F':
				crsB_addString(&buffer, numBuffer, fmt_float(
					numBuffer, CRS_MAX_FMTNUM, va_arg(args, crs_Float)));

				break;
			case '%':
				crsB_addChar(&buffer, '%');

				break;
			case '\0':
				next--;

				break;
		}

		format = next + 2;
	}

	crs_String* string;
	int         status;

	crsB_addString(&buffer, format, strlen(format));
	status = crsC_try(thread, &createString, &buffer, (void**)&string);
	crsB_free(&buffer);

	if (status != CRS_STATUS_OK) {
		/* error is already in error register */
		crsC_throw(thread, status);
	}

	return string;
}

int
crsF_toInteger(char* str, crs_Integer* value) {
	char*       end;
	crs_Integer result = fmt_tointeger(str, &end);

	if (value != NULL) {
		*value = result;
	}

	return end != str;
}

int
crsF_toFloat(char* str, crs_Float* value) {
	char*     end;
	crs_Float result = fmt_tofloat(str, &end);

	if (value != NULL) {
		*value = result;
	}

	return end != str;
}
