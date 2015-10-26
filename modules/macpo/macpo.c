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
#include "common/perfexpert_fork.h"
#include "common/perfexpert_util.h"

/* macpo_instrument_all */
int macpo_instrument_all(void) {
    char *error = NULL, sql[MAX_BUFFER_SIZE];

    OUTPUT_VERBOSE((2, "%s", _BLUE("Adding MACPO instrumentation")));
    OUTPUT(("%s", _YELLOW("Adding MACPO instrumentation")));

    bzero(sql, MAX_BUFFER_SIZE);
    sprintf(sql, "SELECT name, file, line FROM hotspot WHERE perfexpert_id = "
        "%llu", globals.unique_id);

    if (SQLITE_OK != sqlite3_exec(globals.db, sql, macpo_instrument, NULL,
        &error)) {
        OUTPUT(("%s %s", _ERROR("SQL error"), error));
        sqlite3_free(error);
        return PERFEXPERT_ERROR;
    }

    return PERFEXPERT_SUCCESS;
}

/* macpo_instrument */
static int macpo_instrument(void *n, int c, char **val, char **names) {
    char *t = NULL, *argv[5], *name = val[0], *file = val[1], *line = val[2];
    test_t test;
    int rc;
    char *folder, *fullpath, *filename;


    
    if (PERFEXPERT_SUCCESS != perfexpert_util_file_exists(file)) {
        return PERFEXPERT_SUCCESS;
    }

    OUTPUT_VERBOSE((4, "   instrumenting %s@%s:%s", name, file, line));
    
    if (PERFEXPERT_SUCCESS != perfexpert_util_filename_only(file, &filename)) {
        return PERFEXPERT_SUCCESS;
    }
    
    if (strstr(filename, "unknown-file")){
        return PERFEXPERT_SUCCESS;
    }

    if (PERFEXPERT_SUCCESS != perfexpert_util_path_only(file, &folder)) {
        return PERFEXPERT_ERROR;
    }

    PERFEXPERT_ALLOC(char, fullpath, (strlen(globals.moduledir) + strlen(folder) + 1));
    sprintf(fullpath, "%s/%s", globals.moduledir, folder);

    perfexpert_util_make_path(fullpath);


    /* Remove everyting after the '.' (for OMP functions) */
    /*
    name = t;
    if (strstr(t, ".omp_fn.")) {
        strsep(&t, ".");
    }
    */

    if (strstr(name, ".omp_fun.")) {
        strsep (&name, ".");
    }

    argv[0] = "macpo";
    argv[1] = "" ;//"--macpo:no-compile";

    PERFEXPERT_ALLOC(char, argv[2],
        (strlen(globals.moduledir) + strlen(file) + 24));
    sprintf(argv[2], "--macpo:backup-filename=%s/%s", globals.moduledir, file);

    PERFEXPERT_ALLOC(char, argv[3], (strlen(name) + 25));
    if (0 == line) {
        sprintf(argv[3], "--macpo:instrument=%s", name);
    } else {
        sprintf(argv[3], "--macpo:instrument=%s:%s", name, line);
    }

    PERFEXPERT_ALLOC(char, argv[4], strlen(file));
    strcpy(argv[4], file);

    PERFEXPERT_ALLOC(char, test.output, (strlen(globals.moduledir) + strlen(name) + 14));
    sprintf(test.output, "%s/%s-macpo.output", globals.moduledir, name);
    //test.input = file;
    test.input = NULL;
    test.info = globals.program;

    OUTPUT(("COMMAND=[%s %s %s %s %s]", argv[0], argv[1], argv[2], argv[3], argv[4]));

    rc = perfexpert_fork_and_wait(&test, (char **)argv);

    PERFEXPERT_DEALLOC(test.output);
    PERFEXPERT_DEALLOC(argv[2]);
    PERFEXPERT_DEALLOC(argv[3]);
    PERFEXPERT_DEALLOC(argv[4]);
    PERFEXPERT_DEALLOC(fullpath);

    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
