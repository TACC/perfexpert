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

#include <time.h>

#include "common/perfexpert_constants.h"
#include "common/perfexpert_time.h"

#ifdef __cplusplus
extern "C" {
#endif

/* perfexpert_time_diff */
struct timespec *perfexpert_time_diff(struct timespec *diff,
    struct timespec *start, struct timespec *end) {
    if (0 > (end->tv_nsec - start->tv_nsec)) {
        diff->tv_sec  = end->tv_sec - start->tv_sec - 1;
        diff->tv_nsec = 1000000000 + end->tv_nsec - start->tv_nsec;
    } else {
        diff->tv_sec  = end->tv_sec - start->tv_sec;
        diff->tv_nsec = end->tv_nsec - start->tv_nsec;
    }
    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif
