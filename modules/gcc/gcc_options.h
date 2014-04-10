/*
 * Copyright (c) 2011-2013  University of Texas at Austin. All rights reserved.
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
 * Authors: Leonardo Fialho and Ashay Rane
 *
 * $HEADER$
 */

#ifndef PERFEXPERT_MODULE_GCC_OPTIONS_H_
#define PERFEXPERT_MODULE_GCC_OPTIONS_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _ARGP_H
#include <argp.h>
#endif

/* PerfExpert common headers */
#include "config.h"

/* Structure to handle command line arguments */
static struct argp_option options[] = {
    { 0, 0, 0, 0, "\n[GCC module options]", 1 },

    { 0, 0, 0, 0, "General options:", 2, },
    { "source", 's', "VALUE", OPTION_DOC, "Source file (single files only" },
    { "source", 's', "VALUE", OPTION_HIDDEN, 0 },
    { "fullpath", 'p', "VALUE", OPTION_DOC, "Compiler full path" },
    { "fullpath", 'p', "VALUE", OPTION_HIDDEN, 0 },

    { "help", 'h', 0, OPTION_HIDDEN, "Show help message" },
    { 0 }
};

/* Function declarations */
static int parse_env_vars(void);
static error_t parse_options(int key, char *arg, struct argp_state *state);

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_MODULE_GCC_OPTIONS_H_ */
