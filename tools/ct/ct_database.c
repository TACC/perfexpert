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
#include <stdlib.h>
#include <string.h>

/* Utility headers */
#include <sqlite3.h>

/* PerfExpert tool headers */
#include "ct.h"
#include "ct_database.h"

/* PerfExpert common headers */
#include "common/perfexpert_alloc.h"
#include "common/perfexpert_constants.h"
#include "common/perfexpert_list.h"
#include "common/perfexpert_output.h"

/* select_transformations */
int select_transformations(fragment_t *fragment) {
    char sql[MAX_BUFFER_SIZE], *error = NULL;
    recommendation_t *r = NULL;

    OUTPUT_VERBOSE((7, "%s",
        _BLUE("Looking for code transformers and code patterns")));

    /* For each recommendation in this fragment, select the transformers */
    perfexpert_list_for(r, &(fragment->recommendations), recommendation_t) {
        OUTPUT_VERBOSE((4, "   [%d] %s", r->id,
            _YELLOW("selecting transformations")));

        /* Initialyze the list of transformations */
        perfexpert_list_construct(&(r->transformations));

        /* Find the transformations available for this recommendation */
        bzero(sql, MAX_BUFFER_SIZE);
        sprintf(sql,"SELECT t.transformer, t.id FROM recommender_transformation"
            " AS t INNER JOIN recommender_rt AS rt ON t.id = rt.tid WHERE "
            "rt.rid = %d", r->id);
        OUTPUT_VERBOSE((10, "      SQL: %s", _CYAN(sql)));

        if (SQLITE_OK != sqlite3_exec(globals.db, sql,
            accumulate_transformations, (void *)r, &error)) {
            fprintf(stderr, "Error: SQL error: %s\n", error);
            sqlite3_free(error);
            return PERFEXPERT_ERROR;
        }
    }
    return PERFEXPERT_SUCCESS;
}

/* accumulate_transformations */
static int accumulate_transformations(void *recommendation, int c, char **val,
    char **names) {
    recommendation_t *r = (recommendation_t *)recommendation;
    char sql[MAX_BUFFER_SIZE], *error = NULL;
    transformation_t *t = NULL;

    /* Create the transformation item */
    PERFEXPERT_ALLOC(transformation_t, t, sizeof(transformation_t));
    perfexpert_list_item_construct((perfexpert_list_item_t *)t);
    t->id = atoi(val[1]);
    PERFEXPERT_ALLOC(char, t->program, (strlen(val[0]) + 1));
    strncpy(t->program, val[0], strlen(val[0]));
    perfexpert_list_construct(&(t->patterns));

    OUTPUT_VERBOSE((7, "      [%d] %s", t->id, _GREEN(t->program)));
    OUTPUT_VERBOSE((4, "      [%d] %s", t->id, _YELLOW("selecting patterns")));

    /* Find the pattern recognizers available for this transformation */
    bzero(sql, MAX_BUFFER_SIZE);
    sprintf(sql, "SELECT p.recognizer, p.id FROM recommender_pattern AS p INNER"
        "JOIN recommender_tp AS tp ON p.id = tp.pid WHERE tp.tid = %d;", t->id);
    OUTPUT_VERBOSE((10, "         SQL: %s", _CYAN(sql)));

    if (SQLITE_OK != sqlite3_exec(globals.db, sql, accumulate_patterns,
        (void *)t, &error)) {
        fprintf(stderr, "Error: SQL error: %s\n", error);
        sqlite3_free(error);
        return PERFEXPERT_ERROR;
    }

    /* Add this transformation to the recommendation list */
    perfexpert_list_append((perfexpert_list_t *)&(r->transformations),
        (perfexpert_list_item_t *)t);

    return PERFEXPERT_SUCCESS;
}

/* accumulate_patterns */
static int accumulate_patterns(void *transformation, int c, char **val,
    char **names) {
    transformation_t *t = (transformation_t *)transformation;
    pattern_t *p = NULL;

    /* Create the pattern item */
    PERFEXPERT_ALLOC(pattern_t, p, sizeof(pattern_t));
    perfexpert_list_item_construct((perfexpert_list_item_t *)p);
    p->id = atoi(val[1]);
    PERFEXPERT_ALLOC(char, p->program, (strlen(val[0]) + 1));
    strncpy(p->program, val[0], strlen(val[0]));

    OUTPUT_VERBOSE((7, "         [%d] %s", p->id, _GREEN(p->program)));

    /* Add this pattern to the transformation list */
    perfexpert_list_append((perfexpert_list_t *)&(t->patterns),
        (perfexpert_list_item_t *)p);

    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
