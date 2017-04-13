/*
 * Copyright (c) 2011-2017  University of Texas at Austin. All rights reserved.
 *
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * This file is part of PerfExpert.
 *
 * PerfExpert is free software: you can redistribute it and/or modify it under
 * the terms of the The University of Texas at Austin Research License
 *
 * PerfExpert is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.
 *
 * Authors: Antonio Gomez-Iglesias
 *
 * $HEADER$
 */

#ifndef PERFEXPERT_MODULE_IO_DATABASE_H_
#define PERFEXPERT_MODULE_IO_DATABASE_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Modules headers */
#include "io.h"

/* PerfExpert common headers */
#include "common/perfexpert_list.h"

static int database_export(io_function_t *results);
static int database_input(io_function_t *results);
//static int store_function (code_function_t *function);
#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_MODULE_IO_DATABASE_H_ */
