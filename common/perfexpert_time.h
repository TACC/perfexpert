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

#ifndef PERFEXPERT_TIME_H_
#define PERFEXPERT_TIME_H_

#ifdef __cplusplus
extern "C" {
#endif

/* perfexpert_time_diff */
struct timespec *perfexpert_time_diff(struct timespec *diff,
    struct timespec *start, struct timespec *end);

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_TIME_H */
