/*
 * https://github.com/mochji/crescent
 * libs/baselib.c
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#include <stdio.h>
#include <stddef.h>

#include "crescent/api.h"
#include "crescent/aux.h"

static int base_print(crs_Thread* thread) {
    for (int i = 1; i <= crs_getTop(thread); i++) {
        if (i > 1) {
            printf("\t");
        }

        switch (crs_type(thread, i)) {
            case CRS_TNIL:
                printf("nil");
                break;
            case CRS_TBOOLEAN:
                printf(crs_toBoolean(thread, i) ? "true" : "false");
                break;
            case CRS_TNUMBER: {
                int isInteger;
                crs_toIntegerX(thread, i, &isInteger);

                if (isInteger) {
                    printf(CRS_INTEGER_FMT, crs_toInteger(thread, i));
                } else {
                    printf(CRS_FLOAT_FMT, crs_toFloat(thread, i));
                }

                break;
            }
            case CRS_TSTRING:
                printf("%s", crs_toString(thread, i));
                break;
            case CRS_TTABLE:
            case CRS_TFUNCTION:
            case CRS_TTHREAD:
            case CRS_TUSERDATA:
                printf("%s: %p", crs_name(thread, i), crs_toPointer(thread, i));
                break;
        }
    }

    printf("\n");
    return 0;
}

static int base_error(crs_Thread* thread) {
    crs_error(thread, 1);
    return 0;
}

static void base_warnF(char* msg, void* data) {
    (void)data;
    printf("warning: %s\n", msg);
}

int base_openLib(crs_Thread* thread) {
    crsX_Func lib[] = {
        {"print", &base_print},
        {"error", &base_error},
        {NULL, NULL}
    };

    crs_setWarnF(thread, &base_warnF, NULL);

    crsX_lib(thread, CRS_GLOBALS, lib);
    crs_pushString(thread, CRS_VERSION_STR);
    crsX_setG(thread, "_G", CRS_GLOBALS);
    crsX_setG(thread, "_VERSION", -1);
    crs_pop(thread, 1);

    return 0;
}
