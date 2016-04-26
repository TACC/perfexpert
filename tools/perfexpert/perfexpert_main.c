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
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <signal.h>
#include <time.h>

/* Utility headers */
#include <sqlite3.h>

/* Tools headers */
#include "perfexpert.h"
#include "perfexpert_types.h"

/* Modules headers */
#include "modules/perfexpert_module_base.h"

/* PerfExpert common headers */
#include "config.h"
#include "common/perfexpert_alloc.h"
#include "common/perfexpert_constants.h"
#include "common/perfexpert_database.h"
#include "common/perfexpert_output.h"
#include "common/perfexpert_signal.h"
#include "common/perfexpert_util.h"

/* Global variables, try to not create them! */
globals_t globals; // Variable to hold global options, this one is OK

/* main, life starts here */
int main(int argc, char** argv) {
    char *str = NULL, *cwd = NULL, template[33] = { 0 };
    int rc = PERFEXPERT_UNDEFINED, i = 0;
    perfexpert_module_t *m = NULL;
    perfexpert_step_t *s = NULL;
    struct tm *lt;
    time_t t;
    char *error;
    char sql[MAX_BUFFER_SIZE];
    char command[MAX_BUFFER_SIZE];

    /* Register the perfexpert signal handler */
    signal(SIGSEGV, perfexpert_sighandler);

    /* Set default values for globals */
    globals = (globals_t) {
        .verbose        = 0,                // int
        .colorful       = PERFEXPERT_FALSE, // int
        .dbfile         = NULL,             // *char
        .remove_garbage = PERFEXPERT_FALSE, // int
        .program        = NULL,             // *char
        .program_path   = NULL,             // *char
        .program_full   = NULL,             // *char
        .program_argv   = { 0 },            // *char[]
        .cycle          = 1,                // int
        .workdir        = NULL,             // *char
        .moduledir      = NULL,             // *char
        .unique_id      = 0,                // long long int
        .backup         = NULL              // perfexpert_backup_t
    };

    /* Parse command-line parameters */
    if (PERFEXPERT_SUCCESS != parse_cli_params(argc, argv)) {
        OUTPUT(("%s", _ERROR("parsing command line arguments")));
        goto CLEANUP;
    }

    OUTPUT_VERBOSE((4, "%s", _BLUE("Tool initialization")));

    /* Step 1: Define the unique identification for this PerfExpert session */
    PERFEXPERT_ALLOC(char, str, 20);
    sprintf(str, "%llu%06d", (long long)time(NULL), (int)getpid());
    globals.unique_id = strtoll(str, NULL, 10);
    PERFEXPERT_DEALLOC(str);
    OUTPUT_VERBOSE((5, "   %s %llu", _YELLOW("unique ID:"), globals.unique_id));

    /* Step 2: Define the work directory */
    time(&t);
    lt = localtime(&t);
    strftime(template, 33, ".perfexpert-%Y%m%d_%H%M_XXXXXX", lt);
    if (NULL == (str = mkdtemp(template))) {
        OUTPUT(("%s", _ERROR("while defining the working directory")));
        goto CLEANUP;
    }
    cwd = getcwd(cwd, 0);
    PERFEXPERT_ALLOC(char, globals.workdir, (strlen(cwd) + strlen(str) + 2));
    sprintf(globals.workdir, "%s/%s", cwd, str);
    PERFEXPERT_DEALLOC(cwd);
    if (NULL != globals.workdir) {
        OUTPUT_VERBOSE((5, "   %s %s", _YELLOW("workdir:"), globals.workdir));
    } else {
        OUTPUT(("%s", _ERROR("null globals.workdir")));
        goto CLEANUP;
    }

    /* Step 3: Check if the database file exists and is updated, then connect */
    if ((NULL != globals.dbfile) &&
        (PERFEXPERT_SUCCESS != perfexpert_util_file_exists(globals.dbfile))) {
        OUTPUT(("%s", _ERROR("database file not found")));
        goto CLEANUP;
    }
    if (PERFEXPERT_SUCCESS != perfexpert_database_update(&(globals.dbfile))) {
        OUTPUT(("%s", _ERROR("unable to update database")));
        goto CLEANUP;
    }
    if (NULL == (globals.dbfile)) {
        OUTPUT(("%s", _ERROR("NULL dbfile, something weird happened")));
        goto CLEANUP;
    } else {
        OUTPUT_VERBOSE((5, "   %s %s", _YELLOW("database:"), globals.dbfile));
    }
    if (PERFEXPERT_SUCCESS != perfexpert_database_connect(&(globals.db),
        globals.dbfile)) {
        OUTPUT(("%s", _ERROR("connecting to database")));
        goto CLEANUP;
    }


    i = 0;
    sprintf (command, "");
    while (NULL != globals.program_argv[i]) {
        strcat (command, globals.program_argv[i]);
        i++;
    }


    /* Step 4: Initialize modules */
    if (PERFEXPERT_SUCCESS != perfexpert_module_init()) {
        OUTPUT(("%s", _ERROR("unable to initialize modules")));
        goto CLEANUP;
    }

    printf("%s    %s", PROGRAM_PREFIX, _YELLOW("Modules: "));
    perfexpert_list_for(m, &(module_globals.modules), perfexpert_module_t) {
        printf(" [%s]", m->name);
    }
    printf("\n%s    %s", PROGRAM_PREFIX, _YELLOW("Workflow: "));
    perfexpert_list_for(s, &(module_globals.steps), perfexpert_step_t) {
        printf("[%s/%s] ", s->name, perfexpert_phase_name[s->phase]);
    }
    printf("\n");
    fflush(stdout);

    /* Step 5: Initialize the file backup */
    if (PERFEXPERT_SUCCESS != perfexpert_backup_create(&globals.backup)) {
        OUTPUT(("%s", _ERROR("unable to initialize backup")));
        goto CLEANUP;
    }

    bzero(sql, MAX_BUFFER_SIZE);
    sprintf(sql, "INSERT INTO perfexpert_experiment(perfexpert_id, command, mpi_tasks, threads)"
            " VALUES (%llu, '%s', 1, 1);", globals.unique_id, command);
    OUTPUT_VERBOSE((10, " sql: %s", sql));
    if (SQLITE_OK != sqlite3_exec(globals.db, sql, NULL, NULL, &error)) {
        OUTPUT(("%s %s", _ERROR("SQL error"), error));
        sqlite3_free(error);
        return PERFEXPERT_ERROR;
    }

    /* Step 6: Iterate through steps  */
    i = 0;
    OUTPUT_VERBOSE((4, "%s", _BLUE("Starting optimization workflow")));
    while (1) {
        /* Initialize modules */
        if (PERFEXPERT_SUCCESS != perfexpert_module_init()) {
            OUTPUT(("%s", _ERROR("unable to initialize modules")));
            goto CLEANUP;
        }

        /* Workflow progress */
        perfexpert_list_for(s, &(module_globals.steps), perfexpert_step_t) {
            OUTPUT_VERBOSE((4, "%s [%s/%s]", _GREEN("Workflow step:"),
                s->module->name, perfexpert_phase_name[s->phase]));

            /* Create module working directory */
            PERFEXPERT_ALLOC(char, globals.moduledir,
                (strlen(globals.workdir) + strlen(s->module->name) +
                strlen(perfexpert_phase_name[s->phase]) + 10));
            sprintf(globals.moduledir, "%s/%d/%d_%s_%s",
                globals.workdir, globals.cycle, i, s->module->name,
                perfexpert_phase_name[s->phase]);
            if (NULL == globals.moduledir) {
                OUTPUT(("%s", _ERROR("null moduledir")));
                goto CLEANUP;
            } else {
                OUTPUT_VERBOSE((5, "   %s %s", _YELLOW("module directory:"),
                    globals.moduledir));
            }
            if (PERFEXPERT_ERROR ==
                perfexpert_util_make_path(globals.moduledir)) {
                OUTPUT(("%s", _ERROR("cannot create module work directory")));
                goto CLEANUP;
            }

            /* Call the module */
            if (NULL == s->function) {
                OUTPUT(("%s", _ERROR("null step function")));
                goto CLEANUP;
            }
            switch ((s->status = s->function())) {
                case PERFEXPERT_STEP_ERROR:
                case PERFEXPERT_STEP_FAILURE:
                    OUTPUT(("%s", _ERROR("error in workflow progress")));
                    goto CLEANUP;

                case PERFEXPERT_STEP_UNDEFINED:
                case PERFEXPERT_STEP_SUCCESS:
                case PERFEXPERT_STEP_NOREC:
                    break;
            }

            i++;
            PERFEXPERT_DEALLOC(globals.moduledir);
        }

        /*
//         if ((NULL != globals.sourcefile) || (NULL != globals.target)) {
             #if HAVE_CODE_TRANSFORMATION
             // Call code transformer 
             switch ((rc = transformation())) {
                 case PERFEXPERT_ERROR:
                 case PERFEXPERT_FAILURE:
                 case PERFEXPERT_FORK_ERROR:
                     OUTPUT(("%s",
                         _ERROR("while running code transformer")));
                     goto CLEANUP;

                 case PERFEXPERT_NO_TRANS:
                     OUTPUT(("PerfExpert has no automatic optimization for this "
                         "code. If there is"));
                     OUTPUT(("any recommendation (shown above) try to apply them"
                         " manually"));
                     goto CLEANUP;
             }
             char * file;
             PERFEXPERT_ALLOC(char, file,
                 (strlen(globals.workdir) + strlen(CT_REPORT) + 2));
             sprintf(file, "%s/%s", globals.workdir, CT_REPORT);
             if (NULL == file) {
                 OUTPUT(("%s", _ERROR("null file")));
                return PERFEXPERT_ERROR;
             }
             if (PERFEXPERT_SUCCESS != perfexpert_util_file_print(file)) {
                 OUTPUT(("%s", _ERROR("unable to show transformations")));
             }
             PERFEXPERT_DEALLOC(file);

             #else
             rc = PERFEXPERT_SUCCESS;
             goto CLEANUP;
             #endif
 //        } else {
//             rc = PERFEXPERT_SUCCESS;
//             goto CLEANUP;
//         }

        */

        OUTPUT(("%s", _BLUE("Starting another optimization cycle...")));

        /* Finalize modules */
        if (PERFEXPERT_SUCCESS != perfexpert_module_fini()) {
            OUTPUT(("%s", _ERROR("unable to finalize modules")));
            return PERFEXPERT_ERROR;
        }

        goto CLEANUP;
        globals.cycle++;
    }

    CLEANUP:
    OUTPUT_VERBOSE((4, "%s", _BLUE("Tool finalization")));

    /* Step 7: Disconnect from DB */
    if (PERFEXPERT_SUCCESS != perfexpert_database_disconnect(globals.db)) {
        OUTPUT(("%s", _ERROR("disconnecting from database")));
        return PERFEXPERT_FAILURE;
    }

    /* Step 8: Remove the garbage */
    if (NULL != globals.workdir) {
        if (PERFEXPERT_TRUE == globals.remove_garbage) {
            OUTPUT_VERBOSE((1, "%s", _BLUE("Removing temporary directory")));
            if (PERFEXPERT_SUCCESS !=
                perfexpert_util_remove_dir(globals.workdir)) {
                OUTPUT(("unable to remove work directory (%s)",
                    globals.workdir));
            }
        } else {
            OUTPUT(("%s [%s]", _CYAN("Temporary files are available in"),
                globals.workdir));
        }
    }

    OUTPUT(("%s %llu", _YELLOW("The unique ID of this PerfExpert run is:"),
        globals.unique_id));

    /* Step 9: Free memory */
    i = 0;
    while (NULL != globals.program_argv[i]) {
        PERFEXPERT_DEALLOC(globals.program_argv[i]);
        i++;
    }
    PERFEXPERT_DEALLOC(globals.dbfile);
    PERFEXPERT_DEALLOC(globals.workdir);
    PERFEXPERT_DEALLOC(globals.moduledir);
    PERFEXPERT_DEALLOC(globals.program_path);
    PERFEXPERT_DEALLOC(globals.program_full);
    perfexpert_alloc_free_all();

    if (rc == PERFEXPERT_UNDEFINED) { 
        rc = PERFEXPERT_SUCCESS;
    }
    return rc;
}

#ifdef __cplusplus
}
#endif

// EOF
