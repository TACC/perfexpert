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

/* Tools headers */
#include "tools/perfexpert/perfexpert_types.h"

/* Module headers */
#include "sqlrules.h"
#include "sqlrules_logic.h"

/* PerfExpert common headers */
#include "common/perfexpert_alloc.h"
#include "common/perfexpert_constants.h"
#include "common/perfexpert_database.h"
#include "common/perfexpert_list.h"
#include "common/perfexpert_output.h"

/* select_recommendations */
int select_recommendations(void) {
    char *error = NULL, sql[MAX_BUFFER_SIZE];

    OUTPUT_VERBOSE((4, "%s", _BLUE("Accumulating rules")));

    /* Select all strategies, accumulate them */
    bzero(sql, MAX_BUFFER_SIZE);
    sprintf(sql, "SELECT id, name, type, file, line, depth FROM hotspot WHERE "
        "perfexpert_id = %llu", globals.unique_id);

    if (SQLITE_OK != sqlite3_exec(globals.db,
        "SELECT id, name, statement FROM rule;", accumulate_rules,
        (void *)&(my_module_globals.rules), &error)) {
        OUTPUT(("%s %s", _ERROR("SQL error"), error));
        sqlite3_free(error);
        return PERFEXPERT_ERROR;
    }

    OUTPUT_VERBOSE((8, "%d %s", perfexpert_list_get_size(
        &(my_module_globals.rules)), _MAGENTA("rules found")));

    /* Select hotspots */
    if (SQLITE_OK != sqlite3_exec(globals.db, sql, select_recom_hotspot, NULL,
        &error)) {
        OUTPUT(("%s %s", _ERROR("SQL error"), error));
        sqlite3_free(error);
        return PERFEXPERT_ERROR;
    }

    return PERFEXPERT_SUCCESS;
}

/* accumulate_rules */
static int accumulate_rules(void *rules, int c, char **val, char **names) {
    rule_t *r = NULL;

    PERFEXPERT_ALLOC(rule_t, r, sizeof(rule_t));
    perfexpert_list_item_construct((perfexpert_list_item_t *)r);
    PERFEXPERT_ALLOC(char, r->name, (strlen(val[1]) + 1));
    strcpy(r->name, val[1]);
    PERFEXPERT_ALLOC(char, r->query, (strlen(val[2]) + 1));
    strcpy(r->query, val[2]);
    perfexpert_list_append((perfexpert_list_t *)rules,
        (perfexpert_list_item_t *)r);

    OUTPUT_VERBOSE((10, "   %s", r->name));

    return PERFEXPERT_SUCCESS;
}

/* select_recommendations_hotspot */
static int select_recom_hotspot(void *var, int c, char **val, char **names) {
    char *error = NULL, sql[MAX_BUFFER_SIZE], *id = val[0], *name =  val[1],
        *type =  val[2], *file =  val[3], *line =  val[4], *depth = val[5];
    sqlite3_stmt *statement = NULL;
    rule_t *r = NULL;
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

    /* For each rule... */
    perfexpert_list_for(r, &(my_module_globals.rules), rule_t) {
        OUTPUT_VERBOSE((8, "   %s '%s'", _CYAN("running"), r->name));

        /* ...prepare the SQL statement... */
        if (SQLITE_OK != sqlite3_prepare_v2(globals.db, r->query,
            strlen(r->query), &statement, NULL)) {
            OUTPUT(("%s %s", _ERROR("SQL error"), error, r->query));
            sqlite3_free(error);
            return PERFEXPERT_ERROR;
        }

        /* ...bind UID... */
        if (SQLITE_OK != sqlite3_bind_int64(statement,
            sqlite3_bind_parameter_index(statement, "@HID"),
            strtoll(id, NULL, 10))) {
            OUTPUT_VERBOSE((9, "      %s (%llu)", _RED("ignoring @HID"),
                globals.unique_id));
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
        my_module_globals.rec_count);

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
    printf("#-----------------------------------------------------------------\n");
    printf("# Recommendations for %s:%s\n", file, line);
    printf("#-----------------------------------------------------------------\n");

    if (NULL != my_module_globals.metrics_FP) {
        fprintf(my_module_globals.metrics_FP, "%% Hotspot=%s:%s\n", file, line);
        fprintf(my_module_globals.metrics_FP, "source.file=%s\n", file);
        fprintf(my_module_globals.metrics_FP, "source.line=%s\n", line);
        fprintf(my_module_globals.metrics_FP, "source.type=%s\n", type);
        fprintf(my_module_globals.metrics_FP, "source.name=%s\n", name);
        fprintf(my_module_globals.metrics_FP, "source.depth=%s\n", depth);
    }

    /* Select top-N recommendations, output them besides the pattern */
    bzero(sql, MAX_BUFFER_SIZE);
    sprintf(sql, "SELECT name, reason, id, example FROM recommendation AS r "
        "INNER JOIN temp_%d_%s AS t ON r.id = t.rid ORDER BY t.score DESC "
        "LIMIT %d;", (int)getpid(), id, my_module_globals.rec_count);

    if (SQLITE_OK != sqlite3_exec(globals.db, sql, print_recom, NULL, &error)) {
        OUTPUT(("%s %s", _ERROR("SQL error"), error));
        sqlite3_free(error);
        return PERFEXPERT_ERROR;
    }

    return PERFEXPERT_SUCCESS;
}

/* output_recommendations */
static int print_recom(void *var, int count, char **val, char **names) {

    my_module_globals.no_rec = PERFEXPERT_SUCCESS;

    printf("#\n# %s\n#\n", _MAGENTA("Possible optimization:"));
    printf("%s\n%s\n\n", _CYAN("Description:"), val[0]);
    printf("%s\n%s\n\n", _CYAN("Reason:"), val[1]);
    printf("%s\n\n%s\n\n", _CYAN("Example:"), val[3]);

    if (NULL != my_module_globals.metrics_FP) {
        fprintf(my_module_globals.metrics_FP, "recommender.rid=%s\n", val[2]);
    }

    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
