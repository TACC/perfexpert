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
// TODO(goyalankit): remove hardcoded associativity
#define ASSOCIATIVITY 8

#define BIT_MASK (((unsigned) -1 >> (31 - (11))) & ~((1U << (6)) - 1))

unsigned address_to_set(size_t address) {
    return ((address & BIT_MASK) >> 6);
}

void print_set_conflicts(std::vector<conflict_list_t> &conflicts, int num_sets) {
    int total_conflicts = 0;
    std::map< int, std::map<int, int>* > core_set_var_conf;
    for (int core_id = 0; core_id < conflicts.size(); core_id++) {
        core_set_var_conf[core_id] = new std::map<int, int>();
        for (int n = 0; n < conflicts[core_id].size(); n++) {
            conflict_t ct = conflicts[core_id][n];
            total_conflicts++;
            (*core_set_var_conf[core_id])[ct.set_number]++;
        }
    }

    std::cout << "Total Conflicts: " << total_conflicts << std::endl;

    for (int i = 0; i < core_set_var_conf.size(); ++i) {
        if (core_set_var_conf[i]->size() != 0) {
            std::cout << "Conflicts for core number: " << i << std::endl;
            std::cout << "set_num" << " " << "num_conflicts" << std::endl;
        }

        for (std::map<int, int>::iterator it = core_set_var_conf[i]->begin();
                it != core_set_var_conf[i]->end(); ++it) {
            std::cout << it->first << " " << it->second << std::endl;
        }
    }
}

int set_cache_conflict_analysis(const global_data_t& global_data) {
    const mem_info_bucket_t& bucket = global_data.mem_info_bucket;
    const int num_cores = sysconf(_SC_NPROCESSORS_CONF);
    const int num_streams = global_data.stream_list.size();
    cache_data_t l1_data = global_data.l1_data;
    l1_data.associativity = ASSOCIATIVITY;
    const int num_sets = l1_data.size/
        (l1_data.line_size  * l1_data.associativity);
    std::vector< conflict_list_t > set_conflicts_per_core(num_cores);
    // list of per set avl trees for each core
    std::vector<avl_tree_list_t *> avl_trees_per_core(num_cores,
            new avl_tree_list_t(num_sets, new avl_tree()));

    std::cout << "Getting set cache conflicts" << std::endl;

    assert(l1_data.size != 0 && l1_data.line_size != 0
            && l1_data.associativity !=0);

    for (int i = 0; i < bucket.size(); i++) {
        const mem_info_list_t& list = bucket.at(i);

        unsigned set_id = 0;
        int reuse_distance;
        size_t core_id;
        size_t var_idx, cache_line;
        for (int j = 0; j < list.size(); j++) {
            const mem_info_t& mem_info = list.at(j);
            var_idx = mem_info.var_idx;
            cache_line = ADDR_TO_CACHE_LINE(mem_info.address);
            set_id = address_to_set(mem_info.address);
            core_id = mem_info.coreID;
            avl_tree_list_t &set_avl_list = (*avl_trees_per_core[core_id]);
            std::vector<conflict_t> &set_conflicts =
                set_conflicts_per_core[core_id];

            // Quick validation check
            if (core_id >= 0 && core_id < num_cores ||
                    var_idx >= 0 && var_idx < num_streams) {
                // if address is already seen => not a cold miss
                if (set_avl_list[set_id]->contains(cache_line)) {
                    reuse_distance = set_avl_list[set_id]->
                        get_distance(cache_line);
                    if (reuse_distance > l1_data.associativity) {
                        conflict_t conflict(set_id, var_idx);
                        set_conflicts.push_back(conflict);
                    }
                }
                set_avl_list[set_id]->insert(&mem_info);
            }
        }
    }

    print_set_conflicts(set_conflicts_per_core, num_sets);

    return 0;
}
