/*
 * Copyright (c) 2013  University of Texas at Austin. All rights reserved.
 *
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * This file is part of PerfExpert.
 *
 * PerfExpert is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * PerfExpert is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with PerfExpert. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Leonardo Fialho
 *
 * $HEADER$
 */

#ifndef PERFEXPERT_OPTIONS_H_
#define PERFEXPERT_OPTIONS_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _ARGP_H
#include <argp.h>
#endif

/* PerfExpert headers */
#include "config.h"

/* Basic package info */
const char *argp_program_version = PACKAGE_STRING;
const char *argp_program_bug_address = PACKAGE_BUGREPORT;

/* Structure to handle command line arguments. Try to keep the content of
 * this structure compatible with the parse_cli_params() and show_help().
 */
static struct argp_option options[] = {
    {   "after", 'a', "COMMAND", 0,
        "Command to execute after each target program run",
    }, {
        "knc-after", 'A', "COMMAND", 0,
        "Command to execute after each target program run on the KNC card",
    }, {
        "before", 'b', "COMMAND", 0,
        "Command to execute before each target program run",
    }, {
        "knc-before", 'B', "COMMAND", 0,
        "Command to execute before each target program run on the KNC card",
    }, {
        "colorful", 'c', 0, 0,
        "Enable colors on verbose mode"
    }, {
        "database", 'd', "FILE", 0,
        "Select the recommendation database file",
    }, {
        "leave-garbage", 'g', 0, 0,
        "Don't remove work directory after finalize",
    }, {
        0, 'h', 0, OPTION_HIDDEN,
        "Give a very short usage message"
    }, {
        "knc", 'k', "CARD_ID", 0,
        "KNC Card where experiments should run on",
    },
#if HAVE_CODE_TRANSFORMATION
    {   "target", 'm', "TARGET", 0,
        "Use GNU standard 'make' command to compile the code (it requires the \
source code available in current directory)",
    },
#endif
    {   "order", 'o', "relevance|performance|mixed", 0,
        "Order in which hotspots should be sorted (default: none)",
    }, {
        "prefix", 'p', "PREFIX", 0,
        "Add a prefix to the command line (e.g. \"mpirun\"). Use double quotes \
to specify arguments with spaces within (e.g. -p \"mpirun -n 2\"). Use \
semicolon (';') to run multiple commands",
    }, {
        "knc-prefix", 'P', "PREFIX", 0,
        "Add a prefix to the command line which will run on the KNC (e.g. \
\"mpirun\"). Use double quotes to specify arguments with spaces within (e.g. \
-p \"mpirun -n 2\"). Use semicolon (';') to run multiple commands",
    }, {
        "recommendations", 'r', "INTEGER", 0,
        "Number of recommendations PerfExpert should provide",
    },
#if HAVE_CODE_TRANSFORMATION
    {   "source", 's', "SOURCE_FILE", 0,
        "Specify the source code file (if your source code has more than one \
file please use a Makefile and choose -m option it also enables the automatic \
optimization option -a)",
    },
#endif
    {   "measurement-tool", 't', "hpctoolkit|vtune", 0,
        "Set the tool to be used to collect performance counter values",
    }, {
        "verbose", 'v', "LEVEL", OPTION_ARG_OPTIONAL,
        "Enable verbose mode (default: 5, range: 0-10)"
    }, {
        0, 0, 0, OPTION_DOC,
        "Use CC, CFLAGS and LDFLAGS to select compiler and compilation/linkage \
flags"
    }, { 0 }
};

static char args_doc[] = "<threshold> <target_program> [program_arguments]";
static char doc[] = "\nPerfExpert -- an easy-to-use automatic performance \
diagnosis and optimization tool for HPC applications.\n\nIf the flags -m and \
-s are not available, recompile PerfExpert enabling ROSE Compiler support.";

//     output_format(w.ws_col-1, 20, 0, "Define the relevance (in %% of runtime) o\
// f code fragments PerfExpert should take into consideration (> 0 and <= 1)\n");

/* Function declarations */
static int parse_env_vars(void);
static error_t parse_options(int key, char *arg, struct argp_state *state);

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_OPTIONS_H_ */
