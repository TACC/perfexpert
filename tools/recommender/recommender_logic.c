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
#include <stdio.h>
#include <string.h>

/* Utility headers */
#include <sqlite3.h>

/* PerfExpert headers */
#include "recommender.h"
#include "perfexpert_alloc.h"
#include "perfexpert_database.h"
#include "perfexpert_list.h"
#include "perfexpert_output.h"
#include "perfexpert_util.h"

/* select_recommendations_all */
int select_recommendations_all(perfexpert_list_t *segments) {
    int rc = PERFEXPERT_NO_REC;
    segment_t *segment = NULL;

    OUTPUT_VERBOSE((4, "%s", _BLUE("Selecting recommendations")));

    /* For each code bottleneck... */
    segment = (segment_t *)perfexpert_list_get_first(segments);
    while ((perfexpert_list_item_t *)segment != &(segments->sentinel)) {
        /* ...select recommendations */
        switch (select_recommendations(segment)) {
            case PERFEXPERT_NO_REC:
                OUTPUT(("%s", _GREEN("Sorry, we have no recommendations")));
                goto MOVE_ON;

            case PERFEXPERT_SUCCESS:
                rc = PERFEXPERT_SUCCESS;
                break;

            case PERFEXPERT_ERROR:
            default: 
                OUTPUT(("%s", _ERROR("Error: selecting recommendations")));
                return PERFEXPERT_ERROR;
        }
        /* Move to the next code bottleneck */
        MOVE_ON:
        segment = (segment_t *)perfexpert_list_get_next(segment);
    }
    return rc;
}

/* select_recommendations */
int select_recommendations(segment_t *segment) {
    function_t *function = NULL;
    char *error_msg = NULL;
    char sql[BUFFER_SIZE];
    double weight = 0.0;
    int rc = 0;

    OUTPUT_VERBOSE((4, "   %s (%s:%d)", _YELLOW("selecting recommendation for"),
        segment->filename, segment->line_number));

    /* Select all functions, accumulate them */
    if (SQLITE_OK != sqlite3_exec(globals.db,
        "SELECT id, desc, statement FROM function;", accumulate_functions,
        (void *)&(segment->functions), &error_msg)) {
        OUTPUT(("%s %s", _ERROR("Error: SQL error"), error_msg));
        sqlite3_free(error_msg);
        return PERFEXPERT_ERROR;
    }

    OUTPUT_VERBOSE((8, "      %s (%d)", _MAGENTA("function(s) found"),
        perfexpert_list_get_size(&(segment->functions))));

    /* Create a temporary table to store possible recommendations */
    bzero(sql, BUFFER_SIZE);
    sprintf(sql, "%s_%d_%d %s %s", "CREATE TEMP TABLE recommendation",
        (int)getpid(), segment->rowid, "(function_id INTEGER,",
        "recommendation_id INTEGER, score FLOAT, weigth FLOAT);");
    OUTPUT_VERBOSE((10, "      %s", _CYAN("creating temporary table")));
    OUTPUT_VERBOSE((10, "         SQL: %s", _CYAN(sql)));

    if (SQLITE_OK != sqlite3_exec(globals.db, sql, NULL, NULL, &error_msg)) {
        OUTPUT(("%s %s", _ERROR("Error: SQL error"), error_msg));
        sqlite3_free(error_msg);
        return PERFEXPERT_ERROR;
    }

    /* Print recommendations header */
    if (PERFEXPERT_SUCCESS != output_header(segment)) {
        OUTPUT(("%s", _ERROR("Error: unable to print recommendations header")));
        return PERFEXPERT_ERROR;
    }

    /* For each function... */
    function = (function_t *)perfexpert_list_get_first(&(segment->functions));
    while ((perfexpert_list_item_t *)function != &(segment->functions.sentinel)) {
        sqlite3_stmt *statement;
        
        OUTPUT_VERBOSE((8, "      %s '%s' [%d bytes]", _CYAN("running"),
            function->desc, strlen(function->statement)));

        /* Prepare the SQL statement */
        if (SQLITE_OK != sqlite3_prepare_v2(globals.db, function->statement,
            BUFFER_SIZE, &statement, NULL)) {
            OUTPUT(("%s %s", _ERROR("Error: SQL error"), error_msg));
            sqlite3_free(error_msg);
            return PERFEXPERT_ERROR;
        }

        /* Bind ROWID */
        if (SQLITE_OK != sqlite3_bind_int(statement,
            sqlite3_bind_parameter_index(statement, "@RID"), segment->rowid)) {
            OUTPUT(("         %s (%d)", _RED("ignoring @RID"), segment->rowid));
        }

        /* Bind loop depth */
        if (SQLITE_OK != sqlite3_bind_int(statement,
            sqlite3_bind_parameter_index(statement, "@LPD"),
            segment->loopdepth)) {
            OUTPUT(("         %s (%d)", _RED("ignoring @LPD"),
                segment->loopdepth));
        }

        /* Run query */
        while (SQLITE_ROW == (rc = sqlite3_step(statement))) {
            /* It is possible that this function does not return results... */
            if (SQLITE_DONE == rc) {
                continue;
            }

            /* It is possible that this function returns an error... */
            if (SQLITE_ROW != rc) {
                return PERFEXPERT_ERROR;
            }

            /* ... but if there is one row, check if the two first columns are
             * SQLITE_INTEGER and SQLITE_FLOAT respectivelly
             */
            if (SQLITE_INTEGER != sqlite3_column_type(statement, 0)) {
                OUTPUT(("         %s", _ERROR("1st column is not an integer")));
                continue;
            }
            if (SQLITE_FLOAT != sqlite3_column_type(statement, 1)) {
                OUTPUT(("         %s", _ERROR("2nd column is not a float")));
                continue;
            }

            /* Consider only the results where the score is positive */
            if (0 < sqlite3_column_double(statement, 1)) {
                /* Insert recommendation into the temporary table */
                bzero(sql, BUFFER_SIZE);
                sprintf(sql, "%s_%d_%d %s (%d, %d, %f, %f);",
                    "INSERT INTO recommendation", (int)getpid(), segment->rowid,
                    "VALUES", function->id, sqlite3_column_int(statement, 0),
                    sqlite3_column_double(statement, 1), weight);
                if (SQLITE_OK != sqlite3_exec(globals.db, sql, NULL, NULL,
                    &error_msg)) {
                    OUTPUT(("%s %s", _ERROR("Error: SQL error"), error_msg));
                    sqlite3_free(error_msg);
                    return PERFEXPERT_ERROR;
                }

                OUTPUT_VERBOSE((10, "         Function=%d, Recom=%d, Score=%f",
                    function->id, sqlite3_column_int(statement, 0),
                    sqlite3_column_double(statement, 1)));
            }
        }

        /* Something went wrong :-/ */
        if (SQLITE_DONE != rc) {
            OUTPUT(("%s %s", _ERROR("Error: SQL error"), error_msg));
            sqlite3_free(error_msg);
            return PERFEXPERT_ERROR;
        }
        
        /* SQLite3 cleanup */
        if (SQLITE_OK != sqlite3_reset(statement)) {
            OUTPUT(("%s %s", _ERROR("Error: SQL error"), error_msg));
            sqlite3_free(error_msg);
            return PERFEXPERT_ERROR;
        }
        if (SQLITE_OK != sqlite3_finalize(statement)) {
            OUTPUT(("%s %s", _ERROR("Error: SQL error"), error_msg));
            sqlite3_free(error_msg);
            return PERFEXPERT_ERROR;
        }
        
        /* Move to the next code bottleneck */
        function = (function_t *)perfexpert_list_get_next(function);
    }

    /* Select top-N recommendations, output them besides the pattern */
    bzero(sql, BUFFER_SIZE);
    sprintf(sql, "%s %s%d_%d %s %s %d;",
        "SELECT r.desc AS desc, r.reason AS reason, r.id AS id, r.example",
        "AS example FROM recommendation AS r INNER JOIN recommendation_",
        (int)getpid(), segment->rowid, "AS m ON r.id = m.recommendation_id",
        "ORDER BY m.score DESC LIMIT", globals.rec_count);

    OUTPUT_VERBOSE((10, "      %s [%d]", _CYAN("top-N"), globals.rec_count));
    OUTPUT_VERBOSE((10, "         SQL: %s", _CYAN(sql)));

    rc = PERFEXPERT_NO_REC;
    if (SQLITE_OK != sqlite3_exec(globals.db, sql, output_recommendations,
        (void *)&(rc), &error_msg)) {
        OUTPUT(("%s %s", _ERROR("Error: SQL error"), error_msg));
        sqlite3_free(error_msg);
        return PERFEXPERT_ERROR;
    }

    return rc;
}

/* accumulate_functions */
int accumulate_functions(void *functions, int count, char **val, char **names) {
    function_t *function = NULL;
    
    /* Copy SQL query result into functions list */
    PERFEXPERT_ALLOC(function_t, function, sizeof(function_t));
    perfexpert_list_item_construct((perfexpert_list_item_t *)function);
    function->id = atoi(val[0]);
    PERFEXPERT_ALLOC(char, function->desc, (strlen(val[1]) + 1));
    strncpy(function->desc, val[1], strlen(val[1]));
    bzero(function->statement, BUFFER_SIZE);
    strncpy(function->statement, val[2], strlen(val[2]));
    perfexpert_list_append((perfexpert_list_t *)functions,
        (perfexpert_list_item_t *)function);

    OUTPUT_VERBOSE((10, "      '%s'", function->desc));

    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
