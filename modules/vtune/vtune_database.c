/*
 * Copyright (c) 2011-2015  University of Texas at Austin. All rights reserved.
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
#include "vtune_types.h"
#include "vtune_database.h"

/* PerfExpert common headers */
#include "common/perfexpert_alloc.h"
#include "common/perfexpert_constants.h"
#include "common/perfexpert_database.h"
#include "common/perfexpert_list.h"
#include "common/perfexpert_output.h"


/* database_hw_events */
static int database_hw_events(vtune_hw_profile_t *profile) {
    
    char *error = NULL, sql[MAX_BUFFER_SIZE];
    int id = 0; 
    const char * ext_id;

    vtune_event_t * v = NULL;
    perfexpert_list_for (v, profile->events_by_id, vtune_event_t) {
        bzero(sql, MAX_BUFFER_SIZE);
        sprintf (sql, "SELECT id FROM vtune_counter_type WHERE "
            "name='%s'", v->name);

        OUTPUT_VERBOSE((9, "  %s SQL: %s", _YELLOW(v->name), sql));
        
        if (SQLITE_OK != sqlite3_exec(globals.db, sql, NULL, (void*) ext_id, &error)) {
            OUTPUT(("%s %s", _ERROR("SQL error"), error));
            sqlite3_free(error);
            return PERFEXPERT_ERROR;
        }

        sprintf (sql, "INSERT INTO vtune_counters (perfexpert_id, "
            "id, samples, value, vtune_counters_id) VALUES " 
            "(%llu, %llu, %llu, %llu, %llu);",
            globals.unique_id, id, v->samples, v->value, ext_id);

        OUTPUT_VERBOSE((9, "  [%d] %s SQL: %s", id,
            _YELLOW(v->name),
            sql));
        
        // Insert procedure in database
        if (SQLITE_OK != sqlite3_exec(globals.db, sql, NULL, NULL, &error)) {
            OUTPUT(("%s %s", _ERROR("SQL error"), error));
            sqlite3_free(error);
            return PERFEXPERT_ERROR;
        }
        id++;
    }
    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
