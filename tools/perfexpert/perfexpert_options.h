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

/* Structure to handle command line arguments */
static struct argp_option options[] = {
#if HAVE_CODE_TRANSFORMATION
    { 0, 0, 0, 0, "Automatic optimization options:", 1,
    }, {"target", 'm', "TARGET", 0, "Use GNU standard 'make' command to compile"
        " the code (it will run in the current directory)",
    }, {"source", 's', "FILE", 0, "Set the source code file (does not work with"
        " multiple files, choose -m option instead)",
    }, {0, 0, 0, 0,
        "Use CC, CFLAGS, CPPFLAGS and LDFLAGS to set compiler/linker options",
    }, 
#endif

    { 0, 0, 0, 0, "Target program execution options:", 2,
    }, {"after", 'a', "COMMAND", 0,
        "Command to execute after each run of the target program",
    }, {"before", 'b', "COMMAND", 0,
        "Command to execute before each run of the target program",
    }, {"prefix", 'p', "PREFIX", 0, "Add a prefix to the command line, use "
        "double quotes to set arguments with spaces within (e.g. -p \"mpirun -n"
        " 2\")", },

    { 0, 0, 0, 0, "Target program execution on Intel Phi (aka MIC) options:", 3,
    }, {"mic-card", 'C', "NAME", 0,
        "MIC Card where experiments should run on (e.g. -C \"mic0\")",
    }, {"mic-after", 'A', "COMMAND", 0,
        "Command to execute after each run (on MIC) of the target program",
    }, {"mic-before", 'B', "COMMAND", 0,
        "Command to execute before each run (on MIC) of the target program",
    }, {"mic-prefix", 'P', "PREFIX", 0, "Add a prefix to the command line which"
        " will run on the MIC, use double quotes to set arguments with spaces "
        "within (e.g. -P \"mpirun -n 2\")", },

    { 0, 0, 0, 0, "Output formating options:", 4,
    }, {"colorful", 'c', 0, 0, "Enable ANSI colors"
    }, {"verbose", 'v', "LEVEL", OPTION_ARG_OPTIONAL,
        "Enable verbose mode (default: 5, range: 0-10)"
    }, {"order", 'o', "relevance|performance|mixed", 0,
        "Order in which hotspots should be sorted (default: unsorted)",
    }, {"recommendations", 'r', "VALUE", 0,
        "Number of recommendations PerfExpert should provide (default: 3)", },

    { 0, 0, 0, 0, "Other options:", 5,
    }, {"database", 'd', "FILE", 0,
        "Select a recommendation database file different from the default",
    }, {"leave-garbage", 'g', 0, 0,
        "Do not remove temporary directory when finalize",
    }, {"measurement-tool", 't', "hpctoolkit|vtune", 0, "Set the tool that "
        "should be used to collect performance counter values (default: "
        "hpctoolkit)", },

    { 0, 0, 0, 0, "Informational options:", -1, },
    { 0, 'h', 0, OPTION_HIDDEN, "Give a very short usage message" },
    { 0 }
};

static char args_doc[] = "THRESHOLD PROGRAM [PROGRAM ARGUMENTS]";
static char doc[] = "\nPerfExpert -- an easy-to-use automatic performance "
    "diagnosis and optimization\n              tool for HPC applications\n\n"
    "  THRESHOLD           Threshold (relevance % of runtime) to take hotspots "
    "into\n                      consideration (range: between 0 and 1, accepts"
    " fraction)\n"
    "  PROGRAM             Program (binary) to analyze (do not use shell "
    "scripts)\n"
    "  PROGRAM ARGUMENTS   Program arguments, see documentation if any argument"
    "\n                      starts with a dash sign ('-')";

/* Function declarations */
static int parse_env_vars(void);
static error_t parse_options(int key, char *arg, struct argp_state *state);

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_OPTIONS_H_ */
