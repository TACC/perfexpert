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

typedef gsl_histogram histogram_t;

typedef std::vector<mem_info_list_t> mem_info_bucket_t;

typedef std::vector<trace_info_list_t> trace_info_bucket_t;

typedef std::vector<histogram_t*> histogram_list_t;
typedef std::vector<histogram_list_t> histogram_matrix_t;

typedef std::vector<avl_tree*> avl_tree_list_t;

typedef std::pair<size_t, size_t> pair_t;
typedef std::vector<pair_t> pair_list_t;

typedef std::vector<double> double_list_t;
typedef std::vector<int> int_list_t;

typedef struct {
    size_t size, line_size, associativity, count;
} cache_data_t;

typedef struct {
    cache_data_t l1_data, l2_data, l3_data;
    name_list_t stream_list;
    mem_info_bucket_t mem_info_bucket;
    trace_info_bucket_t trace_info_bucket;
} global_data_t;

typedef struct {
    size_t count, l1_conflicts, l2_conflicts;
} stream_list_t;

/* Different kinds of analyses options. */
#define ANALYSIS_REUSE_DISTANCE     (1 << 0)
#define ANALYSIS_CACHE_CONFLICTS    (1 << 1)
#define ANALYSIS_PREFETCH_STREAMS   (1 << 2)
#define ANALYSIS_STRIDES            (1 << 3)

#define ANALYSIS_ALL                (~0)

/* Number of reuse distance entries to display for each stream. */
#define REUSE_DISTANCE_COUNT        3

#endif /* ANALYSIS_DEFS_H_ */
