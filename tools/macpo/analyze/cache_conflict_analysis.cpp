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

#include <iostream>

#include "analysis_defs.h"
#include "avl_tree.h"
#include "histogram.h"
#include "reuse_dist_analysis.h"

int cache_conflict_analysis(const global_data_t& global_data,
        double_list_t& conflict_list) {
    const mem_info_bucket_t& bucket = global_data.mem_info_bucket;
    const int num_cores = sysconf(_SC_NPROCESSORS_CONF);
    const int num_streams = global_data.stream_list.size();

    int_list_t hit_list;
    int_list_t miss_list;

    hit_list.resize(num_streams);
    miss_list.resize(num_streams);

    for (int j=0; j<num_streams; j++) {
        hit_list[j] = 0;
        miss_list[j] = 0;
    }

    #pragma omp parallel for
    for (int i=0; i<bucket.size(); i++) {
        const mem_info_list_t& list = bucket.at(i);

        avl_tree tree;

        int_list_t local_hit_list;
        int_list_t local_miss_list;

        local_hit_list.resize(num_streams);
        local_miss_list.resize(num_streams);

        for (int j=0; j<num_streams; j++) {
            local_hit_list[j] = 0;
            local_miss_list[j] = 0;
        }

        std::map<size_t, short> cache_line_owner;

        for (int j=0; j<list.size(); j++) {
            const mem_info_t& mem_info = list.at(j);

            const unsigned short core_id = mem_info.coreID;
            const size_t var_idx = mem_info.var_idx;
            const size_t address = mem_info.address;
            const short read_write = mem_info.read_write;

            // Quick validation check.
            if (core_id >= 0 && core_id < num_cores ||
                    var_idx >= 0 && var_idx < num_streams) {
                // Check if this cache line has been access earlier,
                // and thus, if it has an owner.
                if (cache_line_owner.find(address) == cache_line_owner.end()) {
                    if (read_write == TYPE_WRITE) {
                        cache_line_owner[address] = core_id;
                        local_hit_list[var_idx] += 1;
                    }
                } else {
                    // This cache line already has an owner.
                    if (read_write == TYPE_WRITE) {
                        // Chances of a conflict between owner and core_id.
                        short owner = cache_line_owner[address];
                        if (owner == core_id) {
                            local_hit_list[var_idx] += 1;
                        } else {
                            local_miss_list[var_idx] += 1;
                        }
                    } else {
                        // This cache line is only being read,
                        // no conflicts here.
                        local_hit_list[var_idx] += 1;
                    }
                }
            }
        }

        // Sum up the histogram values into result histogram.
        #pragma omp single
        for (int j=0; j<num_streams; j++) {
            hit_list[j] += local_hit_list[j];
            miss_list[j] += local_miss_list[j];
        }
    }

    for (int j=0; j<num_streams; j++) {
        double hits = hit_list[j];
        double misses = miss_list[j];

        // Approximate answers are fine.
        if (hits + misses > 0) {
            conflict_list[j] = misses + (hits + misses);
        }
    }

    return 0;
}

int print_cache_conflicts(const global_data_t& global_data,
        double_list_t& conflict_list) {
    const int num_streams = global_data.stream_list.size();
    for (int i=0; i<num_streams; i++) {
        double conflict_percentage = 100.0 * conflict_list[i];

        if (conflict_percentage < 0)
            conflict_percentage = 0.0;

        if (conflict_percentage > 100)
            conflict_percentage = 100.0;

        std::cout << "var: " << global_data.stream_list[i] <<
                ", conflict ratio: " << (double) conflict_percentage << "%." <<
                std::endl;
    }

    std::cout << std::endl;
    return 0;
}
