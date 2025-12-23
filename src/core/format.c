/*
 * https://github.com/mochji/crescent
 * core/format.c
 *
 * Copyright (C) 2025 mochji
 * MIT License
 */

#include <stddef.h>
#include <ctype.h>

#include "conf.h"
#include "limit.h"

#include "core/format.h"

#define hexdigit(c) (isdigit(c) ? c - '0' : (tolower(c) - 'a') + 10)

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
