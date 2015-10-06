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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "common/perfexpert_fake_globals.h"
#include "common/perfexpert_constants.h"
#include "common/perfexpert_log.h"
#include "common/perfexpert_output.h"

/* perfexpert_log */
void perfexpert_log(const char *format, ...) {
    va_list arglist;
    char *str = NULL, temp_str[MAX_LOG_ENTRY], logfile[MAX_BUFFER_SIZE],
        *longdate = NULL;
    FILE *logfile_FP;
    time_t now_time;

    time(&now_time);
    longdate = asctime(localtime(&now_time));
    longdate[strlen(longdate) - 1] = 0;

    va_start(arglist, format);
    vasprintf(&str, format, arglist);

    bzero(temp_str, MAX_LOG_ENTRY);
    sprintf(temp_str, "%d %s %s %ld --- %s\n", now_time, longdate,
        PROGRAM_PREFIX, (int)getpid(), str);

    bzero(logfile, MAX_BUFFER_SIZE);
    sprintf(logfile, "%s/%s", getenv("HOME"), PERFEXPERT_LOGFILE);

    logfile_FP = fopen(logfile, "a");
    if (logfile_FP) {
        fwrite(temp_str, 1, (int)strlen(temp_str), logfile_FP);
    }
    fclose(logfile_FP);
    free(str);
    va_end(arglist);
}

#ifdef __cplusplus
}
#endif
