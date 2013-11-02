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

#ifndef RECOMMENDER_OPTIONS_H_
#define RECOMMENDER_OPTIONS_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _ARGP_H
#include <argp.h>
#endif

/* PerfExpert common headers */
#include "config.h"

/* Basic package info */
const char *argp_program_version = PACKAGE_STRING;
const char *argp_program_bug_address = PACKAGE_BUGREPORT;

/* Structure to handle command line arguments */
static struct argp_option options[] = {
    { "workdir", 'w', "DIR", 0, "Directory where output files should be created"
        " (default: current directory)", },
    { "uid", 'u', "ID", 0, "Required to locate metrics in database", },
    { "recommendations", 'r', "COUNT", 0,
        "Number of recommendations PerfExpert should provide (default: 3)", },
    { "database", 'd', "FILE", 0, "Select a recommendation database file "
        "different from the default (I hope you know what you are doing)", },
    { "output", 'o', "FILE", 0,
        "Set output file for recommendations (default: STDOUT)" },
    { "transformer", 't', "FILE", 0,
        "Set output file for code transformer" },
    { "colorful", 'c', 0, 0, "Enable ANSI colors" },
    { "verbose", 'v', "LEVEL", 0, "Enable verbose mode (range: 0-10)" },

    { 0, 0, 0, 0, "Informational options:", -1, },
    { 0, 'h', 0, OPTION_HIDDEN, "Give a very short usage message" },
    { 0 }
};

static char args_doc[] = "-u ID";
static char doc[] = "\nPerfExpert -- an easy-to-use automatic performance "
    "diagnosis and optimization\n              tool for HPC applications\n";

/* Function declarations */
static int parse_env_vars(void);
static error_t parse_options(int key, char *arg, struct argp_state *state);

#ifdef __cplusplus
}
#endif

#endif /* RECOMMENDER_OPTIONS_H_ */
