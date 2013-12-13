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

#ifndef REUSE_DIST_ANALYSIS_H_
#define REUSE_DIST_ANALYSIS_H_

#include <gsl/gsl_histogram.h>
#include <iostream>

#include "avl_tree.h"

#define DIST_INFINITY   20  // FIXME: Use number of cache lines.

bool pair_sort(const pair_t& p1, const pair_t& p2);

int flatten_and_sort_histogram(gsl_histogram*& hist, pair_list_t& pair_list);

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

int print_reuse_distances(const global_data_t& global_data,
        histogram_list_t& rd_list);

int reuse_distance_analysis(const global_data_t& global_data,
        histogram_list_t& rd_list);

#endif  /* REUSE_DIST_ANALYSIS_H_ */
