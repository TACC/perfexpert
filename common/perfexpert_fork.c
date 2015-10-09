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

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/limits.h>

#include "common/perfexpert_fake_globals.h"
#include "common/perfexpert_constants.h"
#include "common/perfexpert_fork.h"
#include "common/perfexpert_output.h"
#include "common/perfexpert_list.h"
#include "install_dirs.h"

/* perfexpert_fork_and_wait */
int perfexpert_fork_and_wait(test_t *test, char *argv[]) {
    int pipe1[2], pipe2[2], input_FP = 0, output_FP = 0;
    pid_t pid = -1;
    int r_bytes = 0, w_bytes = 0, rc = PERFEXPERT_UNDEFINED;
    char *temp_str, buffer[MAX_BUFFER_SIZE];

    OUTPUT_VERBOSE ((9, "checking %s", argv[0]));
    /* Sanity check: what is the full path of the binary? */
    if (PERFEXPERT_SUCCESS != perfexpert_util_full_path(argv[0], &temp_str)) {
        OUTPUT(("%s", _ERROR("file does not exist or is not in the PATH")));
        return PERFEXPERT_ERROR;
    }
    OUTPUT_VERBOSE ((9,"[%s] %s", argv[0], "is in the PATH"));
    /* Sanity check: does the binary exists and is executable? */
    if (PERFEXPERT_ERROR == perfexpert_util_file_is_exec(temp_str)) {
        OUTPUT(("%s (%s)", _ERROR((char *)"file is not executable"), temp_str));
        return PERFEXPERT_ERROR;
    }

    #define PARENT_READ  pipe1[0]
    #define CHILD_WRITE  pipe1[1]
    #define CHILD_READ   pipe2[0]
    #define PARENT_WRITE pipe2[1]

    /* Flush everything...*/
    fflush(stdout);
    fflush(stderr);

    /* Creating pipes */
    if (-1 == pipe(pipe1)) {
        OUTPUT(("%s", _ERROR((char *)"unable to create pipe1")));
        return PERFEXPERT_ERROR;
    }
    if (-1 == pipe(pipe2)) {
        OUTPUT(("%s", _ERROR((char *)"unable to create pipe2")));
        return PERFEXPERT_ERROR;
    }

    /* Forking child */
    pid = fork();
    if (-1 == pid) {
        OUTPUT(("%s", _ERROR((char *)"unable to fork")));
        return PERFEXPERT_ERROR;
    }

    if (0 == pid) {
        /* Child */
        close(PARENT_WRITE);
        close(PARENT_READ);

        OUTPUT_VERBOSE((10, "   %s %s", _CYAN((char *)"program"), argv[0]));

        if (-1 == dup2(CHILD_READ, STDIN_FILENO)) {
            OUTPUT(("%s", _ERROR((char *)"unable to DUP STDIN")));
            return PERFEXPERT_ERROR;
        }
        if (NULL != test->output) {
            if (-1 == dup2(CHILD_WRITE, STDOUT_FILENO)) {
                OUTPUT(("%s", _ERROR((char *)"unable to DUP STDOUT")));
                return PERFEXPERT_ERROR;
            }
            if (-1 == dup2(CHILD_WRITE, STDERR_FILENO)) {
                OUTPUT(("%s", _ERROR((char *)"unable to DUP STDERR")));
                return PERFEXPERT_ERROR;
            }
        }

        execvp(argv[0], argv);

        OUTPUT(("child process failed to run, check if program exists (%s)",
            errno));
        exit(PERFEXPERT_FORK_ERROR);
    } else {
        /* Parent */
        close(CHILD_READ);
        close(CHILD_WRITE);

        /* If there is any input file, open and send it to the child process */
        if (NULL != test->input) {
            if (-1 == (input_FP = open(test->input, O_RDONLY))) {
                OUTPUT(("%s (%s)", _ERROR((char *)"unable to open input file"),
                    test->input));
                return PERFEXPERT_ERROR;
            } else {
                OUTPUT_VERBOSE((10, "   %s   %s", _CYAN((char *)"stdin"),
                    test->input));
                bzero(buffer, MAX_BUFFER_SIZE);
                while (0 != (r_bytes = read(input_FP, buffer,
                    MAX_BUFFER_SIZE))) {
                    w_bytes = write(PARENT_WRITE, buffer, r_bytes);
                    bzero(buffer, MAX_BUFFER_SIZE);
                }
                close(input_FP);
                close(PARENT_WRITE);
            }
        }

        /* Read child process' answer and write it to output file */
        if (NULL != test->output) {
            OUTPUT_VERBOSE((10, "   %s  %s", _CYAN((char *)"stdout"),
                test->output));

            if (-1 == (output_FP = open(test->output, O_CREAT|O_WRONLY|O_APPEND,
                0644))) {
                OUTPUT(("%s (%s)", _ERROR((char *)"unable to open output file"),
                    test->output));
                return PERFEXPERT_ERROR;
            } else {
                bzero(buffer, MAX_BUFFER_SIZE);
                while (0 != (r_bytes = read(PARENT_READ, buffer,
                    MAX_BUFFER_SIZE))) {
                    w_bytes = write(output_FP, buffer, r_bytes);
                    bzero(buffer, MAX_BUFFER_SIZE);
                }
                close(output_FP);
                close(PARENT_READ);
            }
        }

        /* Just wait for the child... */
        wait(&rc);
        OUTPUT_VERBOSE((10, "   %s  %d", _CYAN((char *)"result"), rc >> 8));
    }

    /* Showing the result */
    OUTPUT_VERBOSE((7, "   [%s] >> [%s] (rc=%d)", argv[0],
        test->info ? test->info : "", rc >> 8));

    return rc >> 8;
}

#ifdef __cplusplus
}
#endif
