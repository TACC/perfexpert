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
#include <string.h>
#include <time.h>

/* Utility headers */
#include <sqlite3.h>

/* Modules headers */
#include "macpo.h"

#include "macpo_module.h"

#include "../perfexpert_module_base.h"

/* PerfExpert common headers */
#include "common/perfexpert_alloc.h"
#include "common/perfexpert_constants.h"
#include "common/perfexpert_fork.h"
#include "common/perfexpert_output.h"
#include "common/perfexpert_time.h"

/* macpo_instrument_all */
int macpo_instrument_all(void) {
    char *error = NULL, sql[MAX_BUFFER_SIZE];
    int i = 0;

    for (i = 0; i < my_module_globals.num_inst_files; ++i) {
       // PERFEXPERT_DEALLOC(my_module_globals.inst_files[i].file);
       // PERFEXPERT_DEALLOC(my_module_globals.inst_files[i].destfile);
    } 
    my_module_globals.num_inst_files = 0;

    bzero(sql, MAX_BUFFER_SIZE);
    sprintf(sql, "SELECT name, file, line FROM hotspot WHERE perfexpert_id = "
        //"%llu AND relevance>= %f", globals.unique_id, my_module_globals.threshold);
        "%llu ", globals.unique_id);
    if (SQLITE_OK != sqlite3_exec(globals.db, sql, macpo_instrument, NULL,
        &error)) {
        OUTPUT(("%s %s", _ERROR("SQL error"), error));
        sqlite3_free(error);
        return PERFEXPERT_ERROR;
    }

    if (PERFEXPERT_SUCCESS != instrument_files()) {
        OUTPUT(("%s", _ERROR("Problem instrumenting code")));
        return PERFEXPERT_ERROR;
    }
    return PERFEXPERT_SUCCESS;
}

/* macpo_instrument */
static int macpo_instrument(void *n, int c, char **val, char **names) {
    char *t = NULL, *name = val[0], *file = val[1];
    long line;
    int i;
    int prevInst = PERFEXPERT_FALSE;
    char *folder, *fullpath, *filename, *rose_name;

    line = strtoll(val[2], NULL, 10);

    if (PERFEXPERT_SUCCESS != perfexpert_util_file_is_writable(file)) {
        OUTPUT_VERBOSE((9, "  file %s is not writable", file));
        return PERFEXPERT_SUCCESS;
    }

    OUTPUT_VERBOSE((6, "  instrumenting %s   @   %s:%ld", name, file, line));
    // Remove everything after '(' in the function name (if exists)
    char *ptr = strchr(name, '(');
    if (ptr) {
        *ptr = 0;
    }

    //if (PERFEXPERT_SUCCESS != perfexpert_util_file_exists(file)) {
    //    return PERFEXPERT_SUCCESS;
    //}

    if (strlen(name)<=1) {
        return PERFEXPERT_SUCCESS;
    }

    if (PERFEXPERT_SUCCESS != perfexpert_util_filename_only(file, &filename)) {
        return PERFEXPERT_SUCCESS;
    }

    if (PERFEXPERT_SUCCESS != perfexpert_util_path_only(file, &folder)) {
        return PERFEXPERT_ERROR;
    }

    //OUTPUT_VERBOSE((6, " going into the loop"));
    for (i = 0; i < my_module_globals.instrument.maxfiles; ++i) {
        if (strcmp (my_module_globals.instrument.list[i].file, file) == 0) {
            prevInst = PERFEXPERT_TRUE;
            int pos = my_module_globals.instrument.list[i].maxlocations;
            my_module_globals.instrument.list[i].locations[pos] = line;
            PERFEXPERT_ALLOC(char, my_module_globals.instrument.list[i].names[pos], strlen(name)+1);
            snprintf(my_module_globals.instrument.list[i].names[pos], strlen(name)+1, "%s", name);
            OUTPUT_VERBOSE((6, "Added1 %s", name));
            my_module_globals.instrument.list[i].maxlocations++;
        }
    }
    if (prevInst == PERFEXPERT_FALSE) {
        int pos = my_module_globals.instrument.maxfiles;
        PERFEXPERT_ALLOC(char, my_module_globals.instrument.list[pos].names[0], strlen(name)+1);
        snprintf(my_module_globals.instrument.list[pos].names[0], strlen(name)+1, "%s", name);
        my_module_globals.instrument.list[pos].locations[0] = line;
        OUTPUT_VERBOSE((6, "Added2 %s", name));

        my_module_globals.instrument.list[pos].maxlocations = 1;
        PERFEXPERT_ALLOC(char, my_module_globals.instrument.list[pos].file, strlen(file)+1);
        snprintf(my_module_globals.instrument.list[pos].file, strlen(file)+1, "%s", file);
        PERFEXPERT_ALLOC(char, my_module_globals.instrument.list[pos].backfile, strlen(globals.moduledir) +
                         strlen(file) + 8);
        snprintf(my_module_globals.instrument.list[pos].backfile, strlen(globals.moduledir) + strlen(folder) +
                        strlen(filename) + 8, "%s/%s/inst_%s", globals.moduledir, folder, filename);
//        snprintf(my_module_globals.instrument.list[pos].backfile, strlen(globals.moduledir) + strlen(file) +
//                 7, "%s/%s_inst", globals.moduledir, file);
        my_module_globals.instrument.maxfiles++;

        my_module_globals.num_inst_files++;
    }

    return PERFEXPERT_SUCCESS;
}

int instrument_files() {
    char *argv[9];
    int i, j;
    test_t test;
    int rc;
    
    argv[0] = "macpo.sh";
    for (i = 0; i < my_module_globals.instrument.maxfiles; ++i) {
        fileInsts currfile;
        currfile = my_module_globals.instrument.list[i];
        char arguments[MAX_COLLECTION];
        char *target = arguments;
        long length = 0;
        char *file, *filename, *folder, *fullpath;
        file = currfile.file;

        OUTPUT_VERBOSE((9,"\n\n\n\n Instrumenting file %s\n\n\n\n", file));

        if (PERFEXPERT_SUCCESS != perfexpert_util_filename_only(file, &filename)) {
            return PERFEXPERT_SUCCESS;
        }

        if (PERFEXPERT_SUCCESS != perfexpert_util_path_only(file, &folder)) {
            return PERFEXPERT_ERROR;
        }

        // Create the folder where to store the backup
        PERFEXPERT_ALLOC(char, fullpath, (strlen(globals.moduledir) +
                     strlen(folder) + 10));
        snprintf(fullpath, strlen(globals.moduledir) + strlen(folder) + 10,
                 "%s/%s", globals.moduledir, folder);
        perfexpert_util_make_path(fullpath);
       
        // Copy the file to the backup location 
        if (PERFEXPERT_SUCCESS != perfexpert_util_file_copy(my_module_globals.instrument.list[i].backfile, my_module_globals.instrument.list[i].file)) {
            OUTPUT(("%s impossible to copy file %s to %s", _ERROR("IO ERROR"), my_module_globals.instrument.list[i].file, my_module_globals.instrument.list[i].backfile));
        }

        for (j = 0; j < currfile.maxlocations; ++j) {
            char* name = currfile.names[j];
            int line = currfile.locations[j];
            OUTPUT_VERBOSE((6, "Processing name %s in file %s @ %ld", name, currfile.file, line));
            if (line == 0) {
                if (j == 0) {
                    target += sprintf(target, "%s", name);
                    length+=strlen(name)+1;
                }
                else {
                    target += sprintf(target, ",%s", name);
                    length+=strlen(name)+2;
                }
            }
            else{
                if (j == 0) {
                    target += sprintf(target, "%s:%d", name, line);
                    length += strlen(name)+1+perfexpert_util_digits(line);
                }
                else {
                    target += sprintf(target, ",%s:%d", name, line);
                    length += strlen(name)+1+perfexpert_util_digits(line)+1;
                }
            }
            OUTPUT_VERBOSE((6, "LENGTH: %ld", length));
        }            
        PERFEXPERT_ALLOC(char, argv[1], 25 + length);
        snprintf(argv[1], 25 + length, "--macpo:check-alignment=%s", arguments);
        PERFEXPERT_ALLOC(char, argv[2], 30 + length);
        snprintf(argv[2], 30 + length, "--macpo:record-tripcount=%s", arguments);
        PERFEXPERT_ALLOC(char, argv[3], 30 + length);
        snprintf(argv[3], 30 + length, "--macpo:vector-strides=%s", arguments);

        PERFEXPERT_ALLOC(char, argv[4],
            (strlen(globals.moduledir) + strlen(folder) + strlen(filename) + 35));
        snprintf(argv[4], strlen(globals.moduledir) + +strlen(folder) + strlen(filename) + 35,
            "--macpo:backup-filename=%s/%s/inst_%s", globals.moduledir, folder, filename);

        PERFEXPERT_ALLOC(char, argv[5], 25 + length);
        snprintf(argv[5], 25 + length, "--macpo:instrument=%s", arguments);

        argv[6] = "--macpo:no-compile";

        PERFEXPERT_ALLOC(char, argv[7], strlen(file)+1);
        snprintf(argv[7], strlen(file)+1,"%s",file);

        argv[8] = NULL;  /* Add NULL to indicate the end of arguments */

        PERFEXPERT_ALLOC(char, test.output, (strlen(globals.moduledir) +
                     strlen(file) + 20));
        snprintf(test.output, strlen(globals.moduledir) + strlen(file) + 20,
                "%s/%s-macpo.output", globals.moduledir, file);
        test.input = NULL;
        test.info = globals.program;

        OUTPUT_VERBOSE((6, "   COMMAND=[%s %s %s %s %s %s %s %s]", argv[0], argv[1],
                        argv[2], argv[3], argv[4], argv[5], argv[6], argv[7]));

        rc = perfexpert_fork_and_wait(&test, (char **)argv);

    
        if (!my_module_globals.ignore_return_code) {
            switch (rc) {
                case PERFEXPERT_FAILURE:
                case PERFEXPERT_ERROR:
                    OUTPUT(("%s (return code: %d) Usually, this means that an error"
                        " happened during the program execution. To see the program"
                        "'s output, check the content of this file: [%s]. If you "
                        "want to PerfExpert ignore the return code next time you "
                        "run this program, set the 'return-code' option for the "
                        "macpo module. See 'perfepxert -H macpo' for details.",
                        _ERROR("the target program returned non-zero"), rc,
                        test.output));
                    return PERFEXPERT_ERROR;

                case PERFEXPERT_SUCCESS:
                    OUTPUT_VERBOSE((7, "[ %s  ]", _BOLDGREEN("OK")));
                    break;

                default:
                    break;
            }
        }
    

        char * rose_name;
        PERFEXPERT_ALLOC(char, rose_name, (strlen(filename) + 6));
        snprintf(rose_name, strlen(filename) + 6, "rose_%s", filename);


        if (PERFEXPERT_SUCCESS != perfexpert_util_file_rename(rose_name, file)) {
            OUTPUT(("%s impossible to copy file %s to %s", _ERROR("IO ERROR"),
                    rose_name, file));
        }
        else {
            OUTPUT_VERBOSE((9, "Copied file %s to: %s", rose_name, file));
        }

        PERFEXPERT_DEALLOC(rose_name);

        PERFEXPERT_DEALLOC(test.output);
        PERFEXPERT_DEALLOC(argv[1]);
        PERFEXPERT_DEALLOC(argv[2]);
        PERFEXPERT_DEALLOC(argv[3]);
        PERFEXPERT_DEALLOC(argv[4]);
        PERFEXPERT_DEALLOC(argv[5]);
        PERFEXPERT_DEALLOC(argv[7]);
        PERFEXPERT_DEALLOC(fullpath);
    }

    return PERFEXPERT_SUCCESS;
}

int macpo_compile() {
    int comp_loaded = PERFEXPERT_FALSE;

    OUTPUT_VERBOSE((9, "compiling the code"));
    if (PERFEXPERT_TRUE == perfexpert_module_available("make")) {
        OUTPUT_VERBOSE((5, "%s",
            _CYAN("will use make as compilation module")));
        myself_module.measurement = (perfexpert_module_measurement_t *) perfexpert_module_get("make");
        if (NULL != myself_module.measurement)
            comp_loaded = PERFEXPERT_TRUE;
    }

    if (comp_loaded == PERFEXPERT_FALSE && PERFEXPERT_TRUE == perfexpert_module_available("icc")) {
        OUTPUT_VERBOSE((5, "%s",
            _CYAN("will use icc as compilation module")));
        myself_module.measurement = (perfexpert_module_measurement_t *) perfexpert_module_get("icc");
        if (NULL != myself_module.measurement)
            comp_loaded = PERFEXPERT_TRUE;
    }

    if (comp_loaded == PERFEXPERT_FALSE && PERFEXPERT_TRUE == perfexpert_module_available("gcc")) {
        OUTPUT_VERBOSE((5, "%s",
            _CYAN("will use gcc as compilation module")));
        myself_module.measurement = (perfexpert_module_measurement_t *) perfexpert_module_get("gcc");
        if (NULL != myself_module.measurement)
            comp_loaded = PERFEXPERT_TRUE;
    }
    if (comp_loaded == PERFEXPERT_FALSE) {
        OUTPUT(("%s", _ERROR("required compilation module not available")));
        return PERFEXPERT_ERROR;
    }

    if (myself_module.measurement->compile() != PERFEXPERT_SUCCESS) {
        OUTPUT(("%s", _ERROR (" failed to compile ")));
        return PERFEXPERT_ERROR;
    }

    return PERFEXPERT_SUCCESS;
}

int macpo_run() {
    struct timespec time_start, time_end, time_diff;
    char *argv[MAX_ARGUMENTS_COUNT];
    int rc = PERFEXPERT_SUCCESS;
    test_t test;
    int i = 0, argc = 0;


    /* Run the global BEFORE program */
    if (NULL != globals.before) {
        char *before[MAX_ARGUMENTS_COUNT];
        perfexpert_string_split(globals.before, before,  ' ');
        OUTPUT_VERBOSE((8, " %s", "running the before program"));
        PERFEXPERT_ALLOC(char, test.output, (strlen(globals.moduledir) + 20));
        sprintf(test.output, "%s/before.output", globals.moduledir);
        test.input = NULL;
        test.info = before[0];

        if (0 != perfexpert_fork_and_wait(&test,
            (char **)before)) {
            OUTPUT(("   %s", _RED("'before' command returns non-zero")));
        }
        PERFEXPERT_DEALLOC(test.output);
    }

    /* Run the BEFORE program */
    if (NULL != my_module_globals.before[0]) {
        OUTPUT_VERBOSE((8, " %s", "running the before program"));
        PERFEXPERT_ALLOC(char, test.output, (strlen(globals.moduledir) + 20));
        sprintf(test.output, "%s/before.output", globals.moduledir);
        test.input = NULL;
        test.info = my_module_globals.before[0];

        if (0 != perfexpert_fork_and_wait(&test,
            (char **)my_module_globals.before)) {
            OUTPUT(("   %s", _RED("'before' command returns non-zero")));
        }
        PERFEXPERT_DEALLOC(test.output);
    }

    /* Create the command line */
    /* add the PREFIX */

    i = 0;
    argc = 0;
    if (NULL != globals.prefix) {
        char *pref[MAX_ARGUMENTS_COUNT];
        perfexpert_string_split(globals.prefix, pref, ' ');
        while (NULL != pref[i]) {
            argv[argc] = pref[i];
            i++;
            argc++;
        }
    }

    i = 0;
    while (NULL != my_module_globals.prefix[i]) {
        argv[argc] = my_module_globals.prefix[i];
        argc++;
        i++;
    }

    /* if we need something for MACPO, put it here. But we don't need it */

    /* add the program */
    argv[argc] = globals.program_full;
    i = 0;
    argc++;
    while (NULL != globals.program_argv[i]) {
        argv[argc] = globals.program_argv[i];
        argc++;
        i++;
    }

    /* Required to indicate the last parameter */
    argv[argc] = NULL;

    /* Not using OUTPUT_VERBOSEbecause I want only one line */
    if (8 <= globals.verbose) {
        printf("%s    %s", PROGRAM_PREFIX, _YELLOW("command line:"));
        for (i = 0; i < argc; i++) {
            printf(" %s", argv[i]);
        }
        printf("\n");
    }

    PERFEXPERT_ALLOC(char, test.output,
        (strlen(globals.moduledir) + 8));
    sprintf(test.output, "%s/macpo-run.output", globals.moduledir);
    test.input  = NULL;
    test.info   = globals.program;

    /* fork_and_wait */
   // clock_gettime(CLOCK_MONOTONIC, &time_start);
    rc = perfexpert_fork_and_wait(&test, (char **)argv);
   // clock_gettime(CLOCK_MONOTONIC, &time_end);

    PERFEXPERT_DEALLOC(test.output);
    if (PERFEXPERT_FALSE == my_module_globals.ignore_return_code) {
        switch (rc) {
            case PERFEXPERT_FAILURE:
            case PERFEXPERT_ERROR:
                OUTPUT(("%s (return code: %d) Usually, this means that an error"
                    " happened during the program execution. To see the program"
                    "'s output, check the content of this file: [%s]. If you "
                    "want to PerfExpert ignore the return code next time you "
                    "run this program, set the 'return-code' option for the "
                    "macpo module. See 'perfepxert -H macpo' for details.",
                    _ERROR("the target program returned non-zero"), rc,
                    test.output));
                return PERFEXPERT_ERROR;

            case PERFEXPERT_SUCCESS:
                OUTPUT_VERBOSE((7, "[ %s  ]", _BOLDGREEN("OK")));
                break;

            default:
                break;
        }
    }
    /* Calculate and display runtime */
    //perfexpert_time_diff(&time_diff, &time_start, &time_end);
    //OUTPUT(("   [1/1] %lld.%.9ld seconds (includes measurement overhead)",
    //    (long long)time_diff.tv_sec, time_diff.tv_nsec));

    // Run the global after command
    if (NULL != globals.after) {
        char *after[MAX_ARGUMENTS_COUNT];
        perfexpert_string_split(globals.after, after, ' ');
        PERFEXPERT_ALLOC(char, test.output, (strlen(globals.moduledir) + 20));
        sprintf(test.output, "%s/after.output", globals.moduledir);
        test.input = NULL;
        test.info = after[0];

        if (0 != perfexpert_fork_and_wait(&test,
            (char **)after)) {
            OUTPUT(("%s", _RED("'after' command return non-zero")));
        }
        PERFEXPERT_DEALLOC(test.output);
    }

    /* Run the AFTER program */
    if (NULL != my_module_globals.after[0]) {
        PERFEXPERT_ALLOC(char, test.output, (strlen(globals.moduledir) + 20));
        sprintf(test.output, "%s/after.output", globals.moduledir);
        test.input = NULL;
        test.info = my_module_globals.after[0];

        if (0 != perfexpert_fork_and_wait(&test,
            (char **)my_module_globals.after)) {
            OUTPUT(("%s", _RED("'after' command return non-zero")));
        }
        PERFEXPERT_DEALLOC(test.output);
    }
    return PERFEXPERT_SUCCESS;
}

int macpo_analyze() {
    char * argv[3];
    int rc;
    test_t test;

    argv[0] = "macpo-analyze";
    argv[1] = "macpo.out";
    argv[2] = NULL;

    PERFEXPERT_ALLOC(char, test.output, (strlen(globals.moduledir) + 22));
    snprintf(test.output, strlen(globals.moduledir) + 22,
            "%s/macpo-analyze.output", globals.moduledir);
    OUTPUT_VERBOSE((6, "OUTPUT: %s" , test.output));
    test.input = NULL;
    test.info = globals.program;

    OUTPUT_VERBOSE((6, "   COMMAND=[%s %s]", argv[0], argv[1]));

    rc = perfexpert_fork_and_wait(&test, (char **)argv);

    switch (rc) {
        case PERFEXPERT_FAILURE:
        case PERFEXPERT_ERROR:
            OUTPUT(("%s (return code: %d) Usually, this means that an error"
                " happened during the program execution. To see the program"
                "'s output, check the content of this file: [%s]. If you "
                "want to PerfExpert ignore the return code next time you "
                "run this program, set the 'return-code' option for the "
                "macpo module. See 'perfepxert -H macpo' for details.",
                _ERROR("the target program returned non-zero"), rc,
                test.output));
            return PERFEXPERT_ERROR;

        case PERFEXPERT_SUCCESS:
            OUTPUT_VERBOSE((7, "[ %s  ]", _BOLDGREEN("OK")));
            break;

        default:
            break;
    }

    PERFEXPERT_DEALLOC(test.output);
    if (PERFEXPERT_SUCCESS != macpo_restore_code()) {
        OUTPUT(("%s", _ERROR (" failed to restore the code ")));
        return PERFEXPERT_ERROR;
    }
    if (PERFEXPERT_SUCCESS != macpo_compile()) {
        OUTPUT(("%s", _ERROR (" failed to compile after restoring the code ")));
    }
    return PERFEXPERT_SUCCESS;
}

/* This function restores the code, removing the instrumented code by the original */
int macpo_restore_code() {
    int i;
    char *file;
    char *backfile;
    char *filename;
    char *folder;
    char *fullpath_inst;
    char *fullpath;
    char *destfolder;
 
    OUTPUT_VERBOSE((9, "Restoring code after running the instrumented version"));   
    for (i=0; i<my_module_globals.instrument.maxfiles; ++i) {
        file = my_module_globals.instrument.list[i].file;
        backfile = my_module_globals.instrument.list[i].backfile;

        if (PERFEXPERT_SUCCESS != perfexpert_util_file_exists(file)) {
            return PERFEXPERT_SUCCESS;
        }

        if (PERFEXPERT_SUCCESS != perfexpert_util_filename_only(file, &filename)) {
            return PERFEXPERT_SUCCESS;
        }

        if (PERFEXPERT_SUCCESS != perfexpert_util_path_only(file, &folder)) {
            return PERFEXPERT_ERROR;
        }
       
        /* Make sure that we create the folder for the instrumented code */ 
        PERFEXPERT_ALLOC(char, destfolder, (strlen(globals.moduledir) + strlen(folder) + 4));
        snprintf(destfolder, strlen(globals.moduledir) + strlen(folder) + 4, "%s/%s", globals.moduledir, folder);
        perfexpert_util_make_path(destfolder);
        PERFEXPERT_DEALLOC(destfolder);

        PERFEXPERT_ALLOC(char, fullpath, strlen(globals.moduledir) + strlen(filename) +
                         strlen(folder) + 8);
        PERFEXPERT_ALLOC(char, fullpath_inst, strlen(globals.moduledir) + strlen(filename) +
                         strlen(folder) + 8);
        
        snprintf(fullpath, strlen(globals.moduledir) + strlen(folder) + strlen(filename) + 8,
                 "%s/%s/%s", globals.moduledir, folder, filename);
        snprintf(fullpath_inst, strlen(globals.moduledir) + strlen(folder) + strlen(filename) + 8,
                 "%s/%s/inst_%s", globals.moduledir, folder, filename);

//        OUTPUT_VERBOSE((9, "Copying instrumented file %s to %s", file, fullpath_inst));
        if (PERFEXPERT_SUCCESS != perfexpert_util_file_rename(file, fullpath_inst)) {
            OUTPUT(("%s impossible to copy file %s to %s", _ERROR("IO ERROR"), file, fullpath_inst));
        }
//        OUTPUT_VERBOSE((9, "Restoring original file %s to %s", backfile, file));
        if (PERFEXPERT_SUCCESS != perfexpert_util_file_copy(file, backfile)) {
            OUTPUT(("%s impossible to copy file %s to %s", _ERROR("IO ERROR"), backfile, file));
        }
        PERFEXPERT_DEALLOC(fullpath);
        PERFEXPERT_DEALLOC(fullpath_inst);
        
    }
    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
