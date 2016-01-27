/*
 * Copyright (c) 2011-2013 University of Texas at Austin. All rights reserved.
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

#ifndef PERFEXPERT_OUTPUT_H_
#define PERFEXPERT_OUTPUT_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _STDIO_H_
#include <stdio.h>
#endif

#ifndef _STDLIB_H_
#include <stdlib.h>
#endif

#ifndef _STDARG_H_
#include <stdarg.h>
#endif

#ifndef _STRING_H_
#include <string.h>
#endif

#ifndef _UNISTD_H_
#include <unistd.h>
#endif

#include "perfexpert_constants.h"

/* see perfexpert_output function */
#ifndef OUTPUT
#define OUTPUT(a) output a
#endif
#ifndef OUTPUT_VERBOSE
#define OUTPUT_VERBOSE(a) output_verbose a
#endif

#ifndef PROGRAM_PREFIX
#define PROGRAM_PREFIX "[---]"
#endif

/* Colorful output functions, definitions and other stuff
 * See http://en.wikipedia.org/wiki/ANSI_escape_code for a complete list
 */
#define ATTR_NONE     0
#define ATTR_BRIGHT   1
#define ATTR_BLINK    5
#define COLOR_BLACK   0
#define COLOR_RED     1
#define COLOR_GREEN   2
#define COLOR_YELLOW  3
#define COLOR_BLUE    4
#define COLOR_MAGENTA 5
#define COLOR_CYAN    6
#define COLOR_WHITE   7

/* Function declarations */
static inline char* colorful(int attr, int fg, int bg, char *s);
static inline char* colorful_err(int attr, int fg, int bg, const char *s,
    const char *file, int line, const char *function);
static inline void output(const char *format, ...);
static inline void output_verbose(int level, const char *format, ...);

/* Use all your artistic side to combine the color as you prefer */
#define _ERROR(a) colorful_err(ATTR_BRIGHT, COLOR_RED, COLOR_BLACK, a, \
    __FILE__, __LINE__, __func__)

#define _RED(a)        colorful(ATTR_NONE,   COLOR_RED,     COLOR_BLACK, a)
#define _GREEN(a)      colorful(ATTR_NONE,   COLOR_GREEN,   COLOR_BLACK, a)
#define _YELLOW(a)     colorful(ATTR_NONE,   COLOR_YELLOW,  COLOR_BLACK, a)
#define _BLUE(a)       colorful(ATTR_NONE,   COLOR_BLUE,    COLOR_BLACK, a)
#define _MAGENTA(a)    colorful(ATTR_NONE,   COLOR_MAGENTA, COLOR_BLACK, a)
#define _CYAN(a)       colorful(ATTR_NONE,   COLOR_CYAN,    COLOR_BLACK, a)
#define _WHITE(a)      colorful(ATTR_BRIGHT, COLOR_WHITE,   COLOR_BLACK, a)
#define _BOLDRED(a)    colorful(ATTR_BRIGHT, COLOR_RED,     COLOR_BLACK, a)
#define _BOLDGREEN(a)  colorful(ATTR_BRIGHT, COLOR_GREEN,   COLOR_BLACK, a)
#define _BOLDYELLOW(a) colorful(ATTR_BRIGHT, COLOR_YELLOW,  COLOR_BLACK, a)

/* colorful (never call this function directly ) */
static inline char* colorful(int attr, int fg, int bg, char *s) {
    if (PERFEXPERT_TRUE == globals.colorful) {
        static char *colored;
        static int size;

        if ((strlen(s) + 15) > size) {
            colored = (char *)realloc(colored, (strlen(s) + 15));
            size = strlen(s) + 15;
        }
        bzero(colored, size);
        sprintf(colored, "\x1b[%d;%d;%dm%s\x1b[0m", attr, fg + 30, bg + 40, s);

        return colored;
    } else {
        return s;
    }
}

/* colorful_err (never call this function directly ) */
static inline char* colorful_err(int attr, int fg, int bg, const char *s,
    const char *file, int line, const char *function) {
    if (PERFEXPERT_TRUE == globals.colorful) {
        static char *colored_err;
        static int size = 0;

        if ((strlen(s) + strlen(file) + strlen(function) + 25) > size) {
            colored_err = (char *)realloc(colored_err,
                (strlen(s) + strlen(file) + strlen(function) + 25));
            size = strlen(s) + 25;
        }
        bzero(colored_err, size);
        sprintf(colored_err, "[%s@%s:%d] \x1b[%d;%d;%dm%s\x1b[0m", function,
            file, line, attr, fg + 30, bg + 40, s);

        return colored_err;
    } else {
        static char *err;
        static int size;

        if ((strlen(s) + strlen(file) + strlen(function) + 10) > size) {
            err = (char *)realloc(err,
                (strlen(s) + strlen(file) + strlen(function) + 25));
            size = strlen(s) + 10;
        }
        bzero(err, size);
        sprintf(err, "[%s@%s:%d] %s", function, file, line, s);

        return err;
    }
}

/* output (never call this functions directly) */
static inline void output(const char *format, ...) {
    char *temp_str = NULL;
    char *str = NULL;
    int rc = 0;
    size_t total_len = 0;
    va_list arglist;

    va_start(arglist, format);
    rc = vasprintf(&str, format, arglist);
    total_len = strlen(str) + 14 + strlen(PROGRAM_PREFIX);

    temp_str = (char *)malloc(total_len);
    if (NULL == temp_str) {
        printf("%s out of memory\n", PROGRAM_PREFIX);
    }
    snprintf(temp_str, total_len, "%s %s\n", PROGRAM_PREFIX, str);
    total_len = write(fileno(stdout), temp_str, (int)strlen(temp_str));
    fflush(stdout);

    free(str);
    free(temp_str);

    va_end(arglist);
}

/* output_verbose (never call this function directly) */
static inline void output_verbose(int level, const char *format, ...) {
    va_list arglist;
    char *temp_str = NULL;
    char *str = NULL;
    int rc = 0;
    size_t total_len = 0;

    #ifdef _I_AM_A_MODULE_
    if (my_module_globals.verbose >= level) {
    #else
    if (globals.verbose >= level) {
    #endif
        va_start(arglist, format);
        rc = vasprintf(&str, format, arglist);
        total_len = strlen(str) + 14 + strlen(PROGRAM_PREFIX);

        temp_str = (char *) malloc(total_len);
        if (NULL == temp_str) {
            printf("%s out of memory\n", PROGRAM_PREFIX);
        }
        snprintf(temp_str, total_len, "%s %s\n", PROGRAM_PREFIX, str);
        total_len = write(fileno(stdout), temp_str, (int)strlen(temp_str));
        fflush(stdout);

        free(str);
        free(temp_str);

        va_end(arglist);
    }
}

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_OUTPUT_H */
