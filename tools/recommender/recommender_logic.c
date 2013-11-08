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

/* PerfExpert tool headers */
#include "recommender.h"
#include "recommender_logic.h"

/* PerfExpert common headers */
#include "common/perfexpert_alloc.h"
#include "common/perfexpert_constants.h"
#include "common/perfexpert_database.h"
#include "common/perfexpert_list.h"
#include "common/perfexpert_output.h"

/* select_recommendations */
int select_recommendations(void) {
    char *error = NULL, sql[MAX_BUFFER_SIZE];

    OUTPUT_VERBOSE((4, "%s", _BLUE("Accumulating strategies")));

    perfexpert_list_construct(&(globals.strategies));

    /* Select all strategies, accumulate them */
    bzero(sql, MAX_BUFFER_SIZE);
    sprintf(sql, "SELECT id, name, type, file, line, depth FROM hotspot WHERE "
        "perfexpert_id = %llu", globals.uid);

    if (SQLITE_OK != sqlite3_exec(globals.db,
        "SELECT id, name, statement FROM recommender_strategy;",
        accumulate_strategies, (void *)&(globals.strategies), &error)) {
        OUTPUT(("%s %s", _ERROR("SQL error"), error));
        sqlite3_free(error);
        return PERFEXPERT_ERROR;
    }

    OUTPUT_VERBOSE((8, "%d %s", perfexpert_list_get_size(&(globals.strategies)),
        _MAGENTA("strategies found")));

    /* Select hotspots */
    if (SQLITE_OK != sqlite3_exec(globals.db, sql, select_recom_hotspot, NULL,
        &error)) {
        OUTPUT(("%s %s", _ERROR("SQL error"), error));
        sqlite3_free(error);
        return PERFEXPERT_ERROR;
    }

    return PERFEXPERT_SUCCESS;
}

/* accumulate_strategies */
static int accumulate_strategies(void *strategies, int c, char **val,
    char **names) {
    strategy_t *s = NULL;

    PERFEXPERT_ALLOC(strategy_t, s, sizeof(strategy_t));
    perfexpert_list_item_construct((perfexpert_list_item_t *)s);
    PERFEXPERT_ALLOC(char, s->name, (strlen(val[1]) + 1));
    strcpy(s->name, val[1]);
    PERFEXPERT_ALLOC(char, s->query, (strlen(val[2]) + 1));
    strcpy(s->query, val[2]);
    perfexpert_list_append((perfexpert_list_t *)strategies,
        (perfexpert_list_item_t *)s);

    OUTPUT_VERBOSE((10, "   %s", s->name));

    return PERFEXPERT_SUCCESS;
}

/* select_recommendations_hotspot */
static int select_recom_hotspot(void *var, int c, char **val, char **names) {
    char *error = NULL, sql[MAX_BUFFER_SIZE], *id = val[0], *name =  val[1],
        *type =  val[2], *file =  val[3], *line =  val[4], *depth = val[5];
    sqlite3_stmt *statement = NULL;
    strategy_t *s = NULL;
    int rc = 0;

    OUTPUT(("%s %s:%s", _YELLOW("Selecting recommendations for"), file, line));

    /* Create a temporary table to store possible recommendations */
    bzero(sql, MAX_BUFFER_SIZE);
    sprintf(sql, "CREATE TEMP TABLE temp_%d_%s (rid INTEGER, score FLOAT);",
        (int)getpid(), id);

    if (SQLITE_OK != sqlite3_exec(globals.db, sql, NULL, NULL, &error)) {
        OUTPUT(("%s %s", _ERROR("SQL error"), error));
        sqlite3_free(error);
        return PERFEXPERT_ERROR;
    }

    /* For each strategy... */
    perfexpert_list_for(s, &(globals.strategies), strategy_t) {
        OUTPUT_VERBOSE((8, "   %s '%s'", _CYAN("running"), s->name));

        /* ...prepare the SQL statement... */
        if (SQLITE_OK != sqlite3_prepare_v2(globals.db, s->query,
            strlen(s->query), &statement, NULL)) {
            OUTPUT(("%s %s", _ERROR("SQL error"), error, s->query));
            sqlite3_free(error);
            return PERFEXPERT_ERROR;
        }

        /* ...bind UID... */
        if (SQLITE_OK != sqlite3_bind_int64(statement,
            sqlite3_bind_parameter_index(statement, "@HID"),
            strtoll(id, NULL, 10))) {
            OUTPUT_VERBOSE((9, "      %s (%llu)", _RED("ignoring @HID"),
                globals.uid));
        }

        /* ...run strategy... */
        while (SQLITE_ROW == (rc = sqlite3_step(statement))) {
            /* If this strategy returns an error... */
            if (SQLITE_ROW != rc) {
                return PERFEXPERT_ERROR;
            }

            /* ...if strategy's results are null or in an invalid format... */
            if ((SQLITE_INTEGER != sqlite3_column_type(statement, 0)) ||
                (SQLITE_FLOAT != sqlite3_column_type(statement, 1)) ||
                (SQLITE_DONE == rc)) {
                continue;
            }

            /* ...insert recommendation into temporary table... */
            bzero(sql, MAX_BUFFER_SIZE);
            sprintf(sql, "INSERT INTO temp_%d_%s VALUES (%d, %f);",
                (int)getpid(), id, sqlite3_column_int(statement, 0),
                sqlite3_column_double(statement, 1));

            if (SQLITE_OK != sqlite3_exec(globals.db, sql, NULL, NULL,
                &error)) {
                OUTPUT(("%s %s", _ERROR("SQL error"), error));
                sqlite3_free(error);
                return PERFEXPERT_ERROR;
            }

            OUTPUT_VERBOSE((10, "      [recommendation=%d], [score=%f]",
                sqlite3_column_int(statement, 0),
                sqlite3_column_double(statement, 1)));
        }

        /* Something went wrong... :-( */
        if (SQLITE_DONE != rc) {
            OUTPUT(("%s %s", _ERROR("SQL error"), error));
            sqlite3_free(error);
            return PERFEXPERT_ERROR;
        }

        /* ...and cleanup the statement! */
        if (SQLITE_OK != sqlite3_reset(statement)) {
            OUTPUT(("%s %s", _ERROR("SQL error"), error));
            sqlite3_free(error);
            return PERFEXPERT_ERROR;
        }
        if (SQLITE_OK != sqlite3_finalize(statement)) {
            OUTPUT(("%s %s", _ERROR("SQL error"), error));
            sqlite3_free(error);
            return PERFEXPERT_ERROR;
        }
    }

    /* Check if there is any recommendation */
    bzero(sql, MAX_BUFFER_SIZE);
    sprintf(sql, "SELECT COUNT(*) FROM temp_%d_%s;", (int)getpid(), id,
        globals.rec_count);

    if (SQLITE_OK != sqlite3_exec(globals.db, sql, perfexpert_database_get_int,
        (void *)&rc, &error)) {
        OUTPUT(("%s %s", _ERROR("SQL error"), error));
        sqlite3_free(error);
        return PERFEXPERT_ERROR;
    }

    if (0 == rc) {
        return PERFEXPERT_SUCCESS;
    }

    /* Print recommendations header */
    fprintf(globals.outputfile_FP,
        "#-----------------------------------------------------------------\n");
    fprintf(globals.outputfile_FP, "# Recommendations for %s:%s\n", file, line);
    fprintf(globals.outputfile_FP,
        "#-----------------------------------------------------------------\n");

    if (NULL != globals.outputmetrics) {
        fprintf(globals.outputmetrics_FP, "%% Hotspot=%s:%s\n", file, line);
        fprintf(globals.outputmetrics_FP, "source.file=%s\n", file);
        fprintf(globals.outputmetrics_FP, "source.line=%s\n", line);
        fprintf(globals.outputmetrics_FP, "source.type=%s\n", type);
        fprintf(globals.outputmetrics_FP, "source.name=%s\n", name);
        fprintf(globals.outputmetrics_FP, "source.depth=%s\n", depth);
    }

    /* Select top-N recommendations, output them besides the pattern */
    bzero(sql, MAX_BUFFER_SIZE);
    sprintf(sql,
        "SELECT name, reason, id, example FROM recommender_recommendation AS r "
        "INNER JOIN temp_%d_%s AS t ON r.id = t.rid ORDER BY t.score "
        "DESC LIMIT %d;", (int)getpid(), id, globals.rec_count);

    if (SQLITE_OK != sqlite3_exec(globals.db, sql, print_recom, NULL, &error)) {
        OUTPUT(("%s %s", _ERROR("SQL error"), error));
        sqlite3_free(error);
        return PERFEXPERT_ERROR;
    }

    return PERFEXPERT_SUCCESS;
}

/* output_recommendations */
static int print_recom(void *var, int count, char **val, char **names) {

    globals.no_rec = PERFEXPERT_SUCCESS;

    fprintf(globals.outputfile_FP, "#\n# %s\n#\n",
        _MAGENTA("Possible optimization:"));
    fprintf(globals.outputfile_FP, "%s\n%s\n\n", _CYAN("Description:"), val[0]);
    fprintf(globals.outputfile_FP, "%s\n%s\n\n", _CYAN("Reason:"), val[1]);
    fprintf(globals.outputfile_FP, "%s\n\n%s\n\n", _CYAN("Example:"), val[3]);

    if (NULL != globals.outputmetrics) {
        fprintf(globals.outputmetrics_FP, "recommender.rid=%s\n", val[2]);
    }

    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
