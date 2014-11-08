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
 * Authors: Leonardo Fialho, Ashay Rane and Ankit Goyal
 *
 * $HEADER$
 */

#ifndef SET_CACHE_CONFLICT_ANALYSIS_H_ 
#define SET_CACHE_CONFLICT_ANALYSIS_H_ 

#include "analysis_defs.h"
#include "avl_tree.h"
#include "histogram.h"

// This method iterates through the trace and based on reuse distance
// (maintained using AVL tree), calculates the conflicts.
int set_cache_conflict_analysis(const global_data_t& global_data);

// this method returns the set number to which an address
// maps to in a cache
unsigned address_to_set(size_t address);

// this struct is used to store a coflict when we are
// 'almost' certain. There are no probabilities involved.
struct conflict_t {
    size_t set_number;
    size_t var_idx;

    conflict_t(size_t _s, size_t _v) :
        set_number(_s), var_idx(_v) {}
};

// this struct is used to store a conflict probability
struct conflict_prob_t {
    size_t set_number;
    size_t var_idx;
    double miss_probability;

    conflict_prob_t(size_t _s, size_t _v, double _mp) :
        set_number(_s), var_idx(_v), miss_probability(_mp) {}
};

typedef std::vector<conflict_t> conflict_list_t;
typedef std::vector<conflict_prob_t> conflict_prob_list_t;

#endif /* SET_CACHE_CONFLICT_ANALYSIS_H_ */
