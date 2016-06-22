/*
 * Copyright (c) 2011-2016  University of Texas at Austin. All rights reserved.
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

#ifndef PERFEXPERT_MODULE_TIMB_DATABASE_H_
#define PERFEXPERT_MODULE_TIMB_DATABASE_H_

#ifdef __cplusplus
extern "C" {
#endif

/* PerfExpert common headers */
#include "common/perfexpert_list.h"

/* Function declarations */
int init_timb_results();
int store_timb_result(int mpi_task, int thread, double load);
int get_mpi_ranks_LCPI();
int get_threads_LCPI();
long long int get_instructions_rank(int rank);
long long int get_instructions_thread(int rank, int thread);

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_MODULE_MACVEC_DATABASE_H_ */
