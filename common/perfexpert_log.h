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

#ifndef PERFEXPERT_LOG_H_
#define PERFEXPERT_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _STDIO_H
#include <stdio.h>
#endif

#ifndef _STDLIB_H
#include <stdlib.h>
#endif

#ifndef _STDARG_H
#include <stdarg.h>
#endif

#ifndef _STRING_H
#include <string.h>
#endif

#ifndef _UNISTD_H
#include <unistd.h>
#endif

#ifndef _TIME_H
#include <time.h>
#endif

#include "common/perfexpert_constants.h"

/* see perfexpert_log function */
#ifndef LOG
#define LOG(a) perfexpert_log a
#endif

/* perfexpert_log */
static void perfexpert_log(const char *format, ...) {
    va_list arglist;
    FILE    *logfile_FP;
    char    *str = NULL;
    char    temp_str[MAX_LOG_ENTRY];
    char    logfile[MAX_BUFFER_SIZE];
    time_t  now_time;
    char    *longdate;

    time(&now_time);
    longdate = asctime(localtime(&now_time));
    longdate[strlen(longdate) - 1] = 0;

    va_start(arglist, format);
    vasprintf(&str, format, arglist);

    bzero(temp_str, MAX_LOG_ENTRY);
    sprintf(temp_str, "%d %s %s %ld --- %s\n", now_time, longdate,
        PROGRAM_PREFIX, globals.pid, str);

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

#endif /* PERFEXPERT_LOG_H */
