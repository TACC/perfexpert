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

#ifndef PERFEXPERT_MODULE_HPCTOOLKIT_OPTIONS_H_
#define PERFEXPERT_MODULE_HPCTOOLKIT_OPTIONS_H_

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
    { 0, 0, 0, 0, "\n[HPCToolkit module options]", 1 },

    { 0, 0, 0, 0, "Target program execution options:", 2 },
    { "inputfile", 'i', "FILE", OPTION_DOC, "Input file to the target program. "
      "Use this option instead of input pipe redirect ('<'). Output pipe ('>') "
      "is not available because PerfExpert already has its standard output file"
    },
    { "inputfile", 'i', "FILE", OPTION_HIDDEN, 0 },
    { "after", 'a', "COMMAND", OPTION_DOC,
      "Command to execute after each run of the target program" },
    { "after", 'a', "COMMAND", OPTION_HIDDEN, 0 },
    { "before", 'b', "COMMAND", OPTION_DOC,
      "Command to execute before each run of the target program" },
    { "before", 'b', "COMMAND", OPTION_HIDDEN, 0 },
    { "prefix", 'p', "PREFIX", OPTION_DOC, "Add a prefix to the command line, "
      "use double quotes to set arguments with spaces within (e.g., \"mpirun -n"
      " 2\")" },
    { "prefix", 'p', "PREFIX", OPTION_HIDDEN, 0 },

    #if HAVE_HPCTOOLKIT_MIC
    { 0, 0, 0, 0, "Target program execution on Intel Phi (MIC) options:", 3 },
    { "mic-card", 'C', "NAME", OPTION_DOC,
      "MIC Card where experiments should run on (e.g., mic0)" },
    { "mic-card", 'C', "NAME", OPTION_HIDDEN, 0 },
    { "mic-after", 'A', "COMMAND", OPTION_DOC,
      "Command to execute after each run (on the MIC) of the target program" },
    { "mic-after", 'A', "COMMAND", OPTION_HIDDEN, 0 },
    { "mic-before", 'B', "COMMAND", OPTION_DOC,
      "Command to execute before each run (on the MIC) of the target program" },
    { "mic-before", 'B', "COMMAND", OPTION_HIDDEN, 0 },
    { "mic-prefix", 'P', "PREFIX", OPTION_DOC, "Add a prefix to the command "
      "line which will run on the MIC, use double quotes to set arguments with "
      "spaces within (e.g., \"mpirun -n 2\")" },
    { "mic-prefix", 'P', "PREFIX", OPTION_HIDDEN, 0 },
    #endif

    { "help", 'h', 0, OPTION_HIDDEN, "Show help message" },
    { 0 }
};

typedef struct arg_options {
    char *prefix;
    char *before;
    char *after;
    char *mic_prefix;
    char *mic_before;
    char *mic_after;
} arg_options_t;

/* Function declarations */
static int parse_env_vars(void);
static error_t parse_options(int key, char *arg, struct argp_state *state);

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_MODULE_HPCTOOLKIT_OPTIONS_H_ */
