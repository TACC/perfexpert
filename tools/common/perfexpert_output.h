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

#ifndef PERFEXPERT_OUTPUT_H_
#define PERFEXPERT_OUTPUT_H_

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

/**
 *  Main macro for use in sending debugging output to screen.
 *
 *  @see perfexpert_output()
 */
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

static inline char* colorful(int attr, int fg, int bg, char* str) {
    if (1 == globals.colorful) {
        static char *colored;

        colored = (char *)realloc(colored, strlen(str) + 15);
        bzero(colored, strlen(str) + 15);
        sprintf(colored, "\x1b[%d;%d;%dm%s\x1b[0m", attr, fg+30, bg+40, str);

        return colored;
    } else {
        return str;
    }
}

/* Use all your artistic side to combine the color as you prefer */
#define _ERROR(a)      colorful(ATTR_BLINK,  COLOR_RED,     COLOR_BLACK, a)
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

/**
 * Global function to send output to screen. This function should never be
 * called directly. The macro OUTPUT should be called instead. Use this function
 * for error messages. Debug and informational messages should use
 * OUTPUT_VERBOSE().
 *
 * @param[in] *format Formatted string output conversion
 */
static inline void output(const char *format, ...) {
    va_list arglist;
    char *str = NULL;
    char *temp_str = NULL;
    size_t total_len;
    int rc;
    
    va_start(arglist, format);
    rc = vasprintf(&str, format, arglist);
    total_len = strlen(str) + 14 + strlen(PROGRAM_PREFIX);
    
    temp_str = (char *) malloc(total_len);
    if (NULL == temp_str) {
        printf("%s Error: out of memory\n", PROGRAM_PREFIX);
    }
    
    snprintf(temp_str, total_len, "%s %s\n", PROGRAM_PREFIX, str);
    
    total_len = write(fileno(stdout), temp_str, (int)strlen(temp_str));
    fflush(stdout);
    
    free(str);
    free(temp_str);
    
    va_end(arglist);
}

/**
 * Global function to send output to screen. This function should never be
 * called directly. The macro OUTPUT_VERBOSE should be called instead. Use this
 * function for debug and informational messages. Error messages should use
 * OUTPUT().
 *
 * @param[in] level   Starting debug level from where this message should be
 *                    outputted
 * @param[in] *format Formatted string output conversion
 */
static inline void output_verbose(int level, const char *format, ...) {
    va_list arglist;
    char *str = NULL;
    char *temp_str = NULL;
    size_t total_len;
    int rc;
    
    if (globals.verbose_level >= level) {
        va_start(arglist, format);
        rc = vasprintf(&str, format, arglist);
        total_len = strlen(str) + 14 + strlen(PROGRAM_PREFIX);
        
        temp_str = (char *) malloc(total_len);
        if (NULL == temp_str) {
            printf("%s Error: out of memory\n", PROGRAM_PREFIX);
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
