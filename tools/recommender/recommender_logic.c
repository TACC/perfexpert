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

#ifdef __cplusplus
extern "C" {
#endif

/* System standard headers */
#include <stdio.h>
#include <string.h>

/* Utility headers */
#include <sqlite3.h>

/* PerfExpert headers */
#include "config.h"
#include "recommender.h"
#include "perfexpert_output.h"
#include "perfexpert_util.h"
#include "perfexpert_list.h"
#include "perfexpert_database.h"

/* select_recommendations */
int select_recommendations(segment_t *segment) {
    char *error_msg = NULL;
    function_t *function;
    char sql[BUFFER_SIZE];
    char temp_str[BUFFER_SIZE];
    double weight = 0;
    int rc;

    perfexpert_list_construct((perfexpert_list_t *) &(segment->functions));
    
    OUTPUT_VERBOSE((7, "=== %s", _BLUE("Querying DB")));

    if (NULL != globals.workdir) {
        fprintf(globals.outputfile_FP, "%% recommendation for %s:%d\n",
                segment->filename, segment->line_number);
        fprintf(globals.outputfile_FP, "code.filename=%s\n",
                segment->filename);
        fprintf(globals.outputfile_FP, "code.line_number=%d\n",
                segment->line_number);
        fprintf(globals.outputfile_FP, "code.type=%s\n", segment->type);
        fprintf(globals.outputfile_FP, "code.function_name=%s\n",
                segment->function_name);
        fprintf(globals.outputfile_FP, "code.loop_depth=%1.0lf\n",
                segment->loop_depth);
        fprintf(globals.outputfile_FP, "code.rowid=%d\n", segment->rowid);
    } else {
        fprintf(globals.outputfile_FP,
                "#--------------------------------------------------\n");
        fprintf(globals.outputfile_FP, "# Recommendations for %s:%d\n",
                segment->filename, segment->line_number);
        fprintf(globals.outputfile_FP,
                "#--------------------------------------------------\n");
    }
    
    /* Select all functions, accumulate them */
    OUTPUT_VERBOSE((8, "%s [%s:%d, ROWID: %d]",
        _YELLOW("looking for functions to segment"), segment->filename,
        segment->line_number, segment->rowid));
    
    if (SQLITE_OK != sqlite3_exec(globals.db,
        "SELECT id, desc, statement FROM function;", accumulate_functions,
        (void *)&(segment->functions), &error_msg)) {
        fprintf(stderr, "Error: SQL error: %s\n", error_msg);
        sqlite3_free(error_msg);
        return PERFEXPERT_ERROR;
    }

    /* Execute all functions, accumulate recommendations */
    OUTPUT_VERBOSE((8, "%d %s", perfexpert_list_get_size(&(segment->functions)),
        _GREEN("functions found")));

    /* Create a temporary table to store possible recommendations */
    bzero(sql, BUFFER_SIZE);
    sprintf(sql, "CREATE TEMP TABLE recommendation_%d_%d (function_id INTEGER,",
        (int)getpid(), segment->rowid);
    strcat(sql, " recommendation_id INTEGER, score FLOAT, weigth FLOAT);");

    OUTPUT_VERBOSE((10, "%s",
        _YELLOW("creating temporary table of recommendations")));
    OUTPUT_VERBOSE((10, "   SQL: %s", _CYAN(sql)));
    
    if (SQLITE_OK != sqlite3_exec(globals.db, sql, NULL, NULL, &error_msg)) {
        fprintf(stderr, "Error: SQL error: %s\n", error_msg);
        sqlite3_free(error_msg);
        return PERFEXPERT_ERROR;
    }

    /* For each function, execute it! */
    function = (function_t *)perfexpert_list_get_first(&(segment->functions));
    while ((perfexpert_list_item_t *)function != &(segment->functions.sentinel)) {
        sqlite3_stmt *statement;
        
        OUTPUT_VERBOSE((8, "%s (ID: %d, %s) [%d bytes]",
            _YELLOW("running function"), function->id, function->desc,
            strlen(function->statement)));

        /* Prepare the SQL statement */
        if (SQLITE_OK != sqlite3_prepare_v2(globals.db, function->statement,
                                            BUFFER_SIZE, &statement, NULL)) {
            fprintf(stderr, "Error: SQL error: %s\n", error_msg);
            sqlite3_free(error_msg);
            return PERFEXPERT_ERROR;
        }

        /* Bind ROWID */
        if (SQLITE_OK != sqlite3_bind_int(statement,
            sqlite3_bind_parameter_index(statement, "@RID"), segment->rowid)) {
            fprintf(stderr, "Error: SQL error: %s\n", error_msg);
            sqlite3_free(error_msg);
            return PERFEXPERT_ERROR;
        }

        /* Bind loop depth */
        if (SQLITE_OK != sqlite3_bind_double(statement,
            sqlite3_bind_parameter_index(statement, "@LPD"),
            segment->loop_depth)) {
            fprintf(stderr, "Error: SQL error: %s\n", error_msg);
            sqlite3_free(error_msg);
            return PERFEXPERT_ERROR;
        }

        /* Run que query */
        OUTPUT_VERBOSE((10, "   %s", _GREEN("list of recommendations: ")));
        do {
            rc = sqlite3_step(statement);

            /* It is possible that this function does not return results */
            if (SQLITE_DONE == rc) {
                OUTPUT_VERBOSE((10,
                    "      end of recommendations for function (%d)",
                    function->id));
                continue;
            } else if (SQLITE_ROW == rc) {
                /* check if there are at least two columns in the SQL result and
                 * their types are SQLITE_INTEGER and SQLITE_FLOAT respectivelly
                 */
                if (SQLITE_INTEGER != sqlite3_column_type(statement, 0)) {
                    OUTPUT(("%s", _ERROR("1st column is not an integer")));
                    continue;
                }
                if (SQLITE_FLOAT != sqlite3_column_type(statement, 1)) {
                    OUTPUT(("%s", _ERROR("2nd column is not a float")));
                    continue;
                }
                
                /* Consider only the results where the score is positive */
                if (0 < sqlite3_column_double(statement, 1)) {
                    /* Find the weigth of this recommendation */
                    weight = 0;
                    bzero(sql, BUFFER_SIZE);
                    strcat(sql,
                           "SELECT weight FROM recommendation_function WHERE");
                    bzero(temp_str, BUFFER_SIZE);
                    sprintf(temp_str,
                            " id_recommendation=%d AND id_function=%d;",
                            sqlite3_column_int(statement, 0), function->id);
                    strcat(sql, temp_str);
                    if (SQLITE_OK != sqlite3_exec(globals.db, sql,
                        perfexpert_database_get_double, (void *)&weight,
                        &error_msg)) {
                        fprintf(stderr, "Error: SQL error: %s\n", error_msg);
                        sqlite3_free(error_msg);
                        return PERFEXPERT_ERROR;
                    }
                    
                    /* Insert recommendation into the temporary table */
                    bzero(sql, BUFFER_SIZE);
                    sprintf(sql, "INSERT INTO recommendation_%d_%d ",
                            (int)getpid(), segment->rowid);
                    bzero(temp_str, BUFFER_SIZE);
                    sprintf(temp_str, "VALUES (%d, %d, %f, %f);", function->id,
                            sqlite3_column_int(statement, 0),
                            sqlite3_column_double(statement, 1), weight);
                    strcat(sql, temp_str);
                    
                    if (SQLITE_OK != sqlite3_exec(globals.db, sql, NULL, NULL,
                                                  &error_msg)) {
                        fprintf(stderr, "Error: SQL error: %s\n", error_msg);
                        sqlite3_free(error_msg);
                        return PERFEXPERT_ERROR;
                    }
                    
                    OUTPUT_VERBOSE((10, "      Func=%d, Rec=%d, Score=%f, Weight=%f",
                        function->id, sqlite3_column_int(statement, 0),
                        sqlite3_column_double(statement, 1), weight));
                }
            }
        } while (SQLITE_ROW == rc);

        /* Something went wrong :-/ */
        if (SQLITE_DONE != rc) {
            fprintf(stderr, "Error: SQL error: %s\n",
                    sqlite3_errmsg(globals.db));
            sqlite3_free(error_msg);
            return PERFEXPERT_ERROR;
        }
        
        /* SQLite3 cleanup */
        if (SQLITE_OK != sqlite3_reset(statement)) {
            fprintf(stderr, "Error: SQL error: %s\n", error_msg);
            sqlite3_free(error_msg);
            return PERFEXPERT_ERROR;
        }
        if (SQLITE_OK != sqlite3_finalize(statement)) {
            fprintf(stderr, "Error: SQL error: %s\n", error_msg);
            sqlite3_free(error_msg);
            return PERFEXPERT_ERROR;
        }
        
        /* Move to the next code bottleneck */
        function = (function_t *)perfexpert_list_get_next(function);
    }

    /* Calculate the normalized rank of recommendations, should use weights? */
    // TODO: someday, I will add the weighting system here (Leo)

    /* Select top-N recommendations, output them besides the pattern */
    bzero(sql, BUFFER_SIZE);
    strcat(sql, "SELECT r.desc AS desc, r.reason AS reason, r.id AS id, ");
    strcat(sql, "r.example AS example FROM recommendation AS r INNER JOIN ");
    bzero(temp_str, BUFFER_SIZE);
    sprintf(temp_str, "recommendation_%d_%d AS m ON r.id = m.recommendation_id",
            (int)getpid(), segment->rowid);
    strcat(sql, temp_str);
    bzero(temp_str, BUFFER_SIZE);
    sprintf(temp_str, " ORDER BY m.score DESC LIMIT %d;", globals.rec_count);
    strcat(sql, temp_str);
    
    OUTPUT_VERBOSE((10, "%s (top %d)",
        _YELLOW("selecting top-N recommendations"), globals.rec_count));
    OUTPUT_VERBOSE((10, "   SQL: %s", _CYAN(sql)));

    rc = PERFEXPERT_NO_REC;

    if (SQLITE_OK != sqlite3_exec(globals.db, sql, output_recommendations,
                                  (void *)&(rc), &error_msg)) {
        fprintf(stderr, "Error: SQL error: %s\n", error_msg);
        sqlite3_free(error_msg);
        return PERFEXPERT_ERROR;
    }

    return rc;
}

/* accumulate_functions */
int accumulate_functions(void *functions, int count, char **val, char **names) {
    function_t *function;
    
    /* Copy SQL query result into functions list */
    function = (function_t *)malloc(sizeof(function_t));
    if (NULL == function) {
        OUTPUT(("%s", _ERROR("Error: out of memory")));
        goto cleanup;
    }
    perfexpert_list_item_construct((perfexpert_list_item_t *)function);

    function->id = atoi(val[0]);
    
    function->desc = (char *)malloc(strlen(val[1]) + 1);
    if (NULL == function->desc) {
        OUTPUT(("%s", _ERROR("Error: out of memory")));
        goto cleanup;
    }
    bzero(function->desc, strlen(val[1]) + 1);
    strncpy(function->desc, val[1], strlen(val[1]));
    
    bzero(function->statement, BUFFER_SIZE);
    strncpy(function->statement, val[2], strlen(val[2]));

    perfexpert_list_append((perfexpert_list_t *)functions,
                           (perfexpert_list_item_t *)function);

    OUTPUT_VERBOSE((10, "%s (ID: %d, %s) [%d bytes]", _YELLOW("function found"),
                    function->id, function->desc, strlen(function->statement)));

    return PERFEXPERT_SUCCESS;

    cleanup:
    /* Close input file */
    if (NULL != globals.inputfile) {
        fclose(globals.inputfile_FP);
    }
    if (NULL != globals.outputfile) {
        fclose(globals.outputfile_FP);
    }

    /* TODO: Free memory */

    return PERFEXPERT_ERROR;
}

#ifdef __cplusplus
}
#endif

// EOF
