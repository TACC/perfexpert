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

#ifndef PREFEXPERT_MODULE_LUSTRE_WRAPPER_H_
#define PREFEXPERT_MODULE_LUSTRE_WRAPPER_H_

#ifdef __cplusplus
extern "C" {
#endif

#define _GNU_SOURCE
#include <stddef.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <stdio.h>

#include <signal.h>
#include <unistd.h>

#include "lustre.h"
#include "perfexpert_unwind.h"


static ssize_t (*real_write)(int fd, const void *buf, size_t count) = NULL;
static ssize_t (*real_fwrite)(const void *ptr, size_t size, size_t n, FILE *s) = NULL;
static ssize_t (*real_read)(int fd, void *buf, size_t count) = NULL;
static ssize_t (*real_fread)(void * ptr, size_t size, size_t n, FILE* s) = NULL;
static int (*real_open)(const char *path, int oflag, ... ) = NULL;
static FILE* (*real_fopen)(const char *path, const char *mode) = NULL;
static int (*real_close)(int fd);
static int (*real_fclose)(FILE *stream) = NULL;

int open(const char *path, int oflag, ... ) { 
    va_list argp;
    va_start(argp, oflag);
    real_open=dlsym(RTLD_NEXT, "open");
    int ret = real_open=(path, oflag, argp);
    va_end(argp);
    return ret;
}

FILE* fopen(const char *path, const char *mode) {
    capture_backtrace (globals.program);
    real_fopen=dlsym(RTLD_NEXT, "fopen");
    return real_fopen(path, mode);
}

ssize_t write(int fd, const void *buf, size_t count) {
    printf ("Writing %lu\n", count);
    capture_backtrace(globals.program);
    real_write=dlsym(RTLD_NEXT, "write");
    return real_write(fd,buf,count);
}

size_t fwrite(const void * ptr, size_t size, size_t n, FILE * s) {
    capture_backtrace(globals.program);
    real_fwrite=dlsym(RTLD_NEXT, "fwrite");
    return real_fwrite(ptr, size, n, s);
}

ssize_t read(int fd, void *buf, size_t count) {
    printf ("Reading %lu\n", count);
    capture_backtrace (globals.program);
    real_read=dlsym(RTLD_NEXT, "read");
    return real_read(fd,buf,count);
}

size_t fread(void *ptr, size_t size, size_t n, FILE * s) {
    printf ("Fread\n");
    capture_backtrace (globals.program);
    real_fread=dlsym(RTLD_NEXT, "fread");
    return real_fread(ptr, size, n, s);
}

int close(int fd) {
    printf ("Closing\n");
    capture_backtrace (globals.program);
    real_close=dlsym(RTLD_NEXT, "close");
    return real_close(fd);
}

int fclose(FILE *stream) {
    capture_backtrace (globals.program);
    real_fclose=dlsym(RTLD_NEXT, "fclose");
    return real_fclose(stream);
}

#ifdef __cplusplus
}
#endif

#endif /* PREFEXPERT_MODULE_LUSTRE_WRAPPER_H_ */
