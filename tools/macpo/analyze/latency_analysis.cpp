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
#include <iostream>
#include <map>

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
        size_t cache_line, short read_or_write, const int DIST_INFINITY) {
    int i;
    bool conflict = false;

    for (i=0; i<num_cores; i++) {
        size_t dist = tree_list[i]->get_distance(cache_line);
        if ((read_or_write == TYPE_WRITE ||
                read_or_write == TYPE_READ_AND_WRITE) && i != core_id &&
                dist >= 0 && dist < DIST_INFINITY) {
            conflict = true;

            tree_list[i]->make_infinite_distance(cache_line);

            if (create_histogram_if_null(hist_matrix[i][var_idx],
                        DIST_INFINITY) < 0) {
                break;
            }

            double existing_count = gsl_histogram_get(hist_matrix[i][var_idx],
                    dist);

            gsl_histogram_accumulate(hist_matrix[i][var_idx], dist,
                    -1.0 * existing_count);
            gsl_histogram_increment(hist_matrix[i][var_idx], DIST_INFINITY-1);
        }
    }

    return conflict;
}

static size_t calculate_distance(histogram_matrix_t& hist_matrix,
        avl_tree_list_t& tree_list, int num_cores, int core_id, size_t var_idx,
        size_t cache_line, short read_or_write, const int DIST_INFINITY) {
    if (conflict(hist_matrix, tree_list, num_cores, var_idx, core_id,
                cache_line, read_or_write, DIST_INFINITY)) {
        return DIST_INFINITY;
    } else {
        return tree_list[core_id]->get_distance(cache_line);
    }
}

int print_cache_conflicts(const global_data_t& global_data,
        double_list_t& conflict_list, bool bot) {
    std::cout << std::endl;

    if (bot == false) {
        std::cout << macpoprefix << "Cache conflicts:" << std::endl;

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
    } else {
        const int num_streams = global_data.stream_list.size();
        for (int i=0; i<num_streams; i++) {
            double conflict_percentage = 100.0 * conflict_list[i];

            if (conflict_percentage < 0)
                conflict_percentage = 0.0;

            if (conflict_percentage > 100)
                conflict_percentage = 100.0;

            std::cout << MSG_CACHE_CONFLICTS << "." <<
                global_data.stream_list[i] << "." <<
                MSG_CONFLICT_PERCENTAGE << "=" <<
                (double) conflict_percentage << std::endl;
        }
    }

    std::cout << std::endl;
    return 0;
}

int print_reuse_distances(const global_data_t& global_data,
        histogram_list_t& rd_list, const int DIST_INFINITY, bool bot) {
    std::cout << std::endl;

    if (bot == false) {
        std::cout << macpoprefix << "Reuse distances:" << std::endl;

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

                    if (max_bin != DIST_INFINITY - 1) {
                        if (max_val > 0) {
                            std::cout << " " << max_bin << " (" << max_val <<
                                " times)";
                        }
                    } else {
                        if (max_val > 0) {
                            std::cout << " inf. (" << max_val << " times)";
                        }
                    }
                }

                std::cout << "." << std::endl;
            }
        }
    } else {
        const int num_streams = global_data.stream_list.size();
        for (int i=0; i<num_streams; i++) {
            if(rd_list[i] != NULL) {
                pair_list_t pair_list;
                flatten_and_sort_histogram(rd_list[i], pair_list);

                size_t limit = std::min((size_t) REUSE_DISTANCE_COUNT,
                        pair_list.size());
                for (size_t j=0; j<limit; j++) {
                    size_t max_bin = pair_list[j].first;
                    size_t max_val = pair_list[j].second;

                    if (max_bin != DIST_INFINITY - 1) {
                        if (max_val > 0) {
                            std::cout << MSG_REUSE_DISTANCE << "." <<
                                global_data.stream_list[i] << "." <<
                                MSG_DISTANCE_VALUE << "[" << j << "]" << "=" <<
                                max_bin << std::endl;

                            std::cout << MSG_REUSE_DISTANCE << "." <<
                                global_data.stream_list[i] << "." <<
                                MSG_DISTANCE_COUNT << "[" << j << "]" << "=" <<
                                max_val << std::endl;
                        }
                    } else {
                        if (max_val > 0) {
                            std::cout << MSG_REUSE_DISTANCE << "." <<
                                global_data.stream_list[i] << "." <<
                                MSG_DISTANCE_VALUE << "[" << j << "]" << "=" <<
                                "inf" << std::endl;

                            std::cout << MSG_REUSE_DISTANCE << "." <<
                                global_data.stream_list[i] << "." <<
                                MSG_DISTANCE_COUNT << "[" << j << "]" << "=" <<
                                max_val << std::endl;
                        }
                    }
                }
            }
        }
    }

    return 0;
}

int latency_analysis(const global_data_t& global_data,
        histogram_list_t& rd_list, double_list_t& conflict_list,
        const int DIST_INFINITY) {
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
                const size_t cache_line = ADDR_TO_CACHE_LINE(mem_info.address);
                const short read_write = mem_info.read_write;

                // Quick validation check.
                if (core_id >= 0 && core_id < num_cores &&
                        var_idx >= 0 && var_idx < num_streams) {

                    avl_tree* tree = tree_list[core_id];
                    if (create_histogram_if_null(
                                histogram_matrix[core_id][var_idx],
                                DIST_INFINITY) < 0) {
                        goto end_iteration;
                    }

                    size_t distance = 0;
                    histogram_t* hist = histogram_matrix[core_id][var_idx];

                    distance = calculate_distance(histogram_matrix, tree_list,
                            num_cores, core_id, var_idx, cache_line, read_write,
                            DIST_INFINITY);

                    // Occupy the last bin in case of overflow.
                    if (distance >= DIST_INFINITY)
                        distance = DIST_INFINITY - 1;

                    gsl_histogram_increment(hist, distance);
                    tree->insert(&mem_info);

                    // Check if this cache line has been access earlier,
                    // and thus, if it has an owner.
                    if (cache_line_owner.find(cache_line) ==
                            cache_line_owner.end()) {
                        if (read_write == TYPE_WRITE ||
                                read_write == TYPE_READ_AND_WRITE) {
                            cache_line_owner[cache_line] = core_id;
                        }
                    } else {
                        // This cache line already has an owner.
                        if (read_write == TYPE_WRITE ||
                                read_write == TYPE_READ_AND_WRITE) {
                            // Chances of a conflict between owner and core_id.
                            short owner = cache_line_owner[cache_line];
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
            #pragma omp critical
            for (int j=0; j<num_cores; j++) {
                for (int k=0; k<num_streams; k++) {
                    histogram_t* h2 = histogram_matrix[j][k];

                    if (h2 != NULL) {
                        if (create_histogram_if_null(rd_list[k],
                                    DIST_INFINITY) == 0) {
                            gsl_histogram_add(rd_list[k], h2);
                        }
                    }
                }
            }

            // Sum up the histogram values into result histogram.
            #pragma omp critical
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
            conflict_list[j] = misses / (hits + misses);
        }
    }

    return 0;
}
