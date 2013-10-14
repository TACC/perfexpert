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

#ifndef PERFEXPERT_ALLOC_H_
#define PERFEXPERT_ALLOC_H_

#ifndef _STDLIB_H
#include <stdlib.h>
#endif

#include "perfexpert_output.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PERFEXPERT_ALLOC(type, ptr, size)                       \
    ptr = (type *)malloc(size);                                 \
    if (NULL == ptr) {                                          \
        OUTPUT(("%s (%s:%d) (%s)",                              \
            _ERROR((char *)"Error: unable to allocate memory"), \
            __FILE__, __LINE__, __FUNCTION__));                 \
        exit(PERFEXPERT_ERROR);                                 \
    }                                                           \
    bzero(ptr, size)

#define PERFEXPERT_DEALLOC(ptr) \
    if (NULL != ptr) {          \
        free(ptr);              \
    }                           \
    ptr = NULL

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_ALLOC_H */
