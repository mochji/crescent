#ifndef PREFIX_H
#define PREFIX_H

#include <stdio.h>

#define crescent_internalError(msg)      \
	fprintf(                             \
		stderr,                          \
		"Crescent: internal panic!\n"    \
		"\t%s\n"                         \
		"\tin internal function %s\n",   \
		msg,                             \
		__func                           \
	);

#endif
