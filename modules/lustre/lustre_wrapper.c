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

#ifndef PREFEXPERT_MODULE_LUSTRE_LIB_H_
#define PREFEXPERT_MODULE_LUSTRE_LIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#define TRACE_MSG fprintf(stderr, "TRACE at: %s() [%s:%d]\n", \
                        __FUNCTION__, __FILE__, __LINE__)

static ssize_t (*real_write)(int fd, const void *buf, size_t count) = NULL;
static size_t (*real_fwrite)(const void *ptr, size_t size, size_t n, FILE *s) = NULL;
static ssize_t (*real_read)(int fd, void *buf, size_t count) = NULL;
static size_t (*real_fread)(void * ptr, size_t size, size_t n, FILE* s) = NULL;
static int (*real_open)(const char *path, int oflag, ... ) = NULL;
static FILE* (*real_fopen)(const char *path, const char *mode) = NULL;
static int (*real_close)(int fd);
static int (*real_fclose)(FILE *stream) = NULL;

void handler(int sig) {
  void *array[10];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}

int open(const char *path, int oflag, ... ) { 
    va_list argp;
    va_start(argp, oflag);
    real_open=dlsym(RTLD_NEXT, "open");
    int ret = real_open=(path, oflag, argp);
    va_end(argp);
    return ret;
}

FILE* fopen(const char *path, const char *mode) {
    real_fopen=dlsym(RTLD_NEXT, "fopen");
    return real_fopen(path, mode);
}

ssize_t write(int fd, const void *buf, size_t count) {
    printf ("Writing %lu\n", count);
    TRACE_MSG;
    real_write=dlsym(RTLD_NEXT, "write");
    return real_write(fd,buf,count);
}

size_t fwrite(const void * ptr, size_t size, size_t n, FILE * s) {
    real_fwrite=dlsym(RTLD_NEXT, "fwrite");
    return real_fwrite(ptr, size, n, s);
}

ssize_t read(int fd, void *buf, size_t count) {
    printf ("Reading %lu\n", count);
    TRACE_MSG;
    real_read=dlsym(RTLD_NEXT, "read");
    return real_read(fd,buf,count);
}

size_t fread(void *ptr, size_t size, size_t n, FILE * s) {
    printf ("Fread\n");
    real_fread=dlsym(RTLD_NEXT, "fread");
    return real_fread(ptr, size, n, s);
}

int close(int fd) {
    printf ("Closing\n");
    TRACE_MSG;
    real_close=dlsym(RTLD_NEXT, "close");
    return real_close(fd);
}

int fclose(FILE *stream) {
    real_fclose=dlsym(RTLD_NEXT, "fclose");
    return real_fclose(stream);
}

#ifdef __cplusplus
}
#endif

#endif /* PREFEXPERT_MODULE_LUSTRE_LIB_H_ */
