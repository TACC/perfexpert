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

#ifndef LATENCY_ANALYSIS_H_
#define LATENCY_ANALYSIS_H_

#include "analysis_defs.h"
#include "avl_tree.h"
#include "histogram.h"

#define DIST_INFINITY   20  // TODO: Get from cache sizes.

static void free_counters(histogram_matrix_t& hist_matrix,
        avl_tree_list_t& tree_list, int num_cores, int num_streams);

static bool init_counters(histogram_matrix_t& hist_matrix,
        avl_tree_list_t& tree_list, int num_cores, int num_streams);

static bool conflict(histogram_matrix_t& hist_matrix,
        avl_tree_list_t& tree_list, int num_cores, int var_idx, int core_id,
        size_t address);

static size_t calculate_distance(histogram_matrix_t& hist_matrix,
        avl_tree_list_t& tree_list, int num_cores, int core_id, size_t var_idx,
        size_t address);

int print_cache_conflicts(const global_data_t& global_data,
        double_list_t& conflict_list);

int print_reuse_distances(const global_data_t& global_data,
        histogram_list_t& rd_list);

int latency_analysis(const global_data_t& global_data,
        histogram_list_t& rd_list, double_list_t& conflict_list);

#endif  /* LATENCY_ANALYSIS_H_ */
