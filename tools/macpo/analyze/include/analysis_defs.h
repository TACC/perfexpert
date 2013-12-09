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

#ifndef ANALYSIS_DEFS_H_
#define ANALYSIS_DEFS_H_

#include <vector>
#include <gsl/gsl_histogram.h>

#include "avl_tree.h"
#include "generic_defs.h"
#include "macpo_record.h"
#include "macpo_record_cxx.h"

// Holds a list of lists of mem_info_t type.
typedef std::vector<mem_info_list_t> mem_info_bucket_t;

typedef std::vector<gsl_histogram*> histogram_list_t;
typedef std::vector<histogram_list_t> histogram_matrix_t;

typedef std::vector<avl_tree*> avl_tree_list_t;

typedef struct {
    size_t size, lines;
} cache_data_t;

typedef struct {
    cache_data_t l1_data, l2_data;
    name_list_t stream_list;
    mem_info_bucket_t mem_info_bucket;
} global_data_t;

typedef struct {
    size_t count, l1_conflicts, l2_conflicts;
} stream_list_t;

#endif /* ANALYSIS_DEFS_H_ */
