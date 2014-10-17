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

#include <cassert>
#include <map>
#include <vector>

#include "analysis_defs.h"
#include "avl_tree.h"
#include "histogram.h"

#include "set_cache_conflict_analysis.h"
// TODO(goyalankit): remove hardcoded number of sets
#define ASSOCIATIVITY 8

#define BIT_MASK (((unsigned) -1 >> (31 - (11))) & ~((1U << (6)) - 1))

unsigned address_to_set(size_t address) {
    return ((address & BIT_MASK) >> 6);
}

void print_set_conflicts(std::vector<conflict_t> &conflicts) {
    int total_conflicts = 0;
    std::map<int, int> var_conflicts;
    std::map<int, int> set_conflicts;
    for (int i = 0; i < conflicts.size(); i++) {
        set_conflicts[conflicts[i].set_number]++;
        var_conflicts[conflicts[i].var_idx]++;
        total_conflicts++;
    }

    for (int j = 0; j < set_conflicts.size(); j++) {
        std::cout << j << " " << set_conflicts[j] << std::endl;
    }
}

int set_cache_conflict_analysis(const global_data_t& global_data) {
    const mem_info_bucket_t& bucket = global_data.mem_info_bucket;
    const int num_cores = sysconf(_SC_NPROCESSORS_CONF);
    cache_data_t l1_data = global_data.l1_data;
    l1_data.associativity = ASSOCIATIVITY;
    const int number_of_sets = l1_data.size/
        (l1_data.line_size  * l1_data.associativity);
    std::vector<conflict_t> set_conflicts;
    avl_tree_list_t set_avl_list(number_of_sets, new avl_tree());

    std::cout << "Getting set cache conflicts" << std::endl;

    assert(l1_data.size != 0 && l1_data.line_size != 0
            && l1_data.associativity !=0);

    for (int i = 0; i < bucket.size(); i++) {
        const mem_info_list_t& list = bucket.at(i);

        unsigned set_id = 0;
        int reuse_distance;
        for (int j = 0; j < list.size(); j++) {
            const mem_info_t& mem_info = list.at(j);
            const size_t var_idx = mem_info.var_idx;
            const size_t cache_line = ADDR_TO_CACHE_LINE(mem_info.address);
            set_id = address_to_set(mem_info.address);

            // if address is already seen => not a cold miss
            if (set_avl_list[set_id]->contains(cache_line)) {
                reuse_distance = set_avl_list[set_id]->get_distance(cache_line);
                if (reuse_distance > l1_data.associativity) {
                    conflict_t conflict(set_id, var_idx);
                    set_conflicts.push_back(conflict);
                }
            }
            set_avl_list[set_id]->insert(&mem_info);
        }
    }

    print_set_conflicts(set_conflicts);

    return 0;
}
