/*
 * Copyright (c) 2011-2016  University of Texas at Austin. All rights reserved.
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

#ifndef PERFEXPERT_MODULE_MACPO_OPTIONS_H_
#define PERFEXPERT_MODULE_MACPO_OPTIONS_H_

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
    { 0, 0, 0, 0, "\n[MACPO module options]", 1 },

    { "return-code", 0, 0, OPTION_DOC, "Ignore the target return code" },
    { "after=COMMAND", 0, 0, OPTION_DOC,
      "Command to execute after each run of the target program" },
    { "before=COMMAND", 0, 0, OPTION_DOC,
      "Command to execute before each run of the target program" },
    { "main=FILENAME", 0, 0, OPTION_DOC,
      "Filename where the find the entry point to the program (main function)" },
    { "prefix=PREFIX", 0, 0, OPTION_DOC, "Add a prefix to the command line, "
      "use double quotes to set arguments with spaces within (e.g., \"mpirun -n"
      " 2\")" },

    { "return-code", 'r', 0, OPTION_HIDDEN, 0 },
    { "after", 'a', "COMMAND", OPTION_HIDDEN, 0 },
    { "before", 'b', "COMMAND", OPTION_HIDDEN, 0 },
    { "main", 'm', "FILENAME", OPTION_HIDDEN, 0 },
    { "prefix", 'p', "PREFIX", OPTION_HIDDEN, 0 },

    { "help", 'h', 0, OPTION_HIDDEN, "Show help message" },

    { 0 }
};

typedef struct arg_options {
    char *prefix;
    char *before;
    char *after;
    char *mainsrc;
} arg_options_t;

/* Function declarations */
static int parse_env_vars(void);
static error_t parse_options(int key, char *arg, struct argp_state *state);

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_MODULE_MACPO_OPTIONS_H_ */
