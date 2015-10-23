/*
 * Copyright (c) 2011-2015  University of Texas at Austin. All rights reserved.
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
 * Authors: Antonio Gomez-Iglesias, Leonardo Fialho and Ashay Rane
 *
 * $HEADER$
 */

#ifdef __cplusplus
extern "C" {
#endif

/* System standard headers */
#include <stdio.h>
#include <stdlib.h>

/* Utility headers */
#include <math.h>
#include <sqlite3.h>

/* Modules headers */
#include "timb.h"
#include "timb_output.h"

/* Tools headers */
#include "tools/perfexpert/perfexpert_types.h"

/* PerfExpert common headers */
#include "common/perfexpert_constants.h"
#include "common/perfexpert_database.h"
#include "common/perfexpert_output.h"

/* prety_print */
#define PRETTY_PRINT(size, symbol) \
    {                              \
        int i = size;              \
        while (i > 0) {            \
            printf("%s", symbol);  \
            i--;                   \
        }                          \
        printf("\n");              \
    }

/* prety_print_bar */
#define PRETTY_PRINT_BAR(size, symbol) \
    {                                  \
        int i = size;                  \
        if (50 < i) {                  \
            i = 49;                    \
            while (i > 0) {            \
                printf("%s", symbol);  \
                i--;                   \
            }                          \
            printf("+");               \
        } else {                       \
            while (i > 0) {            \
                printf("%s", symbol);  \
                i--;                   \
            }                          \
        }                              \
        printf("\n");                  \
    }

/* output_analysis */
int output_analysis(void) {
    char PERCENTAGE[] = "0..........25..........50..........75..........100";
    char *error = NULL, sql[MAX_BUFFER_SIZE];

    /* Get the total cycles */
    bzero(sql, MAX_BUFFER_SIZE);
    if (my_module_globals.measure_mod == HPCTOOLKIT_MOD) {
        sprintf(sql, "SELECT SUM(value) AS total FROM perfexpert_hotspot AS h JOIN "
            "perfexpert_event AS e ON h.id = e.hotspot_id WHERE h.perfexpert_id = "
            "%llu AND e.name = 'PAPI_TOT_CYC';", globals.unique_id);
    }
    if (my_module_globals.measure_mod == VTUNE_MOD) {
        sprintf(sql, "SELECT SUM(value) AS total FROM perfexpert_hotspot AS h JOIN "
            "perfexpert_event AS e ON h.id = e.hotspot_id WHERE h.perfexpert_id = "
            "%llu AND e.name = 'CPU_CLK_UNHALTED_REF_TSC';", globals.unique_id);
    }

    if (SQLITE_OK != sqlite3_exec(globals.db, sql,
        perfexpert_database_get_double, (void *)&my_module_globals.total,
        &error)) {
        OUTPUT(("%s %s", _ERROR("SQL error"), error));
        sqlite3_free(error);
        return PERFEXPERT_ERROR;
    }

    /* Print report header */
    PRETTY_PRINT(79, "=");
    OUTPUT(("%s", _CYAN("Thread balancing")));
    PRETTY_PRINT(79, "=");
    if (0.0 == my_module_globals.total) {
        printf("%s the cycles count is zero, measurement problems?\n\n",
            _BOLDRED("WARNING:"));
    }
    printf("%s this analysis does not take into consideration synchronization "
        "between\n         threads, only the total number of cycles each thread"
        " consumed\n\n", _BOLDRED("WARNING:"));
    printf(" Thread    Cycles     %%    %s\n", PERCENTAGE);

    /* Get thread cycles */
    bzero(sql, MAX_BUFFER_SIZE);
    if (my_module_globals.measure_mod == HPCTOOLKIT_MOD) {
        sprintf(sql, "SELECT h.profile, e.thread_id, SUM(e.value) AS value FROM "
            "perfexpert_hotspot AS h JOIN perfexpert_event AS e ON h.id = "
            "e.hotspot_id WHERE h.perfexpert_id = %llu AND e.name = 'PAPI_TOT_CYC' "
            "GROUP BY e.thread_id ORDER BY e.thread_id ASC;", globals.unique_id);
    }

    if (my_module_globals.measure_mod == VTUNE_MOD) {
        sprintf(sql, "SELECT h.profile, e.thread_id, SUM(e.value) AS value FROM "
            "perfexpert_hotspot AS h JOIN perfexpert_event AS e ON h.id = "
            "e.hotspot_id WHERE h.perfexpert_id = %llu AND e.name = 'CPU_CLK_UNHALTED_REF_TSC' "
            "GROUP BY e.thread_id ORDER BY e.thread_id ASC;", globals.unique_id);
    }

    if (SQLITE_OK != sqlite3_exec(globals.db, sql, output_thread, NULL,
        &error)) {
        OUTPUT(("%s %s", _ERROR("SQL error"), error));
        sqlite3_free(error);
        return PERFEXPERT_ERROR;
    }

    /* Print report footer */
    printf(" -------------------------\n");
    printf("%5d   %10.2g 100.00%% << total cycle count for %s\n",
        my_module_globals.threads, my_module_globals.total,
        _CYAN(globals.program));
    printf("                              with a variance up to %.2f%%\n",
        (100 * ((my_module_globals.maximum - my_module_globals.minimum) /
            my_module_globals.maximum)));
    PRETTY_PRINT(79, "-");
    printf("\n");

    return PERFEXPERT_SUCCESS;
}

/* output_thread */
static inline int output_thread(void *var, int c, char **val, char **names) {

    printf("%5d   %10.2g %6.2f  ", atoi(val[1]), atof(val[2]),
        (atof(val[2]) * 100 / my_module_globals.total));
    PRETTY_PRINT_BAR((int)rint((atof(val[2]) * 100) /
        (my_module_globals.total * 2)), "*");

    if (atof(val[2]) > my_module_globals.maximum) {
        my_module_globals.maximum = atof(val[2]);
    }
    if (atof(val[2]) < my_module_globals.minimum) {
        my_module_globals.minimum = atof(val[2]);
    }
    my_module_globals.threads++;

    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
