/*
 * Copyright (c) 2013  University of Texas at Austin. All rights reserved.
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

#ifndef PERFEXPERT_TIME_H_
#define PERFEXPERT_TIME_H_

#ifndef _TIME_H
#include <time.h>
#endif

#include "perfexpert_constants.h"

#ifdef __cplusplus
extern "C" {
#endif

/* perfexpert_time_diff */
static inline struct timespec *perfexpert_time_diff(struct timespec *diff,
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

#endif /* PERFEXPERT_TIME_H */
