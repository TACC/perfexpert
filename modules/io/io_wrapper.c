/*
 * Copyright (c) 2011-2017  University of Texas at Austin. All rights reserved.
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
 * Authors: Antonio Gomez-Iglesias
 *
 * $HEADER$
 */

#ifndef PREFEXPERT_MODULE_IO_WRAPPER_H_
#define PREFEXPERT_MODULE_IO_WRAPPER_H_

#ifdef __cplusplus
extern "C" {
#endif

#define _GNU_SOURCE
#include <stddef.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <pty.h>
#include <sys/types.h>
#include <signal.h>

#include <fcntl.h>


#include <signal.h>
#include <unistd.h>

#define UNW_LOCAL_ONLY
#include <libunwind.h>

#include "io.h"

char *executable;

static ssize_t (*real_write)(int fd, const void *buf, size_t count) = NULL;
static ssize_t (*real_fwrite)(const void *ptr, size_t size, size_t n, FILE *s) = NULL;
static ssize_t (*real_read)(int fd, void *buf, size_t count) = NULL;
static ssize_t (*real_fread)(void * ptr, size_t size, size_t n, FILE* s) = NULL;
static int (*real_fprintf)( FILE * __restrict stream, const char * __restrict format, ... ) = NULL;
static int (*real_fscanf)( FILE * stream, const char * format, ... ) = NULL;
static int (*real_fputs)( const char * str, FILE * stream ) = NULL;
char * (*real_fgets)( char * str, int num, FILE * stream ) = NULL;
static FILE* (*real_fopen)(const char *path, const char *mode) = NULL;
static int (*real_fclose)(FILE *stream) = NULL;

my_module_globals_t my_module_globals; 

void add_function_to_data(char * function_name, long address, int function) {
    int found = 0;
    int i;

    for (i=0; i<my_module_globals.data[function].size; ++i) {
      if ((strcmp (my_module_globals.data[function].code[i].function_name, function_name)==0) && 
                  (my_module_globals.data[function].code[i].address==address)) {
        found = 1;
        printf ("Function and address founds [%d]\n", function);
        my_module_globals.data[function].code[i].count++;
        break;
      }
    }
    if (found==0) {
        printf ("not found, adding new [%d]\n", function);
        //PERFEXPERT_REALLOC((code_function_t), my_module_globals.data[function].code, (my_module_globals.data[function].size+1)*sizeof(code_function_t));
        my_module_globals.data[function].code = (code_function_t *) realloc (my_module_globals.data[function].code, (my_module_globals.data[function].size+1)*sizeof(code_function_t));
        strcpy (my_module_globals.data[function].code[my_module_globals.data[function].size].function_name, function_name);
        my_module_globals.data[function].code[my_module_globals.data[function].size].address = address;
        my_module_globals.data[function].code[my_module_globals.data[function].size].count=1;
        my_module_globals.data[function].size++;
    }
}

// Call this function to get a backtrace.
void capture_backtrace(char *executable, int function) {
    char name[256];
    unw_cursor_t cursor; unw_context_t uc;
    unw_word_t ip, sp, offp;

    unw_getcontext(&uc);
    unw_init_local(&cursor, &uc);

    int level=0;
    while (unw_step(&cursor) > 0)
    {
        if (level==0) {
            level++;
            continue;
        }
        if (level>1) {
            break;
        }
        if (level==1) {
            char file[256];
            int line = 0;

            name[0] = '\0';
            unw_get_proc_name(&cursor, name, 256, &offp);
            unw_get_reg(&cursor, UNW_REG_IP, &ip);
            unw_get_reg(&cursor, UNW_REG_SP, &sp);
            
            printf ("[%d] Adding function %s and address %lu\n", function, name, (long) ip);
            add_function_to_data (name, (long)ip, function);

            level++;
        }
    }
}

void init() __attribute__ ((constructor));

void init() {
    executable = getenv("PERFEXPERT_PROGRAM");
    return;
}

void finish() __attribute__ ((destructor));
void finish() {
    int i, j;
    FILE *output;
    char buffer[256];

    buffer[255]=0;
    printf("This is the finish method\n");
    real_fopen=dlsym(RTLD_NEXT, "fopen");
    real_fclose=dlsym(RTLD_NEXT, "fclose");
    real_fwrite=dlsym(RTLD_NEXT, "fwrite");
    real_fprintf=dlsym(RTLD_NEXT, "fprintf");

    output=real_fopen("perfexpert_io_output", "w");

    for (i=0; i<MAX_FUNCTIONS; ++i) {
        real_fprintf (output, "Function: %d\n", i);
        for (j=0; j<my_module_globals.data[i].size; ++j) {
            real_fprintf(output, "Source code function: %s -- %ld -- %d\n", my_module_globals.data[i].code[j].function_name, my_module_globals.data[i].code[j].address, my_module_globals.data[i].code[j].count-1);
        }
    }
    real_fclose(output);
}

FILE* fopen(const char *path, const char *mode) {
    executable = getenv("PERFEXPERT_PROGRAM");
    capture_backtrace (executable, FOPEN);
    real_fopen=dlsym(RTLD_NEXT, "fopen");
    return real_fopen(path, mode);
}

size_t fwrite(const void * ptr, size_t size, size_t n, FILE * s) {
    executable = getenv("PERFEXPERT_PROGRAM");
    capture_backtrace(executable, FWRITE);
    real_fwrite=dlsym(RTLD_NEXT, "fwrite");
    return real_fwrite(ptr, size, n, s);
}

int fscanf ( FILE * __restrict stream, const char *__restrict format, ... ) {
    int retval;
    executable = getenv("PERFEXPERT_PROGRAM");
    printf ("\nCalling fscanf\n\n");
    va_list argptr;
    va_start(argptr, format);
    if (stream!=stderr && stream!=stdout) {
        capture_backtrace(executable, FSCANF);
    }
    retval=vfscanf(stream, format, argptr);
    va_end(argptr);
    return retval;
}

int fprintf ( FILE * __restrict stream, const char *__restrict format, ... ) {
    int retval;
    executable = getenv("PERFEXPERT_PROGRAM");
    printf ("\nCalling fprintf\n\n");
    va_list argptr;
    va_start(argptr, format);
    if (stream!=stderr && stream!=stdout) {
        capture_backtrace(executable, FPRINTF);
    }
    retval=vfprintf(stream, format, argptr);
    va_end(argptr);
    return retval;
}

size_t fread(void *ptr, size_t size, size_t n, FILE * s) {
    executable = getenv("PERFEXPERT_PROGRAM");
    capture_backtrace (executable, FREAD);
    real_fread=dlsym(RTLD_NEXT, "fread");
    return real_fread(ptr, size, n, s);
}

char * fgets ( char * str, int num, FILE * stream ) {
    executable = getenv("PERFEXPERT_PROGRAM");
    printf ("THIS IS FGETS\n");
    capture_backtrace(executable, FGETS);
    real_fgets=dlsym(RTLD_NEXT, "fgets");
    return real_fgets (str, num, stream);
}

//fprintf without formatted strings actually calls fputs
int fputs ( const char * str, FILE * stream ) {
    executable = getenv ("PERFEXPERT_PROGRAM");
    printf("THIS IS FPUTS\n");
    capture_backtrace (executable, FPUTS);
    real_fputs=dlsym(RTLD_NEXT, "fputs");
    return real_fputs (str, stream);
}

int fclose(FILE *stream) {
    executable = getenv("PERFEXPERT_PROGRAM");
    capture_backtrace (executable, FCLOSE);
    real_fclose=dlsym(RTLD_NEXT, "fclose");
    return real_fclose(stream);
}


#ifdef __cplusplus
}
#endif

#endif /* PREFEXPERT_MODULE_IO_WRAPPER_H_ */
