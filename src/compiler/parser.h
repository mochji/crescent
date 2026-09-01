/*
 * https://github.com/mochji/crescent
 * compiler/parser.h
 *
 * Copyright (C) 2026 mochji
 * MIT License
 */

#ifndef CRS_COMPILER_PARSER_H
#define CRS_COMPILER_PARSER_H

#include "crescent/conf.h"
#include "limit.h"

#include "core/object.h"
#include "core/buffer.h"

crs_Function* crsP_compile(crs_Stream* stream);

#endif
