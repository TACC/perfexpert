/*
 * Copyright (c) 2013  University of Texas at Austin. All rights reserved.
 * Copyright (c) 2007      Voltaire. All rights reserved.
 * Copyright (c) 2004-2005 The Trustees of Indiana University and Indiana
 *                         University Research and Technology Corporation.
 *                         All rights reserved.
 * Copyright (c) 2004-2006 The University of Tennessee and The University
 *                         of Tennessee Research Foundation. All rights
 *                         reserved.
 * Copyright (c) 2004-2005 High Performance Computing Center Stuttgart,
 *                         University of Stuttgart. All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
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

#ifndef PERFEXPERT_LOG_H_
#define PERFEXPERT_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif
    
#ifndef	_STDIO_H
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

#ifndef PERFEXPERT_CONSTANTS_H_
#include "perfexpert_constants.h"
#endif

/**
 *  Main macro for use in sending debugging output to screen.
 *
 *  @see perfexpert_log()
 */
#ifndef LOG
#define LOG(a) perfexpert_log a
#endif

/**
 * Global function to send data to logfile. This function should never be
 * called directly. The macro LOG should be called instead.
 *
 * @param[in] *format Formatted string output conversion
 */
static void perfexpert_log(const char *format, ...) {
    va_list arglist;
    FILE    *logfile_FP;
    char    *str = NULL;
    char    temp_str[MAX_LOG_ENTRY];
    char    logfile[BUFFER_SIZE];
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

    bzero(logfile, BUFFER_SIZE);
    sprintf(logfile, "%s/%s", getenv("HOME"), LOGFILE);

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
