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
#include <sqlite3.h>

/* Modules headers */
#include "hpctoolkit.h"
#include "hpctoolkit_types.h"
#include "hpctoolkit_database.h"

/* PerfExpert common headers */
#include "common/perfexpert_alloc.h"
#include "common/perfexpert_constants.h"
#include "common/perfexpert_database.h"
#include "common/perfexpert_list.h"
#include "common/perfexpert_output.h"

/* database_profiles */
int database_profiles(perfexpert_list_t *profiles) {
    hpctoolkit_profile_t *p = NULL;
    char *error = NULL;

    /* Check if the required tables are available */
    char sql[] = "PRAGMA foreign_keys = ON;             \
        CREATE TABLE IF NOT EXISTS perfexpert_hotspot ( \
            perfexpert_id INTEGER NOT NULL,             \
            id            INTEGER PRIMARY KEY,          \
            name          VARCHAR NOT NULL,             \
            type          INTEGER NOT NULL,             \
            profile       VARCHAR NOT NULL,             \
            module        VARCHAR NOT NULL,             \
            file          VARCHAR NOT NULL,             \
            line          INTEGER NOT NULL,             \
            depth         INTEGER NOT NULL,             \
            relevance     INTEGER);                     \
        CREATE TABLE IF NOT EXISTS perfexpert_event (   \
            id            INTEGER PRIMARY KEY,          \
            name          VARCHAR NOT NULL,             \
            thread_id     INTEGER NOT NULL,             \
            mpi_task      INTEGER NOT NULL,             \
            experiment    INTEGER NOT NULL,             \
            value         REAL    NOT NULL,             \
            hotspot_id    INTEGER NOT NULL,             \
        FOREIGN KEY (hotspot_id) REFERENCES perfexpert_hotspot(id));";

    OUTPUT_VERBOSE((5, "%s", _BLUE("Writing profiles to database")));

    if (SQLITE_OK != sqlite3_exec(globals.db, sql, NULL, NULL, &error)) {
        OUTPUT(("%s %s", _ERROR("SQL error"), error));
        sqlite3_free(error);
        return PERFEXPERT_ERROR;
    }

    /* For each profile in the list of profiles... */
    perfexpert_list_for(p, profiles, hpctoolkit_profile_t) {
        OUTPUT_VERBOSE((8, "[%d] %s", p->id, _GREEN(p->name)));
        if (PERFEXPERT_SUCCESS != database_hotspots(p)) {
            OUTPUT(("%s (%s)", _ERROR("writing profile"), p->name));
            return PERFEXPERT_ERROR;
        }
    }
    return PERFEXPERT_SUCCESS;
}

/* database_hotspots */
static int database_hotspots(hpctoolkit_profile_t *profile) {
    char *error = NULL, sql[MAX_BUFFER_SIZE];
    hpctoolkit_procedure_t *h = NULL;

    /* For each hotspot in the list of hotspots... */
    perfexpert_list_for(h, &(profile->hotspots), hpctoolkit_procedure_t) {
        bzero(sql, MAX_BUFFER_SIZE);
        if (PERFEXPERT_HOTSPOT_FUNCTION == h->type) {
            sprintf(sql, "INSERT INTO perfexpert_hotspot (perfexpert_id, "
                "profile, name, line, type, module, file, depth) VALUES "
                "(%llu, '%s', '%s', %d, %d, '%s', '%s', 0);",
                globals.unique_id, profile->name, h->name, h->line, h->type,
                h->module != NULL ? h->module->name : "---",
                h->file != NULL ? h->file->name : "---");

            OUTPUT_VERBOSE((9, "  [%d] %s (%s@%s:%d) SQL: %s", h->id,
                _YELLOW(h->name),
                h->module != NULL ? h->module->shortname : "---",
                h->file != NULL ? h->file->shortname : "---", h->line, sql));

        } else if (PERFEXPERT_HOTSPOT_LOOP == h->type) {
            hpctoolkit_loop_t *l = (hpctoolkit_loop_t *)h;

            sprintf(sql, "INSERT INTO perfexpert_hotspot (perfexpert_id, "
                "profile, name, line, type, module, file, depth) VALUES ("
                "%llu, '%s', '%s', %d, %d, '%s', '%s', %d);", globals.unique_id,
            profile->name, l->procedure->name, l->line, l->type,
                l->procedure->module->name, l->procedure->file->name, l->depth);

            OUTPUT_VERBOSE((9, "  [%d] %s (%s@%s:%d) SQL: %s", h->id,
                _YELLOW("loop"), l->procedure->module->shortname,
                l->procedure->file->shortname, h->line, sql));
        }

        /* Insert procedure in database */
        if (SQLITE_OK != sqlite3_exec(globals.db, sql, NULL, NULL, &error)) {
            OUTPUT(("%s %s", _ERROR("SQL error"), error));
            sqlite3_free(error);
            return PERFEXPERT_ERROR;
        }

        /* Do the same with its metrics */
        if (PERFEXPERT_SUCCESS != database_metrics(h)) {
            OUTPUT(("%s (%s)", _ERROR("writing hotspot"),
                h->name));
            return PERFEXPERT_ERROR;
        }
    }

    return PERFEXPERT_SUCCESS;
}

/* database_metrics */
static int database_metrics(hpctoolkit_procedure_t *hotspot) {
    char *error = NULL, sql[MAX_BUFFER_SIZE], *str;
    hpctoolkit_metric_t *m = NULL;
    long long int id = 0;

    /* Find the hotspot_id */
    bzero(sql, MAX_BUFFER_SIZE);
    sprintf(sql, "SELECT id FROM perfexpert_hotspot WHERE perfexpert_id = %llu "
        "ORDER BY id DESC LIMIT 1;", globals.unique_id);

    OUTPUT_VERBOSE((10, "    SQL: %s", sql));

    if (SQLITE_OK != sqlite3_exec(globals.db, sql,
        perfexpert_database_get_long_long_int, (void *)&id, &error)) {
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

    /* For each metric in the list of metrics... */
    perfexpert_list_for(m, &(hotspot->metrics), hpctoolkit_metric_t) {
        /* Replace the '.' by '_' on the metrics name, this is bullshit... */
        PERFEXPERT_ALLOC(char, str, (strlen(m->name) + 1));
        strcpy(str, m->name);
        perfexpert_string_replace_char(str, '.', '_');

        bzero(sql, MAX_BUFFER_SIZE);
        sprintf(sql, "INSERT INTO perfexpert_event (name, thread_id, mpi_task, "
            "experiment, value, hotspot_id) VALUES ('%s', %d, %d, %d, %f, %llu)"
            ";", str, m->thread, m->mpi_rank, m->experiment, m->value, id);

        OUTPUT_VERBOSE((10, "    [%d] %s SQL: %s", m->id, _CYAN(str), sql));

        /* Insert metric in database */
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

int database_set_tasks_threads() {
    char sql[MAX_BUFFER_SIZE];
    char *error;
    int mpi_tasks;
    int threads;
    
    bzero(sql, MAX_BUFFER_SIZE);
    sprintf(sql, "SELECT MAX(mpi_task) FROM perfexpert_event");

    OUTPUT_VERBOSE((10, "    SQL: %s", sql));
    if (SQLITE_OK != sqlite3_exec(globals.db, sql,
        perfexpert_database_get_int, (void *)&mpi_tasks, &error)) {
        OUTPUT(("%s %s", _ERROR("SQL error"), error));
        sqlite3_free(error);
        return PERFEXPERT_ERROR;
    }

    bzero(sql, MAX_BUFFER_SIZE);
    sprintf(sql, "SELECT MAX(thread_id) FROM perfexpert_event");

    OUTPUT_VERBOSE((10, "    SQL: %s", sql));
    if (SQLITE_OK != sqlite3_exec(globals.db, sql,
        perfexpert_database_get_int, (void *)&threads, &error)) {
        OUTPUT(("%s %s", _ERROR("SQL error"), error));
        sqlite3_free(error);
        return PERFEXPERT_ERROR;
    }

    bzero(sql, MAX_BUFFER_SIZE);
    sprintf(sql, "UPDATE perfexpert_experiment SET"
            " mpi_tasks=%d, threads=%d WHERE perfexpert_id=%llu",
            mpi_tasks, threads, globals.unique_id);

    OUTPUT_VERBOSE((10, "    SQL: %s", sql));
    if (SQLITE_OK != sqlite3_exec(globals.db, sql, NULL, NULL, &error)) {
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
