/*
 * Copyright (c) 2013  University of Texas at Austin. All rights reserved.
 *
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * This file is part of PerfExpert.
 *
 * PerfExpert is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * PerfExpert is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with PerfExpert. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Leonardo Fialho
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

/* PerfExpert headers */
#include "config.h"
#include "perfexpert.h"
#include "perfexpert_alloc.h"
#include "perfexpert_constants.h"
#include "perfexpert_database.h"
#include "perfexpert_output.h"
#include "perfexpert_util.h"

/* Global variables, try to not create them! */
globals_t globals; // Variable to hold global options, this one is OK

/* main, life starts here */
int main(int argc, char** argv) {
    char workdir_template[] = ".perfexpert-temp.XXXXXX";
    char *workdir;
    char *report_file;
    int rc = PERFEXPERT_ERROR;

    /* Set default values for globals */
    globals = (globals_t) {
        .verbose       = 0,                // int
        .dbfile        = NULL,             // char *
        .colorful      = PERFEXPERT_FALSE, // int
        .threshold     = 0.0,              // float
        .rec_count     = 3,                // int
        .leave_garbage = PERFEXPERT_FALSE, // int
        .pid           = (long)getpid(),   // long int
        .target        = NULL,             // char *
        .sourcefile    = NULL,             // char *
        .program       = NULL,             // char *
        .program_path  = NULL,             // char *
        .program_full  = NULL,             // char *
        .prog_arg_pos  = 0,                // int
        .main_argc     = argc,             // int
        .main_argv     = argv,             // char **
        .step          = 1,                // int
        .workdir       = NULL,             // char *
        .stepdir       = NULL,             // char *
        .prefix        = NULL,             // char *
        .before        = NULL,             // char *
        .after         = NULL,             // char *
        .knc           = NULL,             // char *
        .knc_prefix    = NULL,             // char *
        .knc_before    = NULL,             // char *
        .knc_after     = NULL              // char *
    };

    /* Parse command-line parameters */
    if (PERFEXPERT_SUCCESS != parse_cli_params(argc, argv)) {
        OUTPUT(("%s", _ERROR("Error: parsing command line arguments")));
        return PERFEXPERT_ERROR;
    }

    /* Create a work directory */
    workdir = mkdtemp(workdir_template);
    if (NULL == workdir) {
        OUTPUT(("%s", _ERROR("Error: creating working directory")));
        return PERFEXPERT_ERROR;
    }
    PERFEXPERT_ALLOC(char, globals.workdir,
        (strlen(getcwd(NULL, 0)) + strlen(workdir) + 2));
    sprintf(globals.workdir, "%s/%s", getcwd(NULL, 0), workdir);
    OUTPUT_VERBOSE((5, "   %s %s", _YELLOW("workdir:"), globals.workdir));

    /* If database was not specified, check if there is any local database and
     * if this database is update
     */
    if (NULL == globals.dbfile) {
        if (PERFEXPERT_SUCCESS !=
            perfexpert_database_update(&(globals.dbfile))) {
            OUTPUT(("%s", _ERROR((char *)"Error: unable to copy database")));
            goto CLEANUP;
        }        
    } else {
        if (PERFEXPERT_SUCCESS != perfexpert_util_file_exists(globals.dbfile)) {
            OUTPUT(("%s", _ERROR((char *)"Error: database file not found")));
            goto CLEANUP;
        }
    }
    OUTPUT_VERBOSE((5, "   %s %s", _YELLOW("database:"), globals.dbfile));

    /* Iterate until some tool return != PERFEXPERT_SUCCESS */
    while (1) {
        /* Create step working directory */
        PERFEXPERT_ALLOC(char, globals.stepdir, (strlen(globals.workdir) + 5));
        sprintf(globals.stepdir, "%s/%d", globals.workdir, globals.step);
        if (PERFEXPERT_ERROR == perfexpert_util_make_path(globals.stepdir,
            0755)) {
            OUTPUT(("%s", _ERROR((char *)"Error: cannot create step workdir")));
            goto CLEANUP;
        }
        OUTPUT_VERBOSE((5, "   %s %s", _YELLOW("stepdir:"), globals.stepdir));

        /* If necessary, compile the user program */
        if ((NULL != globals.sourcefile) || (NULL != globals.target)) {
            if (PERFEXPERT_SUCCESS != compile_program()) {
                OUTPUT(("%s", _ERROR("Error: program compilation failed")));
                if (NULL != globals.knc) {
                    OUTPUT(("Are you adding the flags to to compile for MIC?"));                    
                }
                goto CLEANUP;
            }
        }

        /* Call HPCToolkit and stuff (former perfexpert_run_exp) */
        if (PERFEXPERT_SUCCESS != measurements()) {
            OUTPUT(("%s", _ERROR("Error: unable to take measurements")));
            goto CLEANUP;
        }

        /* Call analyzer and stuff (former perfexpert) */
        switch (analysis()) {
            case PERFEXPERT_FAILURE:
            case PERFEXPERT_ERROR:
                OUTPUT(("%s", _ERROR("Error: unable to run analyzer")));
                goto CLEANUP;

            case PERFEXPERT_NO_DATA:
                OUTPUT(("Unable to analyze your application"));

                /* Print analysis report */
                PERFEXPERT_ALLOC(char, report_file,
                    (strlen(globals.stepdir) + strlen(ANALYZER_REPORT) + 2));
                sprintf(report_file, "%s/%s", globals.stepdir, ANALYZER_REPORT);
                if (PERFEXPERT_SUCCESS !=
                    perfexpert_util_file_print(report_file)) {
                    OUTPUT(("%s",
                        _ERROR("Error: unable to show analysis report")));
                }
                PERFEXPERT_DEALLOC(report_file);
                rc = PERFEXPERT_NO_DATA;
                goto CLEANUP;

            case PERFEXPERT_SUCCESS:
                rc = PERFEXPERT_SUCCESS;
        }

        /* Call recommender */
        switch (recommendation()) {
            case PERFEXPERT_ERROR:
            case PERFEXPERT_FAILURE:
                OUTPUT(("%s", _ERROR("Error: unable to run recommender")));
                goto CLEANUP;

            case PERFEXPERT_NO_REC:
                OUTPUT(("No recommendation found, printing analysys report"));
                rc = PERFEXPERT_NO_REC;
                goto REPORT;

            case PERFEXPERT_SUCCESS:
                rc = PERFEXPERT_SUCCESS;
        }

        if ((NULL != globals.sourcefile) || (NULL != globals.target)) {
            #if HAVE_ROSE == 1
            /* Call code transformer */
            switch (transformation()) {
                case PERFEXPERT_ERROR:
                case PERFEXPERT_FAILURE:
                    OUTPUT(("%s",
                            _ERROR("Error: unable to run code transformer")));
                    goto CLEANUP;

                case PERFEXPERT_NO_TRANS:
                    OUTPUT(("Unable to apply optimizations automatically"));
                    rc = PERFEXPERT_NO_TRANS;
                    goto REPORT;

                case PERFEXPERT_SUCCESS:
                    rc = PERFEXPERT_SUCCESS;
            }
            #endif
        } else {
            rc = PERFEXPERT_SUCCESS;
            goto REPORT;
        }
        OUTPUT(("Starting another optimization round..."));
        PERFEXPERT_DEALLOC(globals.stepdir);
        globals.step++;
    }

    /* Print analysis report */
    REPORT:
    PERFEXPERT_ALLOC(char, report_file,
        (strlen(globals.stepdir) + strlen(ANALYZER_REPORT) + 2));
    sprintf(report_file, "%s/%s", globals.stepdir, ANALYZER_REPORT);
    if (PERFEXPERT_SUCCESS != perfexpert_util_file_print(report_file)) {
        OUTPUT(("%s", _ERROR("Error: unable to show analysis report")));
    }
    PERFEXPERT_DEALLOC(report_file);

    /* Print recommendations */
    if ((0 < globals.rec_count) && (PERFEXPERT_NO_REC != rc)) {
        PERFEXPERT_ALLOC(char, report_file,
            (strlen(globals.stepdir) + strlen(RECOMMENDER_REPORT) + 2));
        sprintf(report_file, "%s/%s", globals.stepdir, RECOMMENDER_REPORT);
        if (PERFEXPERT_SUCCESS != perfexpert_util_file_print(report_file)) {
            OUTPUT(("%s", _ERROR("Error: unable to show recommendations")));
        }
        PERFEXPERT_DEALLOC(report_file);
    }

    CLEANUP:
    /* Remove the garbage */
    if (PERFEXPERT_FALSE == globals.leave_garbage) {
        if (PERFEXPERT_SUCCESS != perfexpert_util_remove_dir(globals.workdir)) {
            OUTPUT(("unable to remove work directory (%s)", globals.workdir));
        }        
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
