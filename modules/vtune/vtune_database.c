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
    /*
    char *error = NULL, sql[MAX_BUFFER_SIZE];
    hpctoolkit_procedure_t *h = NULL;

    // For each hotspot in the list of hotspots...
    perfexpert_list_for(h, &(profile->hotspots), hpctoolkit_procedure_t) {
        bzero(sql, MAX_BUFFER_SIZE);
        if (PERFEXPERT_HOTSPOT_FUNCTION == h->type) {
            sprintf(sql, "INSERT INTO hpctoolkit_hotspot (perfexpert_id, "
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

            sprintf(sql, "INSERT INTO hpctoolkit_hotspot (perfexpert_id, "
                "profile, name, line, type, module, file, depth) VALUES ("
                "%llu, '%s', '%s', %d, %d, '%s', '%s', %d);", globals.unique_id,
            profile->name, l->procedure->name, l->line, l->type,
                l->procedure->module->name, l->procedure->file->name, l->depth);

            OUTPUT_VERBOSE((9, "  [%d] %s (%s@%s:%d) SQL: %s", h->id,
                _YELLOW("loop"), l->procedure->module->shortname,
                l->procedure->file->shortname, h->line, sql));
        }

        // Insert procedure in database
        if (SQLITE_OK != sqlite3_exec(globals.db, sql, NULL, NULL, &error)) {
            OUTPUT(("%s %s", _ERROR("SQL error"), error));
            sqlite3_free(error);
            return PERFEXPERT_ERROR;
        }

        // Do the same with its metrics
        if (PERFEXPERT_SUCCESS != database_metrics(h)) {
            OUTPUT(("%s (%s)", _ERROR("writing hotspot"),
                h->name));
            return PERFEXPERT_ERROR;
        }
    }
    */
    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
