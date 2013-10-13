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

#ifdef __cplusplus
extern "C" {
#endif

/* System standard headers */
#include <string.h>
#include <stdio.h>

/* PerfExpert headers */
#include "analyzer.h"
#include "perfexpert_alloc.h"
#include "perfexpert_constants.h"
#include "perfexpert_hash.h"
#include "perfexpert_list.h"
#include "perfexpert_md5.h"
#include "perfexpert_output.h"
#include "perfexpert_string.h"
#include "perfexpert_util.h"

/* machine_parse_file */
int machine_parse_file(const char *file) {
    char buffer[BUFFER_SIZE];
    metric_t *metric = NULL;
    FILE *machine_FP = NULL;
    int line = 0;

    OUTPUT_VERBOSE((4, "%s", _BLUE("Loading machine characterization")));

    if (NULL == (machine_FP = fopen(file, "r"))) {
        OUTPUT(("%s (%s)", _ERROR("Error: unable to open machine file"), file));
        return PERFEXPERT_ERROR;
    }

    bzero(buffer, BUFFER_SIZE);
    while (NULL != fgets(buffer, BUFFER_SIZE - 1, machine_FP)) {
        char *token = NULL;

        line++;

        /* Ignore comments and blank lines and remove the end \n character*/
        if ((0 == strncmp("#", buffer, 1)) ||
            (strspn(buffer, " \t\r\n") == strlen(buffer))) {
            continue;
        }
        buffer[strlen(buffer) - 1] = '\0';

        /* Allocate and set entry data */
        PERFEXPERT_ALLOC(metric_t, metric, sizeof(metric_t));
        metric->id = 0;
        metric->thread = 0;
        metric->mpi_rank = 0;
        metric->experiment = 0;

        token = strtok(buffer, "=");
        PERFEXPERT_ALLOC(char, metric->name, strlen(token) + 1);
        strcpy(metric->name, perfexpert_string_remove_char(token, ' '));
        strcpy(metric->name_md5, perfexpert_md5_string(metric->name));

        token = strtok(NULL, "=");
        metric->value = atof(token);

        /* Add entry to the list and also to the hash */
        perfexpert_hash_add_str(globals.machine_by_name, name_md5, metric);

        OUTPUT_VERBOSE((7, "   [%s]=[%.2f] (%s)", metric->name, metric->value,
            metric->name_md5));
    }

    OUTPUT_VERBOSE((4, "   (%d) %s",
        perfexpert_hash_count_str(globals.machine_by_name),
        _MAGENTA("machine characterization entries found")));

    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
