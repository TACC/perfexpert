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
#include "latency_analysis.h"

static void free_counters(histogram_matrix_t& hist_matrix,
        avl_tree_list_t& tree_list, int num_cores, int num_streams) {
    for (int i=0; i<num_cores; i++) {
        delete tree_list[i];

        for (int j=0; j<num_streams; j++) {
            if (hist_matrix[i][j] != NULL) {
                gsl_histogram_free(hist_matrix[i][j]);
            }
        }
    }
}

static bool init_counters(histogram_matrix_t& hist_matrix,
        avl_tree_list_t& tree_list, int num_cores, int num_streams) {
    int j;

    for (j=0; j<num_cores; j++) {
        tree_list[j] = new avl_tree();
        hist_matrix[j].resize(num_streams);

        if (tree_list[j] == NULL)
            break;

        for (int k=0; k<num_streams; k++) {
            hist_matrix[j][k] = NULL;
        }
    }

    // Clean up if we couldn't allocate memory.
    if (j != num_cores) {
        for (j -= 1; j >= 0; j--) {
            delete tree_list[j];
        }

        // Return failure.
        return false;
    }

    return true;
}

static bool conflict(histogram_matrix_t& hist_matrix,
        avl_tree_list_t& tree_list, int num_cores, int var_idx, int core_id,
        size_t address) {
    int i;
    bool conflict = false;

    for (i=0; i<num_cores; i++) {
        size_t dist = tree_list[i]->get_distance(address);
        if (i != core_id && dist >= 0 && dist < DIST_INFINITY) {
            conflict = true;

            tree_list[i]->set_distance(address, DIST_INFINITY);

            if (create_histogram_if_null(hist_matrix[i][var_idx],
                        DIST_INFINITY) < 0) {
                break;
            }

            gsl_histogram_accumulate(hist_matrix[i][var_idx], dist, -1.0);
            gsl_histogram_increment(hist_matrix[i][var_idx], DIST_INFINITY);
        }
    }

    return conflict;
}

static size_t calculate_distance(histogram_matrix_t& hist_matrix,
        avl_tree_list_t& tree_list, int num_cores, int core_id, size_t var_idx,
        size_t address) {
    if (tree_list[core_id]->contains(address)) {
        if (conflict(hist_matrix, tree_list, num_cores, var_idx, core_id,
                    address)) {
            return DIST_INFINITY;
        } else {
            return tree_list[core_id]->get_distance(address);
        }
    } else {
        return DIST_INFINITY;
    }
}

int print_cache_conflicts(const global_data_t& global_data,
        double_list_t& conflict_list) {
    std::cout << std::endl << mprefix << "Cache conflicts:" << std::endl;

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

int print_reuse_distances(const global_data_t& global_data,
        histogram_list_t& rd_list) {
    std::cout << std::endl << mprefix << "Reuse distances:" << std::endl;

    const int num_streams = global_data.stream_list.size();
    for (int i=0; i<num_streams; i++) {
        if(rd_list[i] != NULL) {
            std::cout << "var: " << global_data.stream_list[i] << ":";

            pair_list_t pair_list;
            flatten_and_sort_histogram(rd_list[i], pair_list);

            size_t limit = std::min((size_t) REUSE_DISTANCE_COUNT,
                    pair_list.size());
            for (size_t j=0; j<limit; j++) {
                size_t max_bin = pair_list[j].first;
                size_t max_val = pair_list[j].second;

                if (max_val > 0)
                    std::cout << " " << max_bin << " (" << max_val << " times)";
            }

            std::cout << "." << std::endl;
        }
    }

    return 0;
}

int latency_analysis(const global_data_t& global_data,
        histogram_list_t& rd_list, double_list_t& conflict_list) {
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

        int_list_t local_hit_list;
        int_list_t local_miss_list;

        local_hit_list.resize(num_streams);
        local_miss_list.resize(num_streams);

        for (int j=0; j<num_streams; j++) {
            local_hit_list[j] = 0;
            local_miss_list[j] = 0;
        }

        avl_tree_list_t tree_list;
        std::map<size_t, short> cache_line_owner;

        histogram_matrix_t histogram_matrix;
        histogram_matrix.resize(num_cores);
        tree_list.resize(num_cores);

        bool valid_allocation = init_counters(histogram_matrix, tree_list,
                num_cores, num_streams);

        if (valid_allocation) {
            for (int j=0; j<list.size(); j++) {
                bool valid_data = true;
                const mem_info_t& mem_info = list.at(j);

                const unsigned short core_id = mem_info.coreID;
                const size_t var_idx = mem_info.var_idx;
                const size_t address = mem_info.address;
                const short read_write = mem_info.read_write;

                // Quick validation check.
                if (core_id >= 0 && core_id < num_cores ||
                        var_idx >= 0 && var_idx < num_streams) {

                    size_t distance = calculate_distance(histogram_matrix,
                            tree_list, num_cores, core_id, var_idx, address);

                    if (create_histogram_if_null(
                                histogram_matrix[core_id][var_idx],
                                DIST_INFINITY) < 0) {
                        goto end_iteration;
                    }

                    // Occupy the last bin in case of overflow.
                    if (distance >= DIST_INFINITY)
                        distance = DIST_INFINITY - 1;

                    gsl_histogram_increment(histogram_matrix[core_id][var_idx],
                            distance);

                    avl_tree* tree = tree_list[core_id];
                    tree->insert(&mem_info);

                    // Check if this cache line has been access earlier,
                    // and thus, if it has an owner.
                    if (cache_line_owner.find(address) ==
                            cache_line_owner.end()) {
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

            // Sum up the histogram values from all cores.
            #pragma omp single
            for (int j=0; j<num_cores; j++) {
                for (int k=0; k<num_streams; k++) {
                    gsl_histogram* h2 = histogram_matrix[j][k];

                    if (h2 != NULL) {
                        if (create_histogram_if_null(rd_list[k],
                                    DIST_INFINITY) == 0) {
                            gsl_histogram_add(rd_list[k], h2);
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

end_iteration:
            free_counters(histogram_matrix, tree_list, num_cores, num_streams);
        } else {    /* valid_allocation */
            // Do nothing because init_counters takes care of freeing memory
            // if all required memory could not be allocated.
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
