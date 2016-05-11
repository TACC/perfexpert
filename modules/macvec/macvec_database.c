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
#include "macvec.h"
#include "macvec_types.h"
#include "macvec_database.h"

/* PerfExpert common headers */
#include "common/perfexpert_alloc.h"
#include "common/perfexpert_constants.h"
#include "common/perfexpert_database.h"
#include "common/perfexpert_hash.h"
#include "common/perfexpert_list.h"
#include "common/perfexpert_md5.h"
#include "common/perfexpert_output.h"

int init_macvec_results() {
    char *error, sql[MAX_BUFFER_SIZE];
    bzero(sql, MAX_BUFFER_SIZE);
    sprintf(sql, " CREATE TABLE IF NOT EXISTS macvec_analysis ("
                 " id         INTEGER PRIMARY KEY,             "
                 " line       INTEGER,                         "
                 " filename   VARCHAR,                         "
                 " analysis   VARCHAR);");
    OUTPUT_VERBOSE((10, "      SQL: %s", sql));
    if (SQLITE_OK != sqlite3_exec(globals.db, sql, NULL, NULL, &error)) {
        OUTPUT(("%s %s", _ERROR("SQL error"), error));
        sqlite3_free(error);
        return PERFEXPERT_ERROR;
    }
    return PERFEXPERT_SUCCESS;
}

/* Store one result of the analysis in the DB */
int store_result(int line, char *filename, char *analysis) {
    char *error = NULL, sql[MAX_BUFFER_SIZE];
    
    bzero(sql, MAX_BUFFER_SIZE);
    /* By inserting a NULL into the primary key we are autoincrementing that id */
    sprintf(sql, "INSERT INTO macvec_analysis(id, line, filename, analysis) "
                 "VALUES(NULL, %d, '%s', '%s');",
                 line, filename, analysis);
    OUTPUT_VERBOSE((10, "      SQL: %s", sql));
    if (SQLITE_OK != sqlite3_exec(globals.db, sql, NULL, NULL, &error)) {
        OUTPUT(("%s %s %s", _ERROR("SQL error"), error, sql));
        sqlite3_free(error);
        return PERFEXPERT_ERROR;
    }
    return PERFEXPERT_SUCCESS;
}

int list_files_hotspots(perfexpert_list_t *files) {
    char *error = NULL, sql[MAX_BUFFER_SIZE];

    bzero(sql, MAX_BUFFER_SIZE);
    sprintf(sql,
        "SELECT DISTINCT(file) FROM "
        "perfexpert_hotspot WHERE perfexpert_id = %llu;", globals.unique_id);

    if (SQLITE_OK != sqlite3_exec(globals.db, sql, import_filenames,
        (void *)files, &error)) {
        OUTPUT(("%s %s", _ERROR("SQL error"), error));
        sqlite3_free(error);
        return PERFEXPERT_ERROR;
    }

    return PERFEXPERT_SUCCESS;
}

/* This function creates a list of the source filenames that contain
 * hotspots and that are writable (we don't analyze external headers,
 * system files,... The files are recompiled with the vectorization
 * report flags so that they can be later on analyzed */
static int import_filenames(void *files, int n, char **val, char **names) {
    char_t *file = NULL;

    /*  Only care about writable files */   
    if (perfexpert_util_file_is_writable(val[0]) == PERFEXPERT_ERROR) {
        return PERFEXPERT_SUCCESS;
    }

    PERFEXPERT_ALLOC(char_t, file, sizeof(char_t));
    perfexpert_list_item_construct((perfexpert_list_item_t *)file);
    PERFEXPERT_ALLOC(char, file->name, (strlen(val[0]) + 1));
    strcpy(file->name, val[0]);
    perfexpert_list_append((perfexpert_list_t *)files,
        (perfexpert_list_item_t *)file);

    return PERFEXPERT_SUCCESS;
}

/* database_import */
int database_import(perfexpert_list_t *profiles, char *filename) {
    macvec_profile_t *p = NULL;
    macvec_hotspot_t *h = NULL;

    /* Select and import profiles */
    OUTPUT_VERBOSE((5, "%s", _YELLOW("Importing profiles")));
    if (PERFEXPERT_SUCCESS != select_profiles(profiles, filename)) {
        OUTPUT(("%s", _ERROR("importing profiles")));
    }

    /* For each profile, select and import its modules */
    perfexpert_list_for(p, profiles, macvec_profile_t) {
        OUTPUT_VERBOSE((5, "%s", _YELLOW("Importing modules")));
        if (PERFEXPERT_SUCCESS != select_modules(p, filename)) {
            OUTPUT(("%s", _ERROR("importing modules")));
        }

        /* Select and import hotspots */
        OUTPUT_VERBOSE((5, "%s", _YELLOW("Importing hotspots")));
        if (PERFEXPERT_SUCCESS != select_hotspots(&(p->hotspots), filename)) {
            OUTPUT(("%s", _ERROR("importing hotspots")));
        }

        /* Map modules to hotspots */
        OUTPUT_VERBOSE((5, "%s", _YELLOW("Mapping hotspots <-> modules")));
        perfexpert_list_for(h, &(p->hotspots), macvec_hotspot_t) {
            if (PERFEXPERT_SUCCESS != map_modules_to_hotspots(h,
                p->modules_by_name)) {
                OUTPUT(("%s", _ERROR("mapping hotspots <-> modules")));
            }
        }

        /* Calculate metadata (cycles, instructions, relevance, and variance) */
        OUTPUT_VERBOSE((5, "%s", _YELLOW("Calculating metadata")));
        if (PERFEXPERT_SUCCESS != calculate_metadata(p)) {
            OUTPUT(("%s", _ERROR("calculating metadata")));
            return PERFEXPERT_ERROR;
        }
    }

    return PERFEXPERT_SUCCESS;
}

/* select_profiles */
static int select_profiles(perfexpert_list_t *profiles, char *filename) {
    char *error = NULL, sql[MAX_BUFFER_SIZE];

    bzero(sql, MAX_BUFFER_SIZE);
    sprintf(sql,
        "SELECT DISTINCT profile FROM perfexpert_hotspot WHERE perfexpert_id = %llu and file = '%s';",
        globals.unique_id, filename);

    OUTPUT_VERBOSE((9, "sql: %s", sql));

    if (SQLITE_OK != sqlite3_exec(globals.db, sql, import_profiles,
        (void *)profiles, &error)) {
        OUTPUT(("%s %s", _ERROR("SQL error"), error));
        sqlite3_free(error);
        return PERFEXPERT_ERROR;
    }

    return PERFEXPERT_SUCCESS;
}

/* import_profiles */
static int import_profiles(void *profiles, int n, char **val, char **names) {
    macvec_profile_t *profile = NULL;

    PERFEXPERT_ALLOC(macvec_profile_t, profile, sizeof(macvec_profile_t));
    perfexpert_list_item_construct((perfexpert_list_item_t *)profile);
    PERFEXPERT_ALLOC(char, profile->name, (strlen(val[0]) + 1));
    strcpy(profile->name, val[0]);
    profile->instructions = 0.0;
    profile->cycles = 0.0;
    profile->modules_by_name = NULL;
    perfexpert_list_construct(&(profile->hotspots));
    perfexpert_list_append((perfexpert_list_t *)profiles,
        (perfexpert_list_item_t *)profile);

    OUTPUT_VERBOSE((7, "   %s", _CYAN(profile->name)));

    return PERFEXPERT_SUCCESS;
}

/* select_modules */
static int select_modules(macvec_profile_t *profile, char * filename) {
    char *error = NULL, sql[MAX_BUFFER_SIZE];

    bzero(sql, MAX_BUFFER_SIZE);
    sprintf(sql,
        "SELECT DISTINCT module FROM perfexpert_hotspot WHERE perfexpert_id = %llu and file = '%s';",
        globals.unique_id, filename);

    OUTPUT_VERBOSE((9, "sql: %s", sql));
    
    if (SQLITE_OK != sqlite3_exec(globals.db, sql, import_modules,
        (void *)profile, &error)) {
        OUTPUT(("%s %s", _ERROR("SQL error"), error));
        sqlite3_free(error);
        return PERFEXPERT_ERROR;
    }

    return PERFEXPERT_SUCCESS;
}

/* import_modules */
static int import_modules(void *profile, int n, char **val, char **names) {
    macvec_module_t *module = NULL;

    PERFEXPERT_ALLOC(macvec_module_t, module, sizeof(macvec_module_t));
    PERFEXPERT_ALLOC(char, module->name, (strlen(val[0]) + 1));
    strcpy(module->name, val[0]);
    strcpy(module->name_md5, perfexpert_md5_string(val[0]));
    module->instructions = 0.0;
    module->importance = 0.0;
    module->cycles = 0.0;
    perfexpert_hash_add_str(((macvec_profile_t *)profile)->modules_by_name,
        name_md5, module);

    OUTPUT_VERBOSE((7, "   %s", _CYAN(module->name)));

    return PERFEXPERT_SUCCESS;
}

/* select_hotspots */
static int select_hotspots(perfexpert_list_t *hotspots, char *filename) {
    char *error = NULL, sql[MAX_BUFFER_SIZE];

    bzero(sql, MAX_BUFFER_SIZE);
    sprintf(sql,
        "SELECT id, name, type, profile, module, file, line, depth FROM "
        "perfexpert_hotspot WHERE perfexpert_id = %llu AND file = '%s';", globals.unique_id, filename);

    OUTPUT_VERBOSE((9, "sql: %s", sql));

    if (SQLITE_OK != sqlite3_exec(globals.db, sql, import_hotspots,
        (void *)hotspots, &error)) {
        OUTPUT(("%s %s", _ERROR("SQL error"), error));
        sqlite3_free(error);
        return PERFEXPERT_ERROR;
    }

    return PERFEXPERT_SUCCESS;
}

/* import_hotspots */
static int import_hotspots(void *hotspots, int n, char **val, char **names) {
    macvec_hotspot_t *hotspot = NULL;

    PERFEXPERT_ALLOC(macvec_hotspot_t, hotspot, sizeof(macvec_hotspot_t));
    perfexpert_list_item_construct((perfexpert_list_item_t *)hotspot);
    hotspot->id = atoi(val[0]);
    PERFEXPERT_ALLOC(char, hotspot->name, (strlen(val[1]) + 1));
    strcpy(hotspot->name, val[1]);
    hotspot->type = strtoll(val[2], NULL, 10);
    hotspot->module = NULL;
    PERFEXPERT_ALLOC(char, hotspot->file, (strlen(val[5]) + 1));
    strcpy(hotspot->file, val[5]);
    hotspot->line = atoi(val[6]);
    hotspot->depth = atoi(val[7]);
    hotspot->experiments = 0;
    hotspot->max_inst = DBL_MIN;
    hotspot->min_inst = DBL_MAX;
    hotspot->instructions = 0.0;
    hotspot->importance = 0.0;
    hotspot->variance = 0.0;
    hotspot->cycles = 0.0;
    perfexpert_list_append((perfexpert_list_t *)hotspots,
        (perfexpert_list_item_t *)hotspot);

    OUTPUT_VERBOSE((7, "   %s %s:%d", _CYAN(hotspot->name), hotspot->file,
        hotspot->line));

    return PERFEXPERT_SUCCESS;
}

/* map_modules_to_hotspots */
static int map_modules_to_hotspots(macvec_hotspot_t *h, macvec_module_t *db) {
    char *module_name = NULL, *error = NULL, sql[MAX_BUFFER_SIZE];

    bzero(sql, MAX_BUFFER_SIZE);
    sprintf(sql, "SELECT module FROM perfexpert_hotspot WHERE perfexpert_id = %llu AND "
        "name = '%s';", globals.unique_id, h->name);

    OUTPUT_VERBOSE((9, "sql: %s", sql));

    if (SQLITE_OK != sqlite3_exec(globals.db, sql,
        perfexpert_database_get_string, (void *)&module_name, &error)) {
        OUTPUT(("%s %s", _ERROR("SQL error"), error));
        sqlite3_free(error);
        return PERFEXPERT_ERROR;
    }

    perfexpert_hash_find_str(db, perfexpert_md5_string(module_name), h->module);
    PERFEXPERT_DEALLOC(module_name);
    if (NULL == h->module) {
        OUTPUT(("%s", _ERROR("module not found")));
        return PERFEXPERT_ERROR;
    }

    OUTPUT_VERBOSE((7, "   %s -> %s", _CYAN(h->name), h->module->name));

    return PERFEXPERT_SUCCESS;
}

/* calculate_metadata */
static int calculate_metadata(macvec_profile_t *profile) {
    char *error = NULL, sql[MAX_BUFFER_SIZE], *total_cycles, *total_inst;
    macvec_module_t *m = NULL, *t = NULL;
    macvec_hotspot_t *h = NULL;

    /* Replace the '.' by '_' on the metrics name, this is bullshit... */
    PERFEXPERT_ALLOC(char, total_cycles,
        (strlen(my_module_globals.measurement->total_cycles_counter) + 1));
    strcpy(total_cycles, my_module_globals.measurement->total_cycles_counter);
    perfexpert_string_replace_char(total_cycles, '.', '_');

    PERFEXPERT_ALLOC(char, total_inst,
        (strlen(my_module_globals.measurement->total_inst_counter) + 1));
    strcpy(total_inst, my_module_globals.measurement->total_inst_counter);
    perfexpert_string_replace_char(total_inst, '.', '_');

    /* For each hotspot... */
    perfexpert_list_for(h, &(profile->hotspots), macvec_hotspot_t) {
        /* Import instructions (sum of all experiments) */
        bzero(sql, MAX_BUFFER_SIZE);
        sprintf(sql, "SELECT SUM(value) FROM perfexpert_event WHERE hotspot_id = %llu "
            "AND name = '%s' GROUP BY experiment;", h->id, total_inst);

        if (SQLITE_OK != sqlite3_exec(globals.db, sql, import_instructions,
            (void *)h, &error)) {
            OUTPUT(("%s %s", _ERROR("SQL error"), error));
            sqlite3_free(error);
            return PERFEXPERT_ERROR;
        }

        /* Import maximum instructions */
        bzero(sql, MAX_BUFFER_SIZE);
        sprintf(sql,
            "SELECT MAX(a) FROM (SELECT SUM(value) AS a FROM perfexpert_event WHERE "
            "hotspot_id = %llu AND name = '%s');", h->id, total_inst);

        if (SQLITE_OK != sqlite3_exec(globals.db, sql,
            perfexpert_database_get_double, (void *)&(h->max_inst), &error)) {
            OUTPUT(("%s %s", _ERROR("SQL error"), error));
            sqlite3_free(error);
            return PERFEXPERT_ERROR;
        }

        /* Import minimum instructions */
        bzero(sql, MAX_BUFFER_SIZE);
        sprintf(sql,
            "SELECT MIN(a) FROM (SELECT SUM(value) AS a FROM perfexpert_event WHERE "
            "hotspot_id = %llu AND name = '%s');", h->id, total_inst);

        if (SQLITE_OK != sqlite3_exec(globals.db, sql,
            perfexpert_database_get_double, (void *)&(h->min_inst), &error)) {
            OUTPUT(("%s %s", _ERROR("SQL error"), error));
            sqlite3_free(error);
            return PERFEXPERT_ERROR;
        }

        /* Import number of experiments */
        bzero(sql, MAX_BUFFER_SIZE);
        sprintf(sql, "SELECT DISTINCT experiment FROM perfexpert_event WHERE "
            "hotspot_id = %llu;", h->id);

        if (SQLITE_OK != sqlite3_exec(globals.db, sql, import_experiment,
            (void *)h, &error)) {
            OUTPUT(("%s %s", _ERROR("SQL error"), error));
            sqlite3_free(error);
            return PERFEXPERT_ERROR;
        }

        /* Import cycles */
        bzero(sql, MAX_BUFFER_SIZE);
        sprintf(sql, "SELECT SUM(value) FROM perfexpert_event WHERE hotspot_id = %llu "
            "AND name = '%s';", h->id, total_cycles);

        if (SQLITE_OK != sqlite3_exec(globals.db, sql,
            perfexpert_database_get_double, (void *)&(h->cycles), &error)) {
            OUTPUT(("%s %s", _ERROR("SQL error"), error));
            sqlite3_free(error);
            return PERFEXPERT_ERROR;
        }

        /* Do the math... */
        if ((DBL_MIN != h->max_inst) &&
            (DBL_MAX != h->min_inst)) {
            h->variance = (h->max_inst - h->min_inst) / h->max_inst;
            h->instructions = h->instructions / h->experiments;
            h->module->instructions += h->instructions;
            profile->instructions += h->instructions;
            h->module->cycles += h->cycles;
            profile->cycles += h->cycles;
        }
    }
    if (profile->cycles <= 0.0 || profile->instructions <= 0.0)
        return PERFEXPERT_SUCCESS;
    
    OUTPUT_VERBOSE((3, "total # of instructions: [%f]", profile->instructions));
    OUTPUT_VERBOSE((3, "total # of cycles: [%f]", profile->cycles));

    /* Calculate importance for hotspots */
    perfexpert_list_for(h, &(profile->hotspots), macvec_hotspot_t) {
        h->importance = h->cycles / profile->cycles;
        OUTPUT_VERBOSE((5, "  [ins=%.2g] [var=%.2g] [cyc=%.2g] [imp=%.2f%%] %s",
            h->instructions, h->variance, h->cycles, h->importance * 100,
            _CYAN(h->name)));

        if (h->importance <= 0.0) {
            continue;
        }
        /* Write the importance back to the database */
        bzero(sql, MAX_BUFFER_SIZE);
        sprintf(sql, "UPDATE perfexpert_hotspot SET relevance = %f WHERE "
            "id = %llu;", h->importance, h->id);

        if (SQLITE_OK != sqlite3_exec(globals.db, sql, NULL, NULL, &error)) {
            OUTPUT(("%s %s", _ERROR("SQL error"), error));
            sqlite3_free(error);
            OUTPUT(("SQL: %s", sql));
            return PERFEXPERT_ERROR;
        }
    }

    /* Calculate importance for modules */
    perfexpert_hash_iter_str(profile->modules_by_name, m, t) {
        m->importance = m->cycles / profile->cycles;
        OUTPUT_VERBOSE((5, "  [ins=%.2g] [cyc=%.2g] [imp=%.2f%%] %s",
            m->instructions, m->cycles, m->importance * 100, _GREEN(m->name)));
    }

    OUTPUT_VERBOSE((5, "  [ins=%.2g] [cyc=%.2g] %s", profile->instructions,
        profile->cycles, _MAGENTA(profile->name)));

    return PERFEXPERT_SUCCESS;
}

/* import_instructions */
static int import_instructions(void *hotspot, int n, char **val, char **names) {
    ((macvec_hotspot_t *)hotspot)->instructions += atof(val[0]);

    return PERFEXPERT_SUCCESS;
}

/* import_experiment */
static int import_experiment(void *hotspot, int n, char **val, char **names) {
    ((macvec_hotspot_t *)hotspot)->experiments++;

    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
