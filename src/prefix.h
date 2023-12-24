/*
 * prefix.h
 *
 * stuff idrk
 */

#ifndef PREFIX_H
#define PREFIX_H

#include <stdio.h>

#define CRESCENT_VERSION_MAJOR 0
#define CRESCENT_VERSION_MINOR 0
#define CRESCENT_VERSION_PATCH 0

#define CRESCENT_VERSION   \
	CRESCENT_VERSION_MAJOR \
	CRESCENT_VERSION_MINOR \
	CRESCENT_VERSION_PATCH

#define crescent_internalError(msg)      \
	do { fprintf(                        \
		stderr,                          \
		"Crescent: internal panic!\n"    \
		"\t%s\n"                         \
		"\tin internal function %s\n",   \
		msg,                             \
		__func__                         \
	);                                   \
	exit(EXIT_FAILURE); } while (0)

#endif
