/*
 * Copyright (c) 2013      University of Texas at Austin. All rights reserved.
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
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

#ifndef OPTTRAN_OUTPUT_H
#define OPTTRAN_OUTPUT_H

#ifndef	_STDIO_H_
#include <stdio.h>
#endif

#ifndef _STDLIB_H_
#include <stdlib.h>
#endif

#ifndef _STDARG_H
#include <stdarg.h>
#endif

#ifndef _STRING_H_
#include <string.h>
#endif

#ifndef _UNISTD_H_
#include <unistd.h>
#endif

/**
 *  Main macro for use in sending debugging output to screen.
 *
 *  @see opttran_output()
 */
#ifndef OPTTRAN_OUTPUT
#define OPTTRAN_OUTPUT(a) output a
#endif
#ifndef OPTTRAN_OUTPUT_VERBOSE
#define OPTTRAN_OUTPUT_VERBOSE(a) output_verbose a
#endif

/**
 * Global function to send output to screen. This function should never be
 * called directly. The macro OPTTRAN_OUTPUT should be called instead. Use this
 * funtion for error messages. Debug and informational messages should use
 * output_verbose().
 *
 * @param[in] *format Formatted string output conversion
 */
static void output(const char *format, ...) {
    va_list arglist;
    char *str = NULL;
    char *temp_str = NULL;
    size_t total_len;
    
    va_start(arglist, format);
    vasprintf(&str, format, arglist);
    total_len = strlen(str);
    
    temp_str = (char *) malloc(total_len + 13);
    if (NULL == temp_str) {
        printf("[opttran_output] ERROR: OUT OF MEMORY\n");
    }
    
    snprintf(temp_str, total_len + 13, "%s\n", str);
    
    write(fileno(stdout), temp_str, (int)strlen(temp_str));
    fflush(stdout);
    
    free(str);
    free(temp_str);
    
    va_end(arglist);
}

/**
 * Global function to send output to screen. This function should never be
 * called directly. The macro OPTTRAN_OUTPUT should be called instead. Use this
 * function for debug and informational messages. Error messages should use
 * output().
 *
 * @param[in] level   Starting debug level from where this message should be
 *                    outputted
 * @param[in] *format Formatted string output conversion
 */
static void output_verbose(int level, const char *format, ...) {
    va_list arglist;
    char *str = NULL;
    char *temp_str = NULL;
    size_t total_len;
    
    if (globals.verbose_level >= level) {
        va_start(arglist, format);
        vasprintf(&str, format, arglist);
        total_len = strlen(str);
        
        temp_str = (char *) malloc(total_len + 13);
        if (NULL == temp_str) {
            printf("[opttran_output] ERROR: OUT OF MEMORY\n");
        }
        
        snprintf(temp_str, total_len + 13, "%s\n", str);
        
        write(fileno(stdout), temp_str, (int)strlen(temp_str));
        fflush(stdout);
        
        free(str);
        free(temp_str);
        
        va_end(arglist);
    }
}

#endif /* OPTTRAN_OUTPUT_H */
