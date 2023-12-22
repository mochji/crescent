/*
	prefix.h

	See license in crescent.c
*/

#ifndef PREFIX_H
#define PREFIX_H

#include <stdio.h>

#define crescent_apiError(msg) do { fprintf(stderr, "\ninternal error!\n\t%s\n\tin internal function %s\n", msg, __func__); exit(EXIT_FAILURE); } while(0)

#endif
