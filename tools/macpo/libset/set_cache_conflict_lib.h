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

#ifndef SET_CACHE_CONFLICT_LIB_H_ 
#define SET_CACHE_CONFLICT_LIB_H_ 

#if defined(__cplusplus)
extern "C" {
    void indigo__process_c(int read_write, int line_number, void* addr,
                    int var_idx, int type_size);

    void indigo__process_f_(int *read_write, int *line_number, void* addr,
                    int *var_idx, int* type_size);

void indigo_set_cache_init();
void indigo__end();
};
#endif

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

struct cache_stats_t {
    uint64_t addr_accesses;
    uint64_t addr_hits;
    uint64_t addr_conflict_misses;
    uint64_t addr_cold_misses;
    uint64_t addr_capacity_misses;

    cache_stats_t() {
        addr_accesses = 0;
        addr_conflict_misses = 0;
        addr_hits = 0;
        addr_cold_misses = 0;
        addr_capacity_misses = 0;
    }
};


/*
 * Static variables
 *
 * */

int num_cores; // total number of cores on the current system

// estoy en libset

#endif /* SET_CACHE_CONFLICT_ANALYSIS_H_ */
