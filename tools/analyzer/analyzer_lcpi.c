/*
 * Copyright (C) 2013 The University of Texas at Austin
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
 * Author: Ashay Rane and Leonardo Fialho
 */

#ifdef __cplusplus
extern "C" {
#endif

/* System standard headers */
#include <string.h>
#include <stdio.h>

/* Utility headers */
#include <matheval.h>

/* PerfExpert headers */
#include "analyzer.h"
#include "perfexpert_alloc.h"
#include "perfexpert_constants.h"
#include "perfexpert_hash.h"
#include "perfexpert_list.h"
#include "perfexpert_md5.h"
#include "perfexpert_output.h"
#include "perfexpert_string.h"

/* lcpi_parse_file */
int lcpi_parse_file(const char *file) {
    char buffer[BUFFER_SIZE];
    lcpi_t *lcpi = NULL;
    FILE *lcpi_FP = NULL;
    int line = 0;

    OUTPUT_VERBOSE((4, "%s", _BLUE("Loading LCPI metrics")));

    if (NULL == (lcpi_FP = fopen(file, "r"))) {
        OUTPUT(("%s (%s)", _ERROR("Error: unable to open LCPI file"), file));
        return PERFEXPERT_ERROR;
    }

    bzero(buffer, BUFFER_SIZE);
    while (NULL != fgets(buffer, BUFFER_SIZE - 1, lcpi_FP)) {
        char *token = NULL;
        int temp = 0;

        line++;

        /* Ignore comments and blank lines */
        if ((0 == strncmp("#", buffer, 1)) ||
            (strspn(buffer, " \t\r\n") == strlen(buffer))) {
            continue;
        }

        /* Remove the end \n character */
        buffer[strlen(buffer) - 1] = '\0';

        /* Replace some characters just to provide a safe expression */
        perfexpert_string_replace_char(buffer, ':', '_');

        /* Allocate and set LCPI data */
        PERFEXPERT_ALLOC(lcpi_t, lcpi, sizeof(lcpi_t));
        lcpi->value = 0.0;

        token = strtok(buffer, "=");
        PERFEXPERT_ALLOC(char, lcpi->name, strlen(token) + 1);
        strcpy(lcpi->name, perfexpert_string_remove_char(token, ' '));
        strcpy(lcpi->name_md5, perfexpert_md5_string(lcpi->name));

        token = strtok(NULL, "=");
        lcpi->expression = evaluator_create(token);
        if (NULL == lcpi->expression) {
            OUTPUT(("%s (%s)", _ERROR("Error: invalid expression at line"),
                line));
            return PERFEXPERT_ERROR;
        }

        /* Add LCPI to global hash of LCPIs */
        perfexpert_hash_add_str(globals.lcpi_by_name, name_md5, lcpi);

        OUTPUT_VERBOSE((7, "   [%s]=[%s] (%s)", lcpi->name,
            evaluator_get_string(lcpi->expression), lcpi->name_md5));
    }

    OUTPUT_VERBOSE((4, "   (%d) %s",
        perfexpert_hash_count_str(globals.lcpi_by_name),
        _MAGENTA("LCPI metric(s) found")));

    return PERFEXPERT_SUCCESS;
}

/* lcpi_compute */
int lcpi_compute(profile_t *profile) {
    lcpi_t *hotspot_lcpi = NULL, *lcpi = NULL;
    procedure_t *hotspot = NULL;
    double *values = NULL;
    char **names = NULL;
    int count = 0;
    int i = 0;

    OUTPUT_VERBOSE((4, "   %s", _CYAN("Calculating LCPI metrics")));

    /* For each hotspot in this profile... */
    hotspot = (procedure_t *)perfexpert_list_get_first(&(profile->hotspots));
    while ((perfexpert_list_item_t *)hotspot != &(profile->hotspots.sentinel)) {
        OUTPUT_VERBOSE((8, "    [%d] %s", hotspot->id, _YELLOW(hotspot->name)));

        /* For each LCPI definition */
        for (lcpi = globals.lcpi_by_name; lcpi != NULL;
            lcpi = lcpi->hh_str.next) {

            /* Copy LCPI definitions into this hotspot LCPI hash */
            PERFEXPERT_ALLOC(lcpi_t, hotspot_lcpi, sizeof(lcpi_t));
            strcpy(hotspot_lcpi->name_md5, lcpi->name_md5);
            hotspot_lcpi->name = lcpi->name;
            hotspot_lcpi->value = lcpi->value;
            hotspot_lcpi->expression = lcpi->expression;

            /* Get the list of variables and their values */
            evaluator_get_variables(hotspot_lcpi->expression, &names, &count);
            if (0 < count) {
                PERFEXPERT_ALLOC(double, values, (sizeof(double*) * count));
                for (i = 0; i < count; i++) {
                    metric_t *metric = NULL;
                    char key_md5[33];

                    strcpy(key_md5, perfexpert_md5_string(names[i]));

                    perfexpert_hash_find_str(hotspot->metrics_by_name,
                        (char *)&key_md5, metric);
                    if (NULL == metric) {
                        perfexpert_hash_find_str(globals.machine_by_name,
                            (char *)&key_md5, metric);
                        if (NULL == metric) {
                            values[i] = 0.0;
                        } else {
                            values[i] = metric->value;
                        }
                    } else {
                        values[i] = metric->value;                    
                    }
                }
 
                /* Evaluate the LCPI expression */
                hotspot_lcpi->value = evaluator_evaluate(
                    hotspot_lcpi->expression, count, names, values);
                PERFEXPERT_DEALLOC(values);
            }

            /* Add the LCPI to the hotspot's list of LCPIs */
            perfexpert_hash_add_str(hotspot->lcpi_by_name, name_md5,
                hotspot_lcpi);

            OUTPUT_VERBOSE((10, "       %s=[%g]", hotspot_lcpi->name,
                hotspot_lcpi->value));
        }
        /* Move on */
        hotspot = (procedure_t *)perfexpert_list_get_next(hotspot);
    }
    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
