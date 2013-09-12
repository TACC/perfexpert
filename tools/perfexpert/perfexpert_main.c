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
#include "perfexpert_output.h"
#include "perfexpert_util.h"
#include "perfexpert_database.h"
#include "perfexpert_constants.h"

/* Global variables, try to not create them! */
globals_t globals; // Variable to hold global options, this one is OK

/* main, life starts here */
int main(int argc, char** argv) {
    char workdir[] = ".perfexpert-temp.XXXXXX";
    char temp_str[BUFFER_SIZE];
    char *temp_str2;
    int rc = PERFEXPERT_ERROR;

    /* Set default values for globals */
    globals = (globals_t) {
        .verbose_level = 0,              // int
        .dbfile        = NULL,           // char *
        .colorful      = 0,              // int
        .threshold     = 0.0,            // float
        .rec_count     = 3,              // int
        .clean_garbage = 0,              // int
        .pid           = (long)getpid(), // long int
        .target        = NULL,           // char *
        .sourcefile    = NULL,           // char *
        .program       = NULL,           // char *
        .program_path  = NULL,           // char *
        .program_full  = NULL,           // char *
        .prog_arg_pos  = 0,              // int
        .main_argc     = argc,           // int
        .main_argv     = argv,           // char **
        .step          = 1,              // int
        .workdir       = NULL,           // char *
        .stepdir       = NULL,           // char *
        .prefix        = NULL,           // char *
        .before        = NULL,           // char *
        .after         = NULL,           // char *
        .knc           = NULL,           // char *
        .knc_prefix    = NULL,           // char *
        .knc_before    = NULL,           // char *
        .knc_after     = NULL            // char *
    };

    /* Parse command-line parameters */
    if (PERFEXPERT_SUCCESS != parse_cli_params(argc, argv)) {
        OUTPUT(("%s", _ERROR("Error: parsing command line arguments")));
        goto clean_up;
    }

    /* Create a work directory */
    bzero(temp_str, BUFFER_SIZE);
    temp_str2 = mkdtemp(workdir);
    if (NULL == temp_str2) {
        OUTPUT(("%s", _ERROR("Error: creating working directory")));
        goto clean_up;
    }
    globals.workdir = (char *)malloc(strlen(getcwd(NULL, 0)) + strlen(temp_str2)
        + 1);
    if (NULL == globals.workdir) {
        OUTPUT(("%s", _ERROR("Error: out of memory")));
        return PERFEXPERT_ERROR;
    }
    bzero(globals.workdir, strlen(getcwd(NULL, 0)) + strlen(temp_str2) + 1);
    sprintf(globals.workdir, "%s/%s", getcwd(NULL, 0), temp_str2);
    OUTPUT_VERBOSE((5, "   %s %s", _YELLOW("workdir:"), globals.workdir));

    /* If database was not specified, check if there is any local database and
     * if this database is update
     */
    if (NULL == globals.dbfile) {
        if (PERFEXPERT_SUCCESS !=
            perfexpert_database_update(&(globals.dbfile))) {
            OUTPUT(("%s", _ERROR((char *)"Error: unable to copy database")));
            goto clean_up;
        }        
    } else {
        if (PERFEXPERT_SUCCESS != perfexpert_util_file_exists(globals.dbfile)) {
            OUTPUT(("%s", _ERROR((char *)"Error: database file not found")));
            goto clean_up;
        }
    }
    OUTPUT_VERBOSE((5, "   %s %s", _YELLOW("database:"), globals.dbfile));

    /* Iterate until some tool return != PERFEXPERT_SUCCESS */
    while (1) {
        /* Create step working directory */
        globals.stepdir = (char *)malloc(strlen(globals.workdir) + 5);
        if (NULL == globals.stepdir) {
            OUTPUT(("%s", _ERROR("Error: out of memory")));
            goto clean_up;
        }
        bzero(globals.stepdir, strlen(globals.workdir) + 5);
        sprintf(globals.stepdir, "%s/%d", globals.workdir, globals.step);
        if (PERFEXPERT_ERROR == perfexpert_util_make_path(globals.stepdir,
            0755)) {
            OUTPUT(("%s", _ERROR((char *)"Error: cannot create step workdir")));
            goto clean_up;
        }
        OUTPUT_VERBOSE((5, "   %s %s", _YELLOW("stepdir:"), globals.stepdir));

        /* If necessary, compile the user program */
        if ((NULL != globals.sourcefile) || (NULL != globals.target)) {
            if (PERFEXPERT_SUCCESS != compile_program()) {
                OUTPUT(("%s", _ERROR("Error: program compilation failed")));
                if (NULL != globals.knc) {
                    OUTPUT(("Are you adding the flags to to compile for MIC?"));                    
                }
                goto clean_up;
            }
        }

        /* Call HPCToolkit and stuff (former perfexpert_run_exp) */
        if (PERFEXPERT_SUCCESS != measurements()) {
            OUTPUT(("%s", _ERROR("Error: unable to take measurements")));
            goto clean_up;
        }

        /* Call analyzer and stuff (former perfexpert) */
        switch (analysis()) {
            case PERFEXPERT_FAILURE:
            case PERFEXPERT_ERROR:
                OUTPUT(("%s", _ERROR("Error: unable to run analyzer")));
                goto clean_up;

            case PERFEXPERT_NO_DATA:
                OUTPUT(("Unable to analyze your application"));

                /* Print analysis report */
                bzero(temp_str, BUFFER_SIZE);
                sprintf(temp_str, "%s/%s.txt", globals.stepdir,
                        ANALYZER_REPORT);

                if (PERFEXPERT_SUCCESS !=
                    perfexpert_util_file_print(temp_str)) {
                    OUTPUT(("%s",
                        _ERROR("Error: unable to show analysis report")));
                }
                rc = PERFEXPERT_NO_DATA;
                goto clean_up;

            case PERFEXPERT_SUCCESS:
                rc = PERFEXPERT_SUCCESS;
        }

        /* Call recommender */
        switch (recommendation()) {
            case PERFEXPERT_ERROR:
            case PERFEXPERT_FAILURE:
                OUTPUT(("%s", _ERROR("Error: unable to run recommender")));
                goto clean_up;

            case PERFEXPERT_NO_REC:
                OUTPUT(("No recommendation found, printing analysys report"));
                rc = PERFEXPERT_NO_REC;
                goto report;

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
                    goto clean_up;

                case PERFEXPERT_NO_TRANS:
                    OUTPUT(("Unable to apply optimizations automatically"));
                    rc = PERFEXPERT_NO_TRANS;
                    goto report;

                case PERFEXPERT_SUCCESS:
                    rc = PERFEXPERT_SUCCESS;
            }
            #endif
        } else {
            rc = PERFEXPERT_SUCCESS;
            goto report;
        }
        OUTPUT(("Starting another optimization round..."));
        globals.step++;
    }

    /* Print analysis report */
    report:
    bzero(temp_str, BUFFER_SIZE);
    sprintf(temp_str, "%s/%s.txt", globals.stepdir, ANALYZER_REPORT);

    if (PERFEXPERT_SUCCESS != perfexpert_util_file_print(temp_str)) {
        OUTPUT(("%s", _ERROR("Error: unable to show analysis report")));
    }

    /* Print recommendations */
    if ((0 < globals.rec_count) && (PERFEXPERT_NO_REC != rc)) {
        bzero(temp_str, BUFFER_SIZE);
        sprintf(temp_str, "%s/%s.txt", globals.stepdir, RECOMMENDER_REPORT);

        if (PERFEXPERT_SUCCESS != perfexpert_util_file_print(temp_str)) {
            OUTPUT(("%s", _ERROR("Error: unable to show recommendations")));
        }
    }

    clean_up:
    /* TODO: Should I remove the garbage? */
    if (!globals.clean_garbage) {
    }

    /* TODO: Free memory */
    if (NULL != globals.stepdir) {
        free(globals.stepdir);
    }
    /* Remove perfexpert.log */
    remove("perfexpert.log");

    return rc;
}

#ifdef __cplusplus
}
#endif

// EOF
