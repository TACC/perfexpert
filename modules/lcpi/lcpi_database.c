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
#include "lcpi.h"
#include "lcpi_types.h"
#include "lcpi_database.h"

/* PerfExpert common headers */
#include "common/perfexpert_alloc.h"
#include "common/perfexpert_constants.h"
#include "common/perfexpert_database.h"
#include "common/perfexpert_hash.h"
#include "common/perfexpert_list.h"
#include "common/perfexpert_md5.h"
#include "common/perfexpert_output.h"

/* database_export */
int database_export(perfexpert_list_t *profiles, const char *table) {
    char *error = NULL, sql[MAX_BUFFER_SIZE];
    lcpi_metric_t *m = NULL, *t = NULL;
    lcpi_profile_t *p = NULL;
    lcpi_hotspot_t *h = NULL;

    OUTPUT_VERBOSE((5, "%s", _YELLOW("Exporting metrics")));

    /* Check if the required tables are available */
    bzero(sql, MAX_BUFFER_SIZE);
    sprintf(sql, "PRAGMA foreign_keys = ON; "
                 "CREATE TABLE IF NOT EXISTS lcpi_metric (  "
                 "id            INTEGER PRIMARY KEY,        "
                 "name          VARCHAR NOT NULL,           "
                 "value         REAL    NOT NULL,           "
                 "mpi_task      INTEGER NOT NULL,           "
                 "thread        INTEGER NOT NULL,           "
                 "hotspot_id    INTEGER NOT NULL,           "
                 "FOREIGN KEY (hotspot_id) REFERENCES perfexpert_hotspot(id));");

    OUTPUT_VERBOSE((5, "%s", _BLUE("Writing profiles to database")));

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

    /* Write to the database... */
    perfexpert_list_for(p, profiles, lcpi_profile_t) {
        OUTPUT_VERBOSE((8, "  %s", _GREEN(p->name)));
        perfexpert_list_for(h, &(p->hotspots), lcpi_hotspot_t) {
            /* ...the metrics */
            perfexpert_hash_iter_str(h->metrics_by_name, m, t) {
                bzero(sql, MAX_BUFFER_SIZE);
                sprintf(sql, "INSERT INTO lcpi_metric (name, value, mpi_task, thread, hotspot_id)"
                    " VALUES ('%s', %f, %d, %d, %llu);", m->name,
                    isnormal(m->value) ? m->value: 0.0, m->mpi_task, m->thread_id, h->id);

                OUTPUT_VERBOSE((10, "      %s SQL: %s", _CYAN(m->name), sql));

                if (SQLITE_OK != sqlite3_exec(globals.db, sql, NULL, NULL,
                    &error)) {
                    OUTPUT(("%s %s", _ERROR("SQL error"), error));
                    sqlite3_free(error);
                    return PERFEXPERT_ERROR;
                }
            }
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
int database_import(perfexpert_list_t *profiles, const char *table) {
    lcpi_profile_t *p = NULL;
    lcpi_hotspot_t *h = NULL;

    /* Select and import profiles */
    OUTPUT_VERBOSE((5, "%s [%s]", _YELLOW("Importing profiles"), table));
    if (PERFEXPERT_SUCCESS != select_profiles(profiles, table)) {
        OUTPUT(("%s", _ERROR("importing profiles")));
    }

    /* For each profile, select and import its modules */
    perfexpert_list_for(p, profiles, lcpi_profile_t) {
        OUTPUT_VERBOSE((5, "%s [%s]", _YELLOW("Importing modules"), table));
        if (PERFEXPERT_SUCCESS != select_modules(p, table)) {
            OUTPUT(("%s", _ERROR("importing modules")));
        }

        /* Select and import hotspots */
        OUTPUT_VERBOSE((5, "%s [%s]", _YELLOW("Importing hotspots"), table));
        if (PERFEXPERT_SUCCESS != select_hotspots(&(p->hotspots), table)) {
            OUTPUT(("%s", _ERROR("importing hotspots")));
        }

        /* Map modules to hotspots */
        OUTPUT_VERBOSE((5, "%s [%s]", _YELLOW("Mapping hotspots <-> modules"), table));
        perfexpert_list_for(h, &(p->hotspots), lcpi_hotspot_t) {
            if (PERFEXPERT_SUCCESS != map_modules_to_hotspots(h,
                p->modules_by_name, table)) {
                OUTPUT(("%s", _ERROR("mapping hotspots <-> modules")));
            }
        }

        /* Calculate metadata (cycles, instructions, relevance, and variance) */
        OUTPUT_VERBOSE((5, "%s [%s]", _YELLOW("Calculating metadata"), table));
        if (PERFEXPERT_SUCCESS != calculate_metadata(p, table)) {
            OUTPUT(("%s", _ERROR("calculating metadata")));
            return PERFEXPERT_ERROR;
        }
    }

    return PERFEXPERT_SUCCESS;
}

/* select_profiles */
static int select_profiles(perfexpert_list_t *profiles, const char *table) {
    char *error = NULL, sql[MAX_BUFFER_SIZE];

    bzero(sql, MAX_BUFFER_SIZE);
    sprintf(sql,
        "SELECT DISTINCT profile FROM perfexpert_hotspot WHERE perfexpert_id = %llu;",
        globals.unique_id);

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
    lcpi_profile_t *profile = NULL;

    PERFEXPERT_ALLOC(lcpi_profile_t, profile, sizeof(lcpi_profile_t));
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
static int select_modules(lcpi_profile_t *profile, const char *table) {
    char *error = NULL, sql[MAX_BUFFER_SIZE];

    bzero(sql, MAX_BUFFER_SIZE);
    sprintf(sql,
        "SELECT DISTINCT module FROM perfexpert_hotspot WHERE perfexpert_id = %llu;",
        globals.unique_id);

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
    lcpi_module_t *module = NULL;

    PERFEXPERT_ALLOC(lcpi_module_t, module, sizeof(lcpi_module_t));
    PERFEXPERT_ALLOC(char, module->name, (strlen(val[0]) + 1));
    strcpy(module->name, val[0]);
    strcpy(module->name_md5, perfexpert_md5_string(val[0]));
    module->instructions = 0.0;
    module->importance = 0.0;
    module->cycles = 0.0;
    perfexpert_hash_add_str(((lcpi_profile_t *)profile)->modules_by_name,
        name_md5, module);

    OUTPUT_VERBOSE((7, "   %s", _CYAN(module->name)));

    return PERFEXPERT_SUCCESS;
}

/* select_hotspots */
static int select_hotspots(perfexpert_list_t *hotspots, const char *table) {
    char *error = NULL, sql[MAX_BUFFER_SIZE];

    bzero(sql, MAX_BUFFER_SIZE);
    sprintf(sql,
        "SELECT id, name, type, profile, module, file, line, depth FROM "
        "perfexpert_hotspot WHERE perfexpert_id = %llu;", globals.unique_id);

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
    lcpi_hotspot_t *hotspot = NULL;

    PERFEXPERT_ALLOC(lcpi_hotspot_t, hotspot, sizeof(lcpi_hotspot_t));
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
    hotspot->metrics_by_name = NULL;
    perfexpert_list_append((perfexpert_list_t *)hotspots,
        (perfexpert_list_item_t *)hotspot);

    OUTPUT_VERBOSE((7, "   %s %s:%d", _CYAN(hotspot->name), hotspot->file,
        hotspot->line));

    return PERFEXPERT_SUCCESS;
}

/* map_modules_to_hotspots */
static int map_modules_to_hotspots(lcpi_hotspot_t *h, lcpi_module_t *db,
    const char *table) {
    char *module_name = NULL, *error = NULL, sql[MAX_BUFFER_SIZE];

    bzero(sql, MAX_BUFFER_SIZE);
    sprintf(sql, "SELECT module FROM perfexpert_hotspot WHERE perfexpert_id = %llu AND "
        "name = '%s';", globals.unique_id, h->name);

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
static int calculate_metadata(lcpi_profile_t *profile, const char *table) {
    char *error = NULL, sql[MAX_BUFFER_SIZE], *total_cycles, *total_inst;
    lcpi_module_t *m = NULL, *t = NULL;
    lcpi_hotspot_t *h = NULL;

    //OJO hardcoded for MIC
/*    if (0 == strcmp("jaketown",
        perfexpert_string_to_lower(my_module_globals.architecture))) {
        my_module_globals.measurement->total_cycles_counter="CPU_CLK_UNHALTED.THREAD_P";
        my_module_globals.measurement->total_inst_counter="INST_RETIRED.ANY_P";
    }
*/
    /* MIC (or KnightsCorner) */
/*    else if (0 == strcmp("mic",
        perfexpert_string_to_lower(my_module_globals.architecture))) {
        my_module_globals.measurement->total_cycles_counter = "CPU_CLK_UNHALTED";
        my_module_globals.measurement->total_inst_counter = "INSTRUCTIONS_EXECUTED";
    }
*/
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
    perfexpert_list_for(h, &(profile->hotspots), lcpi_hotspot_t) {
        /* Import instructions (sum of all experiments) */
        bzero(sql, MAX_BUFFER_SIZE);
        sprintf(sql, "SELECT SUM(value) FROM perfexpert_event WHERE hotspot_id = %llu "
            "AND name = '%s' GROUP BY experiment;", h->id, total_inst);

        OUTPUT_VERBOSE((8, "importing instructions %s", sql));

        if (SQLITE_OK != sqlite3_exec(globals.db, sql, import_instructions,
            (void *)h, &error)) {
            OUTPUT(("%s %s \n%s", _ERROR("SQL error"), error, sql));
            sqlite3_free(error);
            return PERFEXPERT_ERROR;
        }

        /* Import maximum instructions */
        bzero(sql, MAX_BUFFER_SIZE);
        sprintf(sql,
            "SELECT MAX(a) FROM (SELECT SUM(value) AS a FROM perfexpert_event WHERE "
            "hotspot_id = %llu AND name = '%s');", h->id, total_inst);

        OUTPUT_VERBOSE((8, "importing max instructions: %s", sql));

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

        OUTPUT_VERBOSE((8, "importing min instructions: %s", sql));

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

        OUTPUT_VERBOSE((8, "importing experiments: %s", sql));

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

        OUTPUT_VERBOSE((8, "importing cycles: %s", sql));

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
    OUTPUT_VERBOSE((3, "total # of instructions: [%f]", profile->instructions));
    OUTPUT_VERBOSE((3, "total # of cycles: [%f]", profile->cycles));

    /* Calculate importance for hotspots */
    perfexpert_list_for(h, &(profile->hotspots), lcpi_hotspot_t) {
        h->importance = h->cycles / profile->cycles;
        OUTPUT_VERBOSE((5, "  [ins=%.2g] [var=%.2g] [cyc=%.2g] [imp=%.2f%%] %s",
            h->instructions, h->variance, h->cycles, h->importance * 100,
            _CYAN(h->name)));

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
    ((lcpi_hotspot_t *)hotspot)->instructions += atof(val[0]);

    return PERFEXPERT_SUCCESS;
}

/* import_experiment */
static int import_experiment(void *hotspot, int n, char **val, char **names) {
    ((lcpi_hotspot_t *)hotspot)->experiments++;

    return PERFEXPERT_SUCCESS;
}

/* database_get_hound */
double database_get_hound(const char *name) {
    char *error = NULL, sql[MAX_BUFFER_SIZE];
    double value = -1.0;

    bzero(sql, MAX_BUFFER_SIZE);
    sprintf(sql,
        "SELECT value FROM hound WHERE name='%s' AND family=%d AND model=%d;",
        name, perfexpert_cpuinfo_get_family(), perfexpert_cpuinfo_get_model());

    if (SQLITE_OK != sqlite3_exec(globals.db, sql,
        perfexpert_database_get_double, (void *)&value, &error)) {
        OUTPUT(("%s %s", _ERROR("SQL error"), error));
        sqlite3_free(error);
    }

    return value;
}

/* database_get_event */
double database_get_event(const char *name, int hotspot_id, int mpi_task, int thread_id) {
    char *error = NULL, sql[MAX_BUFFER_SIZE];
    double value = -1.0;

    bzero(sql, MAX_BUFFER_SIZE);

    if (my_module_globals.output==SERIAL_OUTPUT) {
        sprintf(sql, "SELECT SUM(value) FROM perfexpert_event WHERE hotspot_id = %d AND "
            "name = '%s';", hotspot_id, name);
    }
    else {
        if (my_module_globals.output==PARALLEL_OUTPUT) {
            sprintf(sql, "SELECT SUM(value) FROM perfexpert_event WHERE hotspot_id = %d AND "
                "name = '%s' AND mpi_task=%d;", hotspot_id, name, mpi_task);
        }
        else {  // Hybrid output
            sprintf(sql, "SELECT SUM(value) FROM perfexpert_event WHERE hotspot_id = %d AND "
                "name = '%s' AND mpi_task=%d and thread_id=%d;", hotspot_id, name, mpi_task, thread_id);
        }
    }

    if (SQLITE_OK != sqlite3_exec(globals.db, sql,
        perfexpert_database_get_double, (void *)&value, &error)) {
        OUTPUT(("%s %s", _ERROR("SQL error"), error));
        sqlite3_free(error);
        return -1.0;
    }
    return value;
}

/*  return number of threads */
int database_get_threads () {
    char sql[MAX_BUFFER_SIZE];
    char *error;
    int value = 0;

    bzero(sql, MAX_BUFFER_SIZE);
    sprintf(sql,
            "SELECT threads FROM perfexpert_experiment WHERE perfexpert_id = %llu;",
            globals.unique_id);
    OUTPUT_VERBOSE((10, "importing threads: %s", sql));
    if (SQLITE_OK != sqlite3_exec(globals.db, sql,
        perfexpert_database_get_int, (int *)&value, &error)) {
        OUTPUT(("%s %s", _ERROR("SQL error"), error));
        sqlite3_free(error);
    }

    return value+1;   
}

/*  return number of MPI tasks */
int database_get_mpi_tasks () {
    char sql[MAX_BUFFER_SIZE];
    char *error;
    int value = 0;

    bzero(sql, MAX_BUFFER_SIZE);
    sprintf(sql,
            "SELECT mpi_tasks FROM perfexpert_experiment WHERE perfexpert_id = %llu;",
            globals.unique_id);
    OUTPUT_VERBOSE((10, "importing MPI tasks: %s", sql));
    if (SQLITE_OK != sqlite3_exec(globals.db, sql,
        perfexpert_database_get_int, (int *)&value, &error)) {
        OUTPUT(("%s %s", _ERROR("SQL error"), error));
        sqlite3_free(error);
    }

    return value+1;
}

static inline int process_hound_info(void *dump, int n, char **val, char **names) {
    lcpi_hound_t *h;
//    int i;
//    for (i = 0; i < n; ++i) {
//        OUTPUT(("PROCESSING %s = %s\n", val[0], val[1]));
        PERFEXPERT_ALLOC(lcpi_hound_t, h, sizeof(lcpi_hound_t));
        PERFEXPERT_ALLOC(char, h->name, strlen(val[0]));
        strcpy(h->name, val[0]);
        strcpy(h->name_md5, perfexpert_md5_string(val[0])); 
        h->value = strtol(val[1], NULL, 10);
//        OUTPUT(("ADDING %s", h->name));
        perfexpert_hash_add_str(my_module_globals.hound_info, name_md5, h);
//    }
    return PERFEXPERT_SUCCESS;
}

int import_hound(lcpi_hound_t *hound) {
    char sql[MAX_BUFFER_SIZE];
    char *error;
   
    bzero(sql, MAX_BUFFER_SIZE);
    sprintf(sql,
            "SELECT name, value FROM hound;");
    OUTPUT_VERBOSE((10, "importing hound information: %s", sql));
    if (SQLITE_OK != sqlite3_exec(globals.db, sql,
        process_hound_info, (lcpi_hound_t*) &hound, &error)) {
        OUTPUT(("%s %s", _ERROR("SQL error"), error));
        sqlite3_free(error);
        return PERFEXPERT_ERROR;
    }
    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
