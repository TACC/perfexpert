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

#ifndef PERFEXPERT_MODULE_HPCTOOLKIT_PAPI_H_
#define PERFEXPERT_MODULE_HPCTOOLKIT_PAPI_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int end;
    int last;
    char events[MAX_BUFFER_SIZE];
} sample_t;

/* Function declarations */
int papi_max_events(void);
int papi_get_sampling_rate(const char *name);
static int get_prime(int start, int end);

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_MODULE_HPCTOOLKIT_PAPI_H_ */
