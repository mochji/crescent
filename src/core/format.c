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

crs_String*
crsF_vformat(crs_Thread* thread, char* format, va_list args) {
	crs_Buffer buffer;
	char*      next;
	char       numBuffer[CRS_MAX_FMTNUM];

	crsB_init(thread, &buffer);

	while ((next = strchr(format, '%')) != NULL) {
		crsB_addString(&buffer, format, next - format);

		switch (*(next + 1)) {
			case 's': {
				char* str = va_arg(args, char*);
				crsB_addString(&buffer, str, strlen(str));

				break;
			}
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
				crsB_addChar(&buffer, '%'); break;
		}

		format = next + 2;
	}

	crsB_addString(&buffer, format, strlen(format));
	crs_String* string = crsS_fromBuffer(&buffer);
	crsB_free(&buffer);

	return string;
}

static int
bToInteger(char* str, crs_Integer* result) {
	crs_Integer value = 0;

	int  success = 1;
	char c;

	while ((c = *(str++))) {
		if (c != '0' && c != '1') {
			value   = 0;
			success = 0;

			break;
		}

		value *= 2;
		value += c - '0';
	}

	if (result != NULL) {
		*result = value;
	}

	return success;
}

static int
dToInteger(char* str, crs_Integer* result) {
	crs_Integer value = 0;

	int  success = 1;
	char c;

	while ((c = *(str++))) {
		if (!isdigit(c)) {
			value   = 0;
			success = 0;

			break;
		}

		value *= 10;
		value += c - '0';
	}

	if (result != NULL) {
		*result = value;
	}

	return success;
}

static int
xToInteger(char* str, crs_Integer* result) {
	crs_Integer value = 0;

	int  success = 1;
	char c;

	while ((c = *(str++))) {
		if (!isxdigit(c)) {
			value   = 0;
			success = 0;

			break;
		}

		value *= 16;
		value += hexdigit(c);
	}

	if (result != NULL) {
		*result = value;
	}

	return success;
}

crs_Integer
crsF_toInteger(char* str, int* success) {
	crs_Integer value;
	int         negative = 0;

	if (str[0] == '-') {
		negative = 1;
		str     += 1;
	}

	int successful;

	if (str[0] == '0' && tolower(str[1]) == 'b') {
		successful = bToInteger(str + 2, &value);
	} else if (str[0] == '0' && tolower(str[1]) == 'x') {
		successful = xToInteger(str + 2, &value);
	} else {
		successful = dToInteger(str, &value);
	}

	if (negative && successful) {
		value = -value;
	}

	if (success != NULL) {
		*success = successful;
	}

	return value;
}

/* TODO: support for exponents (e12, e-6, E+50, you get it) */

crs_Float
crsF_toFloat(char* str, int* success) {
	crs_Float value    = 0;
	int       negative = 0;
	int       afterDot = 0;
	crs_Float scale    = 0.1;

	if (str[0] == '-') {
		negative = 1;
		str     += 1;
	} else if (str[0] == '+') {
		str += 1;
	}

	int  successful = 1;
	char c;

	while ((c = *(str++))) {
		if (c == '.' && !afterDot) {
			afterDot = 1;

			continue;
		}

		if (!isdigit(c)) {
			value      = 0;
			negative   = 0;
			successful = 0;

			break;
		}

		if (afterDot) {
			value += (c - '0') * scale;
			scale /= 10;
		} else {
			value *= 10;
			value += c - '0';
		}
	}

	if (negative) {
		value = -value;
	}

	if (success != NULL) {
		*success = successful;
	}

	return value;
}
