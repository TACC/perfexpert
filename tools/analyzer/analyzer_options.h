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

#ifndef ANALYZER_OPTIONS_H_
#define ANALYZER_OPTIONS_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Structure to handle command line arguments. Try to keep the content of
 * this structure compatible with the parse_cli_params() and show_help().
 */
static struct option long_options[] = {
    {"aggregate",        no_argument,       NULL, 'a'},
    {"colorful",         no_argument,       NULL, 'c'},
    {"help",             no_argument,       NULL, 'h'},
    {"inputfile",        required_argument, NULL, 'i'},
    {"lcpifile",         required_argument, NULL, 'l'},
    {"measurement-tool", required_argument, NULL, 'm'},
    {"machine",          required_argument, NULL, 'M'},
    {"outputfile",       required_argument, NULL, 'o'},
    {"sorting-order",    required_argument, NULL, 'O'},
    {"threshold",        required_argument, NULL, 't'},
    {"thread",           required_argument, NULL, 'T'},
    {"verbose",          required_argument, NULL, 'v'},
    {"workdir",          required_argument, NULL, 'w'},
    {0, 0, 0, 0}
};

/* Function definitions */
static void show_help(void);
static int parse_env_vars(void);

#ifdef __cplusplus
}
#endif

#endif /* ANALYZER_OPTIONS_H_ */
