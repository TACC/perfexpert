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
 * Authors: Antonio Gomez-Iglesias, Leonardo Fialho and Ashay Rane
 *
 * $HEADER$
 */

#ifndef PERFEXPERT_ALLOC_H_
#define PERFEXPERT_ALLOC_H_

#ifndef _STDLIB_H
#include <stdlib.h>
#endif

#include <sys/types.h>
#include "common/perfexpert_output.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PERFEXPERT_ALLOC(type, ptr, size)                            \
    ptr = (type *)malloc(size);                                      \
    if (NULL == ptr) {                                               \
        OUTPUT(("%s", _ERROR((char *)"unable to allocate memory"))); \
        exit(PERFEXPERT_ERROR);                                      \
    }                                                                \
    perfexpert_alloc_add(ptr, size);                                 \
    bzero(ptr, size)

#define PERFEXPERT_REALLOC(type, ptr, size)                            \
    ptr = (type *)realloc(ptr, size);                                  \
    if (NULL == ptr) {                                                 \
        OUTPUT(("%s", _ERROR((char *)"unable to reallocate memory"))); \
        exit(PERFEXPERT_ERROR);                                        \
    }                                                                  \
    perfexpert_alloc_add(ptr, size);                                   \
    bzero(ptr, size)

#define PERFEXPERT_DEALLOC(ptr)    \
    if (NULL != ptr) {             \
        free(ptr);                 \
        perfexpert_alloc_del(ptr); \
    }                              \
    ptr = NULL

/* Accounting functions */
void perfexpert_alloc_add(void *ptr, ssize_t size);
void perfexpert_alloc_del(void *ptr);
void perfexpert_alloc_free_all(void);

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_ALLOC_H */
