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

/* PerfExpert headers */
#include "ct.h"
#include "perfexpert_alloc.h"
#include "perfexpert_constants.h"
#include "perfexpert_output.h"
#include "perfexpert_list.h"

/* select_transformations */
int select_transformations(fragment_t *fragment) {
    recommendation_t *recommendation = NULL;
    char *error_msg = NULL;
    char sql[BUFFER_SIZE];

    OUTPUT_VERBOSE((7, "%s",
        _BLUE("Looking for code transformers and code patterns")));

    /* For each recommendation in this fragment, select the transformers */
    recommendation = (recommendation_t *)perfexpert_list_get_first(
        &(fragment->recommendations));

    while ((perfexpert_list_item_t *)recommendation !=
        &(fragment->recommendations.sentinel)) {
        OUTPUT_VERBOSE((4, "   [%d] %s", recommendation->id,
            _YELLOW("selecting transformations")));

        /* Initialyze the list of transformations */
        perfexpert_list_construct(&(recommendation->transformations));

        /* Find the transformations available for this recommendation */
        bzero(sql, BUFFER_SIZE);
        sprintf(sql, "%s %s %s %d;",
            "SELECT t.transformer, t.id FROM transformation AS t",
            "INNER JOIN recommendation_transformation AS rt",
            "ON t.id = rt.id_transformation WHERE rt.id_recommendation =",
            recommendation->id);
        OUTPUT_VERBOSE((10, "      SQL: %s", _CYAN(sql)));

        if (SQLITE_OK != sqlite3_exec(globals.db, sql,
            accumulate_transformations, (void *)recommendation, &error_msg)) {
            fprintf(stderr, "Error: SQL error: %s\n", error_msg);
            sqlite3_free(error_msg);
            return PERFEXPERT_ERROR;
        }

        /* Move to the next code recommendation */
        recommendation = (recommendation_t *)perfexpert_list_get_next(
            (perfexpert_list_item_t *)recommendation);
    }
    return PERFEXPERT_SUCCESS;
}

/* accumulate_transformations */
int accumulate_transformations(void *recommendation, int count, char **val,
    char **names) {
    transformation_t *transformation = NULL;
    recommendation_t *r = NULL;
    char *error_msg = NULL;
    char sql[BUFFER_SIZE];

    /* Create the transformation item */
    PERFEXPERT_ALLOC(transformation_t, transformation,
        sizeof(transformation_t));
    perfexpert_list_item_construct((perfexpert_list_item_t *)transformation);
    transformation->id = atoi(val[1]);
    PERFEXPERT_ALLOC(char, transformation->program, (strlen(val[0]) + 1));
    strncpy(transformation->program, val[0], strlen(val[0]));
    perfexpert_list_construct(&(transformation->patterns));

    OUTPUT_VERBOSE((7, "      [%d] %s", transformation->id,
        _GREEN(transformation->program)));

    OUTPUT_VERBOSE((4, "      [%d] %s", transformation->id,
        _YELLOW("selecting patterns")));

    /* Find the pattern recognizers available for this transformation */
    bzero(sql, BUFFER_SIZE);
    sprintf(sql, "%s %s %s %d;",
        "SELECT p.recognizer, p.id FROM pattern AS p",
        "INNER JOIN transformation_pattern AS tp ON p.id = tp.id_pattern",
        "WHERE tp.id_transformation =", transformation->id);
    OUTPUT_VERBOSE((10, "         SQL: %s", _CYAN(sql)));

    if (SQLITE_OK != sqlite3_exec(globals.db, sql, accumulate_patterns,
        (void *)transformation, &error_msg)) {
        fprintf(stderr, "Error: SQL error: %s\n", error_msg);
        sqlite3_free(error_msg);
        return PERFEXPERT_ERROR;
    }

    /* Add this transformation to the recommendation list */
    r = (recommendation_t *)recommendation;
    perfexpert_list_append((perfexpert_list_t *)&(r->transformations),
        (perfexpert_list_item_t *)transformation);

    return PERFEXPERT_SUCCESS;
}

/* accumulate_patterns */
int accumulate_patterns(void *transformation, int count, char **val,
    char **names) {
    transformation_t *t = NULL;
    pattern_t *pattern = NULL;

    /* Create the pattern item */
    PERFEXPERT_ALLOC(pattern_t, pattern, sizeof(pattern_t));
    perfexpert_list_item_construct((perfexpert_list_item_t *)pattern);
    pattern->id = atoi(val[1]);
    PERFEXPERT_ALLOC(char, pattern->program, (strlen(val[0]) + 1));
    strncpy(pattern->program, val[0], strlen(val[0]));

    OUTPUT_VERBOSE((7, "         [%d] %s", pattern->id,
        _GREEN(pattern->program)));

    /* Add this pattern to the transformation list */
    t = (transformation_t *)transformation;
    perfexpert_list_append((perfexpert_list_t *)&(t->patterns),
        (perfexpert_list_item_t *)pattern);

    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
