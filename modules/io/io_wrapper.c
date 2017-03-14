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


#include <fcntl.h>


#include <signal.h>
#include <unistd.h>

#define UNW_LOCAL_ONLY
#include <libunwind.h>

#define FOPEN 1
#define FREAD 2
#define FWRITE 3

//#include "common/perfexpert_unwind.h"
//#include "common/perfexpert_output.h"
//#include "common/perfexpert_fake_globals.h"

char *program;

static ssize_t (*real_write)(int fd, const void *buf, size_t count) = NULL;
static ssize_t (*real_fwrite)(const void *ptr, size_t size, size_t n, FILE *s) = NULL;
static ssize_t (*real_read)(int fd, void *buf, size_t count) = NULL;
static ssize_t (*real_fread)(void * ptr, size_t size, size_t n, FILE* s) = NULL;
static int (*real_open)(const char *path, int oflag, ... ) = NULL;
static FILE* (*real_fopen)(const char *path, const char *mode) = NULL;
static int (*real_close)(int fd);
static int (*real_fclose)(FILE *stream) = NULL;

int perfexpert_unwind_get_file_line (unw_word_t addr, char *file, size_t len, int *line, char *executable) {
    static char buf[256];
    char out[256] = {0};
    char *p; 

    if (executable == NULL)
        return -1;
    // prepare command to be executed
    // our program need to be passed after the -e parameter
    sprintf (buf, "/usr/bin/addr2line -C -e %s -f -i %lx", executable, addr);
    //sprintf (buf, "/usr/bin/addr2line -C -e ./example -f -i %lx", addr);
    printf ("Running this command: %s\n", buf);
    //return 1;

    FILE* f = popen (buf, "r");
    int flags;
    int fd = fileno(f);
    flags = fcntl (fd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    fcntl (fd, F_SETFL, flags);

    if (f == NULL)
    {  
        printf ("f is NULL!! \n"); 
        perror (out);
        return 0;
    }   

    printf ("f was correct\n");
    while (fgets(out, 256, f) != NULL) {
        printf ("Buffer: %s\n", out);
    }
    printf ("Out of the loop\n");
//    printf ("Bye\n");
    // get function name
   // fgets (out, 256, f); 

   // printf ("Buffer: %s\n", out);
    pclose(f);
    printf ("Bye\n");
    return 0;
    // get file and line
    fgets (out, 256, f); 
    printf ("Buffer: %s\n", out);
    printf ("Length: %d\n", strlen(out));

    if (out[0] != '?') {
        int l;
        int i = 0;
        char *p = out;

        // file name is until ':'
        while (*p != ':') {   
            p++;
        }

        *p++ = 0;
        // after file name follows line number
        strcpy (file , out);
        sscanf (p,"%d", line);
    }
    else {
        strcpy (file,"unkown");
        *line = 0;
    }
    pclose(f);
    return 1;
}

// Call this function to get a backtrace.
void capture_backtrace(char *executable, int function) {
    printf ("ENTERING BACKTRACE\n");
    char name[256];
    unw_cursor_t cursor; unw_context_t uc;
    unw_word_t ip, sp, offp;

    unw_getcontext(&uc);
    unw_init_local(&cursor, &uc);

    while (unw_step(&cursor) > 0)
    {
        char file[256];
        int line = 0;

        name[0] = '\0';
        unw_get_proc_name(&cursor, name, 256, &offp);
        printf ("Function name: %s\n", name);
        unw_get_reg(&cursor, UNW_REG_IP, &ip);
        unw_get_reg(&cursor, UNW_REG_SP, &sp);

        perfexpert_unwind_get_file_line ((long)ip, file, 256, &line, executable);
/*  
        //if (line==0)
        //    break;
        //if (strlen(file)<2)
        //    break;

        if (function ==FOPEN)
            printf("[fopen] %s in file %s line %d\n", name, file, line);
        if (function ==FREAD)
            printf("[fread] %s in file %s line %d\n", name, file, line);
        if (function ==FWRITE)
            printf("[fwrite] %s in file %s line %d\n", name, file, line);
*/
    }
    printf ("DONE WITH BACKTRACE\n");
}

/*
void init() __attribute__ ((constructor));
void init() {
    program = getenv("PERFEXPERT_PROGRAM");
    if (program == NULL) {
        //OUTPUT(("%s", _ERROR("Program has not been defined"));
        printf("%s \n", "Program has not been defined");
        return;
    }
    else {
//        printf ("The program that is going to run is %s\n", program);
    }
}
*/

/*  int open(const char *path, int oflag, ... ) { 
    va_list argp;
    va_start(argp, oflag);
    real_open=dlsym(RTLD_NEXT, "open");
    int ret = real_open=(path, oflag, argp);
    va_end(argp);
    return ret;
}
*/

/*    FILE* fopen(const char *path, const char *mode) {
    program = getenv("PERFEXPERT_PROGRAM");
    printf ("fopen: %s\n", program);
    capture_backtrace (program, FOPEN);
    printf ("That was the backtrace\n");
    real_fopen=dlsym(RTLD_NEXT, "fopen");
    printf ("Bye fopen\n");
    return real_fopen(path, mode);
}
*/


/*  ssize_t write(int fd, const void *buf, size_t count) {
    printf ("Writing %lu\n", count);
    capture_backtrace(program);
    real_write=dlsym(RTLD_NEXT, "write");
    return real_write(fd,buf,count);
}
*/

size_t fwrite(const void * ptr, size_t size, size_t n, FILE * s) {
    program = getenv("PERFEXPERT_PROGRAM");
    printf ("MY FWRITE\n");
    capture_backtrace(program, FWRITE);
    printf ("before system fwrite\n");
    real_fwrite=dlsym(RTLD_NEXT, "fwrite");
    printf ("after system fwrite\n");
    return real_fwrite(ptr, size, n, s);
}

/*  ssize_t read(int fd, void *buf, size_t count) {
    printf ("Reading %lu\n", count);
    capture_backtrace (program);
    real_read=dlsym(RTLD_NEXT, "read");
    return real_read(fd,buf,count);
}
*/

size_t fread(void *ptr, size_t size, size_t n, FILE * s) {
    program = getenv("PERFEXPERT_PROGRAM");
    printf ("fread\n");
    capture_backtrace (program, FREAD);
    real_fread=dlsym(RTLD_NEXT, "fread");
    return real_fread(ptr, size, n, s);
}

/*  int close(int fd) {
    printf ("Closing\n");
    capture_backtrace (program);
    real_close=dlsym(RTLD_NEXT, "close");
    return real_close(fd);
}
*/

/*int fclose(FILE *stream) {
    program = getenv("PERFEXPERT_PROGRAM");
    printf ("fclose\n");
    capture_backtrace (program);
    real_fclose=dlsym(RTLD_NEXT, "fclose");
    return real_fclose(stream);
}
*/

#ifdef __cplusplus
}
#endif

#endif /* PREFEXPERT_MODULE_IO_WRAPPER_H_ */
