/*
 * https://github.com/mochji/crescent
 * api/aux.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>

#include "crescent/api.h"
#include "crescent/aux.h"

static int loader_str(crs_Thread* thread, void* data, char* buf, int* n) {
    char* str  = *(char**)data;
    int   read = 0;
    char  c;

    if (*str) {
        while ((c = *str) && read < *n) {
            *(buf++) = c;
            read++;
            str++;
        }
    }

    *(char**)data = str;
    *n            = read;

    (void)thread;
    return CRS_OK;
}

static int loader_file(crs_Thread* thread, void* data, char* buf, int* n) {
    FILE* f    = (FILE*)data;
    int   read = (int)fread(buf, sizeof(char), (size_t)*n, f);
    *n         = read;

    (void)thread;
    return ferror(f) ? CRS_ERROR : CRS_OK;
}

static int writer_file(crs_Thread* thread, void* data, char* buf, int n) {
    FILE* f = (FILE*)data;;
    fwrite(buf, sizeof(char), (size_t)n, f);

    (void)thread;
    return ferror(f) ? CRS_ERROR : CRS_OK;
}

CRS_EXPORT int crsX_loadStr(crs_Thread* thread, char* str, char* source) {
    return crs_load(thread, &loader_str, (void*)&str, source);
}

CRS_EXPORT int crsX_loadFile(crs_Thread* thread, char* path, char* source) {
    FILE* f = fopen(path, "rb");
    int   status;

    if (f == NULL) {
        crs_pushString(thread, strerror(errno));
        return CRS_ERROR;
    }

    status = crs_load(thread, &loader_file, (void*)f, source);
    fclose(f);
    return status;
}

CRS_EXPORT int crsX_dumpFile(crs_Thread* thread, int index, char* path) {
    FILE* f = fopen(path, "wb");
    int   status;

    if (f == NULL) {
        crs_pushString(thread, strerror(errno));
        return CRS_ERROR;
    }

    status = crs_dump(thread, index, &writer_file, (void*)f);
    fclose(f);
    return status;
}

CRS_EXPORT void crsX_getG(crs_Thread* thread, char* name) {
    crs_pushString(thread, name);
    crs_get(thread, CRS_GLOBALS, -1);
    crs_remove(thread, -2);
}

CRS_EXPORT void crsX_setG(crs_Thread* thread, char* name, int index) {
    crs_pushString(thread, name);
    crs_set(thread, CRS_GLOBALS, -1, index < 0 ? index - 1 : index);
    crs_pop(thread, 1);
}

CRS_EXPORT void crsX_lib(crs_Thread* thread, int index, crsX_Func* func) {
    if (index < 0) {
        index -= 2;
    }

    while (func->name != NULL) {
        crs_pushString(thread, func->name);
        crs_pushCFunction(thread, func->func);
        crs_set(thread, index, -2, -1);
        crs_pop(thread, 2);
        func++;
    }
}
