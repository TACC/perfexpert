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
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <signal.h>

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
    char workdir_template[] = ".perfexpert-temp.XXXXXX";
    char *workdir = NULL, *file = NULL, *unique_id = NULL;
    int rc = PERFEXPERT_ERROR;

    /* Register the perfexpert signal handler */
    signal(SIGSEGV, perfexpert_sighandler);

    /* Set default values for globals */
    globals = (globals_t) {
        .verbose       = 0,                // int
        .colorful      = PERFEXPERT_FALSE, // int
        .dbfile        = NULL,             // *char
        .rec_count     = 3,                // int
        .leave_garbage = PERFEXPERT_FALSE, // int
        .pid           = (long)getpid(),   // long int
        .target        = NULL,             // *char
        .sourcefile    = NULL,             // *char
        .program       = NULL,             // *char
        .program_path  = NULL,             // *char
        .program_full  = NULL,             // *char
        .program_argv  = { 0 },            // *char[]
        .step          = 1,                // int
        .workdir       = NULL,             // *char
        .stepdir       = NULL,             // *char
        .only_exp      = PERFEXPERT_FALSE, // int
        .compat_mode   = PERFEXPERT_FALSE, // int
        .unique_id     = 0                 // long long int
    };

    /* Parse command-line parameters */
    if (PERFEXPERT_SUCCESS != parse_cli_params(argc, argv)) {
        OUTPUT(("%s", _ERROR("parsing command line arguments")));
        return PERFEXPERT_ERROR;
    }

    /* Initialization: 4 steps */
    OUTPUT_VERBOSE((4, "%s", _BLUE("Initialization phase")));

    /* Step 1: Define the unique identification for this PerfExpert session */
    PERFEXPERT_ALLOC(char, unique_id, 20);
    sprintf(unique_id, "%llu%06d", (long long)time(NULL), (int)getpid());
    globals.unique_id = strtoll(unique_id, NULL, 10);
    OUTPUT_VERBOSE((5, "   %s %llu", _YELLOW("unique ID:"), globals.unique_id));

    /* Step 2: Define the work directory */
    if (NULL == (workdir = mkdtemp(workdir_template))) {
        OUTPUT(("%s", _ERROR("while defining the working directory")));
        return PERFEXPERT_ERROR;
    }
    PERFEXPERT_ALLOC(char, globals.workdir,
        (strlen(getcwd(NULL, 0)) + strlen(workdir) + 2));
    sprintf(globals.workdir, "%s/%s", getcwd(NULL, 0), workdir);
    if (NULL == globals.workdir) {
        OUTPUT(("%s", _ERROR("null globals.workdir")));
        return PERFEXPERT_ERROR;
    } else {
        OUTPUT_VERBOSE((5, "   %s %s", _YELLOW("workdir:"), globals.workdir));
    }

    /* Step 3: Check if the database file exists and is updated, then connect */
    if ((NULL != globals.dbfile) &&
        (PERFEXPERT_SUCCESS != perfexpert_util_file_exists(globals.dbfile))) {
        OUTPUT(("%s", _ERROR((char *)"database file not found")));
        goto CLEANUP;
    }
    if (PERFEXPERT_SUCCESS != perfexpert_database_update(&(globals.dbfile))) {
        OUTPUT(("%s", _ERROR((char *)"unable to update database")));
        goto CLEANUP;
    }
    if (PERFEXPERT_SUCCESS != perfexpert_database_connect(&(globals.db),
        globals.dbfile)) {
        OUTPUT(("%s %s", _ERROR("connecting to database"), globals.db));
        goto CLEANUP;
    }
    if (NULL == globals.dbfile) {
        OUTPUT(("%s", _ERROR("null globals.dbfile")));
        return PERFEXPERT_ERROR;
    } else {
        OUTPUT_VERBOSE((5, "   %s %s", _YELLOW("database:"), globals.dbfile));
    }

    /* Step 4: Initialize modules */
    if (PERFEXPERT_SUCCESS != perfexpert_module_init()) {
        OUTPUT(("%s", _ERROR("unable to initialize modules")));
        return PERFEXPERT_ERROR;
    }

    /* Iterate until some module return != PERFEXPERT_SUCCESS */
    OUTPUT_VERBOSE((4, "%s", _BLUE("Starting optimization workflow")));
    while (1) {
        /* Create step working directory */
        PERFEXPERT_ALLOC(char, globals.stepdir, (strlen(globals.workdir) + 5));
        sprintf(globals.stepdir, "%s/%d", globals.workdir, globals.step);
        if (PERFEXPERT_ERROR == perfexpert_util_make_path(globals.stepdir)) {
            OUTPUT(("%s", _ERROR("cannot create step workdir")));
            goto CLEANUP;
        }
        if (NULL == globals.stepdir) {
            OUTPUT(("%s", _ERROR("null stepdir")));
            return PERFEXPERT_ERROR;
        } else {
            OUTPUT_VERBOSE((5, "   %s %s", _YELLOW("stepdir:"),
                globals.stepdir));
        }

        /* If necessary, compile the user program */
        if ((NULL != globals.sourcefile) || (NULL != globals.target)) {
            if (PERFEXPERT_SUCCESS != compile_program()) {
                OUTPUT(("%s", _ERROR("program compilation failed")));
                goto CLEANUP;
            }
        }

        /* Call measurement modules */
        OUTPUT_VERBOSE((4, "%s", _BLUE("Collecting measurements")));
        if (PERFEXPERT_SUCCESS != perfexpert_module_measurements()) {
            OUTPUT(("%s", _ERROR("while taking measurements")));
            goto CLEANUP;
        }

        /* Call analysis modules */
        OUTPUT_VERBOSE((4, "%s", _BLUE("Analyzing measurements")));
        if (PERFEXPERT_SUCCESS != perfexpert_module_analysis()) {
            OUTPUT(("%s", _ERROR("while analyzing measurements")));
            goto CLEANUP;
        }

        /* Call recommender */
        switch ((rc = recommendation())) {
            case PERFEXPERT_ERROR:
            case PERFEXPERT_FAILURE:
            case PERFEXPERT_FORK_ERROR:
                OUTPUT(("%s", _ERROR("while running recommender")));
                goto CLEANUP;

            case PERFEXPERT_NO_REC:
                OUTPUT(("PerfExpert has no recommendations for this code"));
                goto CLEANUP;
        }
        PERFEXPERT_ALLOC(char, file,
            (strlen(globals.stepdir) + strlen(RECOMMENDER_REPORT) + 2));
        sprintf(file, "%s/%s", globals.stepdir, RECOMMENDER_REPORT);
        if (NULL == file) {
            OUTPUT(("%s", _ERROR("null file")));
            return PERFEXPERT_ERROR;
        }
        if (PERFEXPERT_SUCCESS != perfexpert_util_file_print(file)) {
            OUTPUT(("PerfExpert has no recommendations for this code"));
            goto CLEANUP;
            // TODO: this is a work around when something wrong happens it will
            //       be better to investigate why recommender may return
            //       PERFEXPERT_SUCCESS with empty report file
            OUTPUT(("%s", _ERROR("unable to show recommendations")));
        }
        PERFEXPERT_DEALLOC(file);

        if ((NULL != globals.sourcefile) || (NULL != globals.target)) {
            #if HAVE_CODE_TRANSFORMATION
            /* Call code transformer */
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
            PERFEXPERT_ALLOC(char, file,
                (strlen(globals.stepdir) + strlen(CT_REPORT) + 2));
            sprintf(file, "%s/%s", globals.stepdir, CT_REPORT);
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
        } else {
            rc = PERFEXPERT_SUCCESS;
            goto CLEANUP;
        }
        OUTPUT(("%s", _BLUE("Starting another optimization round...")));
        PERFEXPERT_DEALLOC(globals.stepdir);
        globals.step++;
    }

    CLEANUP:
    /* Remove the garbage */
    if (PERFEXPERT_FALSE == globals.leave_garbage) {
        OUTPUT_VERBOSE((1, "%s", _BLUE("Removing temporary directory")));
        if (PERFEXPERT_SUCCESS != perfexpert_util_remove_dir(globals.workdir)) {
            OUTPUT(("unable to remove work directory (%s)", globals.workdir));
        }
    } else {
        OUTPUT(("%s [%s]", _CYAN("Temporary files are available in"),
            globals.workdir));
    }

    /* Free memory */
    PERFEXPERT_DEALLOC(globals.workdir);
    PERFEXPERT_DEALLOC(globals.stepdir);

    return rc;
}

#ifdef __cplusplus
}
#endif

// EOF
