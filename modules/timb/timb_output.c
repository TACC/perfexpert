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
#include "common/perfexpert_alloc.h"

/* prety_print */
#define PRETTY_PRINT(size, symbol) \
    {                              \
        int i = size;              \
        while (i > 0) {            \
            printf("%s", symbol);  \
            fprintf(report_FP, "%s", symbol); \
            i--;                   \
        }                          \
        printf("\n");              \
        fprintf(report_FP, "\n");  \
    }

/* prety_print_bar */
#define PRETTY_PRINT_BAR(size, symbol) \
    {                                  \
        int i = size;                  \
        if (50 < i) {                  \
            i = 49;                    \
            while (i > 0) {            \
                printf("%s", symbol);  \
                fprintf(report_FP, "%s", symbol); \
                i--;                   \
            }                          \
            printf("+");               \
            fprintf(report_FP, "+");   \
        } else {                       \
            while (i > 0) {            \
                printf("%s", symbol);  \
                fprintf(report_FP, "%s", symbol); \
                i--;                   \
            }                          \
        }                              \
        printf("\n");                  \
        fprintf(report_FP, "\n");      \
    }

/* output_analysis */
int output_analysis(void) {
    char PERCENTAGE[] = "0..........25..........50..........75..........100";
    char *error = NULL, sql[MAX_BUFFER_SIZE];
    char *report_FP_file;
    FILE *report_FP;
    PERFEXPERT_ALLOC(char, report_FP_file, (strlen(globals.moduledir) + 15));
    sprintf(report_FP_file, "%s/report.txt", globals.moduledir);
    if (NULL == (report_FP = fopen(report_FP_file, "a"))) {
        OUTPUT(("%s (%s)", _ERROR("unable to open file"), report_FP_file));
        return;
    }   

    /* Get the total cycles */
    bzero(sql, MAX_BUFFER_SIZE);
    if (my_module_globals.measure_mod == HPCTOOLKIT_MOD) {
        sprintf(sql, "SELECT SUM(value) AS total FROM perfexpert_hotspot AS h JOIN "
            "perfexpert_event AS e ON h.id = e.hotspot_id WHERE h.perfexpert_id = "
            //"%llu AND e.name = 'PAPI_TOT_CYC';", globals.unique_id);
            "%llu AND e.name = 'INST_RETIRED_ANY_P';", globals.unique_id);
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
    //OUTPUT(("%s", _CYAN("Thread balancing")));
    OUTPUT(("%s", _CYAN("Load balancing")));
    fprintf(report_FP, "Load balancing\n");
    PRETTY_PRINT(79, "=");
    if (0.0 == my_module_globals.total) {
        printf("%s the cycles count is zero, measurement problems?\n\n",
            _BOLDRED("WARNING:"));
    }
    printf("%s this analysis does not take into consideration synchronization "
        "between\n         tasks, only the total number of instructions each task"
//        "between\n         threads, only the total number of cycles each thread"
//        " consumed\n\n", _BOLDRED("WARNING:"));
        " executed\n\n", _BOLDRED("WARNING:"));
    fprintf(report_FP, "%s this analysis does not take into consideration synchronization "
        "between\n         tasks, only the total number of instructions each task"
//        "between\n         threads, only the total number of cycles each thread"
//        " consumed\n\n", _BOLDRED("WARNING:"));
        " executed\n\n", _BOLDRED("WARNING:"));
//    printf(" Thread    Cycles     %%    %s\n", PERCENTAGE);
    printf(" Task   Instructions   %%  %s\n", PERCENTAGE);
    fprintf(report_FP, " Task   Instructions   %%  %s\n", PERCENTAGE);
    fclose(report_FP);

    /* Get thread cycles */
    bzero(sql, MAX_BUFFER_SIZE);
    if (my_module_globals.measure_mod == HPCTOOLKIT_MOD) {
        //sprintf(sql, "SELECT h.profile, e.thread_id, SUM(e.value) AS value FROM "
        sprintf(sql, "SELECT h.profile, e.mpi_task, SUM(e.value) AS value FROM "
            "perfexpert_hotspot AS h JOIN perfexpert_event AS e ON h.id = "
            "e.hotspot_id WHERE h.perfexpert_id = %llu AND e.name = 'INST_RETIRED_ANY_P' "
            //"e.hotspot_id WHERE h.perfexpert_id = %llu AND e.name = 'PAPI_TOT_INS' "
            "GROUP BY e.mpi_task ORDER BY e.mpi_task ASC;", globals.unique_id);
            //"GROUP BY e.thread_id ORDER BY e.thread_id ASC;", globals.unique_id);
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

    if (NULL == (report_FP = fopen(report_FP_file, "a"))) {
        OUTPUT(("%s (%s)", _ERROR("unable to open file"), report_FP_file));
        return;
    }   

    /* Print report footer */
    printf(" -------------------------\n");
    fprintf(report_FP, " -------------------------\n");
    //printf("%5d   %10.2g 100.00%% << total cycle count for %s\n",
    printf("%5d   %10.2g 100.00%% << total instruction count for %s\n",
        my_module_globals.threads, my_module_globals.total,
        _CYAN(globals.program));
    printf("                              with a variance up to %.2f%%\n",
        (100 * ((my_module_globals.maximum - my_module_globals.minimum) /
            my_module_globals.maximum)));
    fprintf(report_FP, "%5d   %10.2g 100.00%% << total instruction count for %s\n",
        my_module_globals.threads, my_module_globals.total,
        _CYAN(globals.program));
    fprintf(report_FP, "                              with a variance up to %.2f%%\n",
        (100 * ((my_module_globals.maximum - my_module_globals.minimum) /
            my_module_globals.maximum)));
    PRETTY_PRINT(79, "-");
    PRETTY_PRINT(79, "-");
    printf("\n");
    fprintf(report_FP, "\n");
    fclose(report_FP);
    
    PERFEXPERT_DEALLOC(report_FP_file);
    
    return PERFEXPERT_SUCCESS;
}

/* output_thread */
static inline int output_thread(void *var, int c, char **val, char **names) {

    char *report_FP_file;
    FILE *report_FP;
    PERFEXPERT_ALLOC(char, report_FP_file, (strlen(globals.moduledir) + 15));
    sprintf(report_FP_file, "%s/report.txt", globals.moduledir);
    if (NULL == (report_FP = fopen(report_FP_file, "a"))) {
        OUTPUT(("%s (%s)", _ERROR("unable to open file"), report_FP_file));
        return PERFEXPERT_ERROR;
    }   

    fprintf(report_FP, "%5d   %10.2g %6.2f  ", atoi(val[1]), atof(val[2]),
        (atof(val[2]) * 100 / my_module_globals.total));
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

    fclose(report_FP);
    PERFEXPERT_DEALLOC(report_FP_file);

    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
