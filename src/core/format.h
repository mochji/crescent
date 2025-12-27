/*
 * https://github.com/mochji/crescent
 * core/format.h
 *
 * Copyright (C) 2025 mochji
 * MIT License
 */

#ifndef CRS_CORE_FORMAT_H
#define CRS_CORE_FORMAT_H

#include "conf.h"
#include "limit.h"

extern crs_Integer
crsF_toInteger(char* str, int* success);

extern crs_Float
crsF_toFloat(char* str, int* success);

#endif
