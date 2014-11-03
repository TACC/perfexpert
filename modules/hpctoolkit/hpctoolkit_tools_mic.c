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
#include <time.h>
#include <papi.h>

/* Modules headers */
#include "hpctoolkit.h"
#include "hpctoolkit_tools.h"
#include "hpctoolkit_path.h"

/* Tools headers */
#include "tools/perfexpert/perfexpert_types.h"

/* PerfExpert common headers */
#include "common/perfexpert_alloc.h"
#include "common/perfexpert_constants.h"
#include "common/perfexpert_fork.h"
#include "common/perfexpert_hash.h"
#include "common/perfexpert_output.h"
#include "common/perfexpert_time.h"
#include "install_dirs.h"

/* run_hpcrun_mic */
int run_hpcrun_mic(void) {
    int experiment_count = 0, count = 0, rc = PERFEXPERT_SUCCESS;
//     hpctoolkit_event_t *event = NULL, *t = NULL;
    char *script_file, *argv[4];
    FILE *script_file_FP;
//     test_t test;

    OUTPUT_VERBOSE((10, "there will be %d events/run", papi_max_events() - 1));

    /* If this command should run on the MIC, encapsulate it in a script */
    // PERFEXPERT_ALLOC(char, script_file, (strlen(globals.stepdir) + 15));
    // sprintf(script_file, "%s/mic_hpcrun.sh", globals.stepdir);

    // if (NULL == (script_file_FP = fopen(script_file, "w"))) {
    //     OUTPUT(("%s (%s)", _ERROR("unable to open file"), script_file));
    //     return PERFEXPERT_ERROR;
    // }

    // fprintf(script_file_FP, "#!/bin/sh");

//     /* Fill the script file with all the experiments, before, and after */
//     perfexpert_hash_iter_str(my_module_globals.events_by_name, event, t) {
//         if (0 < count) {
//             /* Add event */
//             fprintf(script_file_FP, " --event %s:%d", event->name,
//                 papi_get_sampling_rate(event->name));

//             count--;
//             continue;
//         }

//         /* Is this a new experiment, but not the first? */
//         if (0 < experiment_count) {
//             /* Add the program and the program arguments to experiment */
//             fprintf(script_file_FP, " %s", globals.program_full);
//             count = 0;
//             while (NULL != globals.program_argv[count]) {
//                 fprintf(script_file_FP, " %s", globals.program_argv[count]);
//                 count++;
//             }

//             /* Add the AFTER program */
//             if (NULL != my_module_globals.knc_after) {
//                 fprintf(script_file_FP, "\n\n# AFTER command\n");
//                 fprintf(script_file_FP, "%s", my_module_globals.knc_after);
//             }
//         }

//         /* Add the BEFORE program */
//         if (NULL != my_module_globals.knc_before) {
//             fprintf(script_file_FP, "\n\n# BEFORE command\n");
//             fprintf(script_file_FP, "%s", my_module_globals.knc_before);
//         }

//         fprintf(script_file_FP, "\n\n# HPCRUN (%d)\n", experiment_count);

//         /* Add PREFIX */
//         if (NULL != my_module_globals.knc_prefix) {
//             fprintf(script_file_FP, "%s ", my_module_globals.knc_prefix);
//         }

//          Arguments to run hpcrun
//         fprintf(script_file_FP, "%s --output %s/measurements", HPCRUN,
//             globals.stepdir);

//         experiment_count++;
//     }

//     /* Add the program and the program arguments to experiment's */
//     fprintf(script_file_FP, " %s", globals.program_full);
//     count = 0;
//     while (NULL != globals.program_argv[count]) {
//         fprintf(script_file_FP, " %s", globals.program_argv[count]);
//         count++;
//     }

//     /* Add the AFTER program */
//     if (NULL != my_module_globals.knc_after) {
//         fprintf(script_file_FP, "\n\n# AFTER command\n");
//         fprintf(script_file_FP, "%s", my_module_globals.knc_after);
//     }
//     fprintf(script_file_FP, "\n\nexit 0\n\n# EOF\n");

    /* Close file and set mode */
    // fclose(script_file_FP);
    // if (-1 == chmod(script_file, S_IRWXU)) {
    //     OUTPUT(("%s (%s)", _ERROR("unable to set script mode"), script_file));
    //     PERFEXPERT_DEALLOC(script_file);
    //     return PERFEXPERT_ERROR;
    // }

//     /* The super-ninja test sctructure */
//     PERFEXPERT_ALLOC(char, test.output, (strlen(globals.stepdir) + 19));
//     sprintf(test.output, "%s/knc_hpcrun.output", globals.stepdir);
//     test.input = NULL;
//     test.info = globals.program;

//     argv[0] = "ssh";
//     argv[1] = my_module_globals.knc;
//     argv[2] = script_file;
//     argv[3] = NULL;

//     /* Not using OUTPUT_VERBOSE because I want only one line */
//     if (8 <= globals.verbose) {
//         printf("%s %s %s %s %s\n", PROGRAM_PREFIX, _YELLOW("command line:"),
//             argv[0], argv[1], argv[2]);
//     }

//     /* Run program and test return code (should I really test it?) */
//     switch (rc = perfexpert_fork_and_wait(&test, argv)) {
//         case PERFEXPERT_ERROR:
//             OUTPUT_VERBOSE((7, "[%s]", _BOLDYELLOW("ERROR")));
//             break;

//         case PERFEXPERT_FAILURE:
//         case 255:
//             OUTPUT_VERBOSE((7, "[%s ]", _BOLDRED("FAIL")));
//             break;

//         case PERFEXPERT_SUCCESS:
//             OUTPUT_VERBOSE((7, "[ %s  ]", _BOLDGREEN("OK")));
//             break;

//         default:
//             OUTPUT_VERBOSE((7, "[UNKNO]"));
//             break;
//     }

//     PERFEXPERT_DEALLOC(script_file);
//     PERFEXPERT_DEALLOC(test.output);

    return rc;
}

#ifdef __cplusplus
}
#endif

// EOF
