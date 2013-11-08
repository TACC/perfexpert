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

/* Modules headers */
#include "macpo.h"

/* PerfExpert common headers */
#include "common/perfexpert_alloc.h"
#include "common/perfexpert_output.h"

/* macpo_inst_all */
int macpo_inst_all(void) {
    char *error = NULL, sql[MAX_BUFFER_SIZE];

    OUTPUT_VERBOSE((2, "%s", _BLUE("Adding MACPO instrumentation")));
    OUTPUT(("%s", _YELLOW("Adding MACPO instrumentation")));

    bzero(sql, MAX_BUFFER_SIZE);
    sprintf(sql, "SELECT name, file, line FROM hotspot WHERE perfexpert_id = "
        "%llu", globals.unique_id);

    if (SQLITE_OK != sqlite3_exec(globals.db, sql, macpo_inst, NULL, &error)) {
        OUTPUT(("%s %s", _ERROR("SQL error"), error));
        sqlite3_free(error);
        return PERFEXPERT_ERROR;
    }

    return PERFEXPERT_SUCCESS;
}

/* macpo_inst */
static int macpo_inst(void *n, int c, char **val, char **names) {
    char *t = NULL, *argv[4], *name = val[0], *file = val[1], *line = val[2];

    OUTPUT_VERBOSE((4, "   instrumenting %s@%s:%s", name, file, line));

    /* Remove everyting after the '.' (for OMP functions) */
    name = t;
    if (strstr(t, ".omp_fn.")) {
        strsep(&t, ".");
    }

    argv[0] = "macpo";
    argv[1] = "--nocompile";

    PERFEXPERT_ALLOC(char, argv[2],
        (strlen(globals.stepdir) + strlen(file) + 17));
    sprintf(argv[2], "--backup=%s/macpo/%s", globals.stepdir, file);

    PERFEXPERT_ALLOC(char, argv[3], (strlen(name) + 20));
    if (0 == line) {
        sprintf(argv[3], "--instrument=%s", name);
    } else {
        sprintf(argv[3], "--instrument=%s:%s", name, line);
    }

    OUTPUT(("COMMAND=[%s %s %s %s]", argv[0], argv[1], argv[2], argv[3]));

    PERFEXPERT_DEALLOC(argv[2]);
    PERFEXPERT_DEALLOC(argv[3]);

    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
