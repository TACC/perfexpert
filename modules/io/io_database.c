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
#include "io.h"
#include "io_database.h"

/* PerfExpert common headers */
#include "common/perfexpert_alloc.h"
#include "common/perfexpert_constants.h"
#include "common/perfexpert_database.h"
#include "common/perfexpert_hash.h"
#include "common/perfexpert_list.h"
#include "common/perfexpert_md5.h"
#include "common/perfexpert_output.h"

#include <omp.h>
#include <sched.h>

/* database_export */
int database_export(io_function_t *results) {
    char *error = NULL, sql[MAX_BUFFER_SIZE];
    int i;

    OUTPUT_VERBOSE((5, "%s", _YELLOW("Exporting IO metrics")));

    /* Check if the required tables are available */
    bzero(sql, MAX_BUFFER_SIZE);
    sprintf(sql, "PRAGMA foreign_keys = ON; "
                 "CREATE TABLE IF NOT EXISTS io_metric (  "
                 "id            INTEGER PRIMARY KEY,        "
                 "function      VARCHAR NOT NULL,           "
                 "filename      VARCHAR NOT NULL,           "
                 "line          INTEGER NOT NULL,           "
                 "value         INTEGER NOT NULL,           );");

    OUTPUT_VERBOSE((5, "%s", _BLUE("Writing IO profiles to database")));

    if (SQLITE_OK != sqlite3_exec(globals.db, sql, NULL, NULL, &error)) {
        OUTPUT(("%s %s", _ERROR("SQL error"), error));
        sqlite3_free(error);
        return PERFEXPERT_ERROR;
    }

    /* Begin a transaction to improve SQLite performance */
    if (SQLITE_OK != sqlite3_exec(globals.db, "BEGIN TRANSACTION;", NULL, NULL,
        &error)) {
        OUTPUT(("%s %s", _ERROR("SQL error"), error));
        sqlite3_free(error);
        return PERFEXPERT_ERROR;
    }

    for (i=0; i<results->size; i++) {
        bzero(sql, MAX_BUFFER_SIZE);
        sprintf(sql, "INSERT INTO io_metric (function, filename, line, value)"
                      " VALUES ('%s', %ld, %ld);", results->code[i].function_name,
                      results->code[i].file_name, results->code[i].address,
                      results->code[i].count);
        OUTPUT_VERBOSE((10, "      %s SQL: %s", _CYAN(results->code[i].function_name), sql));
        if (SQLITE_OK != sqlite3_exec(globals.db, sql, NULL, NULL, &error)) {
            OUTPUT(("%s %s", _ERROR("SQL error"), error));
            sqlite3_free(error);
            return PERFEXPERT_ERROR;
        }
        
    }
    /* End transaction */
    if (SQLITE_OK != sqlite3_exec(globals.db, "END TRANSACTION;", NULL, NULL,
        &error)) {
        OUTPUT(("%s %s", _ERROR("SQL error"), error));
        sqlite3_free(error);
        return PERFEXPERT_ERROR;
    }

    return PERFEXPERT_SUCCESS;
}

/* database_import */
int database_import(perfexpert_list_t *profiles) {
    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
