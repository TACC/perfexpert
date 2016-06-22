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
 * Authors: Antonio Gomez-Iglesias, Leonardo Fialho and Ashay Rane
 *
 * $HEADER$
 */

#ifdef __cplusplus
extern "C" {
#endif

/* System standard headers */
#include <stdio.h>
#include <strings.h>
#include <float.h>
#include <math.h>

/* Utility headers */
#include <sqlite3.h>

/* Tools headers */
#include "tools/perfexpert/perfexpert_types.h"

/* Modules headers */
#include "timb.h"
#include "timb_database.h"

/* PerfExpert common headers */
#include "common/perfexpert_alloc.h"
#include "common/perfexpert_constants.h"
#include "common/perfexpert_database.h"
#include "common/perfexpert_output.h"

int init_timb_results() {
    char *error, sql[MAX_BUFFER_SIZE];
    bzero(sql, MAX_BUFFER_SIZE);
    sprintf(sql, " CREATE TABLE IF NOT EXISTS macvec_analysis ("
                 " id         INTEGER PRIMARY KEY,             "
                 " mpi_task   INTEGER,                         "
                 " thread_id  VARCHAR,                         "
                 " load       REAL);");
    OUTPUT_VERBOSE((10, "      SQL: %s", sql));
    if (SQLITE_OK != sqlite3_exec(globals.db, sql, NULL, NULL, &error)) {
        OUTPUT(("%s %s", _ERROR("SQL error"), error));
        sqlite3_free(error);
        return PERFEXPERT_ERROR;
    }
    return PERFEXPERT_SUCCESS;
}

/* Store one result of the analysis in the DB */
int store_result(int mpi_task, int thread, double load) {
    char *error = NULL, sql[MAX_BUFFER_SIZE];
    
    bzero(sql, MAX_BUFFER_SIZE);
    /* By inserting a NULL into the primary key we are autoincrementing that id */
    sprintf(sql, "INSERT INTO timb_analysis(id, mpi_task, thread_id, load) "
                 "VALUES(NULL, %d, %d, %f);",
                 mpi_task, thread, load);
    OUTPUT_VERBOSE((10, "      SQL: %s", sql));
    if (SQLITE_OK != sqlite3_exec(globals.db, sql, NULL, NULL, &error)) {
        OUTPUT(("%s %s %s", _ERROR("SQL error"), error, sql));
        sqlite3_free(error);
        return PERFEXPERT_ERROR;
    }
    return PERFEXPERT_SUCCESS;
}

int get_mpi_ranks_LCPI() {
    char sql[MAX_BUFFER_SIZE];
    char *error;
    int value = 0;

    bzero(sql, MAX_BUFFER_SIZE);
    sprintf(sql,
            "SELECT mpi_tasks FROM perfexpert_experiment WHERE perfexpert_id = %llu;",
            globals.unique_id);
    OUTPUT_VERBOSE((10, "importing MPI tasks: %s", sql));
    if (SQLITE_OK != sqlite3_exec(globals.db, sql,
        perfexpert_database_get_int, (void *)&value, &error)) {
        OUTPUT(("%s %s", _ERROR("SQL error"), error));
        sqlite3_free(error);
    }   

    return value+1;
}

int get_threads_LCPI() {
    char sql[MAX_BUFFER_SIZE];
    char *error;
    int value = 0;

    bzero(sql, MAX_BUFFER_SIZE);
    sprintf(sql,
            "SELECT threads FROM perfexpert_experiment WHERE perfexpert_id = %llu;",
            globals.unique_id);
    OUTPUT_VERBOSE((10, "importing threads: %s", sql));
    if (SQLITE_OK != sqlite3_exec(globals.db, sql,
        perfexpert_database_get_int, (void *)&value, &error)) {
        OUTPUT(("%s %s", _ERROR("SQL error"), error));
        sqlite3_free(error);
    }

    return value+1;
}

long long int get_instructions_rank(int rank) {
    char sql[MAX_BUFFER_SIZE];
    char *error;
    double value = 0;
    
    bzero(sql, MAX_BUFFER_SIZE);
    sprintf(sql,
            "SELECT SUM(value) FROM perfexpert_event WHERE "
                "mpi_task=%d AND name='INST_RETIRED_ANY_P';", rank);
    OUTPUT(( "    SQL: %s", sql));
    if (SQLITE_OK != sqlite3_exec(globals.db, sql,
        perfexpert_database_get_double, (void *)&value, &error)) {
        OUTPUT(("%s %s", _ERROR("SQL error"), error));
        sqlite3_free(error);
    }
    return value;
}

long long int get_instructions_thread(int rank, int thread) {
    char sql[MAX_BUFFER_SIZE];
    char *error;
    double value = 0;
    
    bzero(sql, MAX_BUFFER_SIZE);
    sprintf(sql,
            "SELECT SUM(value) FROM perfexpert_event WHERE "
                "mpi_task=%d AND thread_id=%d AND name='INST_RETIRED_ANY_P';", rank, thread);
    OUTPUT(("    SQL: %s", sql));
    if (SQLITE_OK != sqlite3_exec(globals.db, sql,
        perfexpert_database_get_double, (void *)&value, &error)) {
        OUTPUT(("%s %s", _ERROR("SQL error"), error));
        sqlite3_free(error);
    }
    OUTPUT(("RETURNING %f", value));
    return value;
}

#ifdef __cplusplus
}
#endif

// EOF
