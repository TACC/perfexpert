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

#include <cmath>
#include <iostream>
#include <set>

#include "analysis_defs.h"
#include "err_codes.h"
#include "record_analysis.h"

#include "latency_analysis.h"
#include "stride_analysis.h"
#include "vector_stride_analysis.h"

int filter_low_freq_records(global_data_t& global_data) {
    mem_info_bucket_t& bucket = global_data.mem_info_bucket;

    for (int i=0; i<bucket.size(); i++) {
        mem_info_list_t& list = bucket.at(i);

        // Create a new histogram.
        gsl_histogram *hist = gsl_histogram_alloc (LINE_NUM_LIMIT);
        if (hist == NULL) {
            return -ERR_NO_MEM;
        }

        gsl_histogram_set_ranges_uniform (hist, 0, LINE_NUM_LIMIT);

        // Shove it into the histogram.
        for (int j=0; j<list.size(); j++) {
            mem_info_t& mem_info = list.at(j);
            gsl_histogram_increment(hist, mem_info.line_number);
        }

        pair_list_t pair_list;
        std::set<size_t> good_line_numbers;

        flatten_and_sort_histogram(hist, pair_list);

        size_t k = list.size() * CUT;
        size_t limit = std::min(k, gsl_histogram_bins(hist));

        for (int j=0; j<limit; j++) {
            size_t max_bin = pair_list[j].first;
            size_t max_val = pair_list[j].second;

            if (max_val > 0)
                good_line_numbers.insert(max_bin);
        }

        // We don't need the histogram any more.
        gsl_histogram_free(hist);

        // Discard all other samples.
        mem_info_list_t::iterator it = list.begin();
        while (it != list.end()) {
            const mem_info_t& mem_info = *it;
            if (good_line_numbers.find(mem_info.line_number) ==
                    good_line_numbers.end()) {
                it = list.erase(it);
            } else {
                it++;
            }
        }
    }

    return 0;
}

int analyze_records(const global_data_t& global_data, int analysis_flags) {
    int code = 0;
    const int num_streams = global_data.stream_list.size();

    if (analysis_flags & (ANALYSIS_CACHE_CONFLICTS | ANALYSIS_REUSE_DISTANCE)) {
        std::cout << mprefix << "Analyzing records for latency." << std::endl;

        histogram_list_t rd_list;
        rd_list.resize(num_streams);

        double_list_t conflict_list;
        conflict_list.resize(num_streams);

        int DIST_INFINITY;
        if (global_data.l3_data.size != 0) {
            const cache_data_t& l3_data = global_data.l3_data;
            DIST_INFINITY = ceil(((double) l3_data.size) / l3_data.line_size);
        } else if (global_data.l2_data.size != 0) {
            const cache_data_t& l2_data = global_data.l2_data;
            DIST_INFINITY = ceil(((double) l2_data.size) / l2_data.line_size);
        } else if (global_data.l1_data.size != 0) {
            const cache_data_t& l1_data = global_data.l1_data;
            DIST_INFINITY = ceil(((double) l1_data.size) / l1_data.line_size);
        } else {
            return -ERR_INV_CACHE;
        }

        if ((code = latency_analysis(global_data, rd_list, conflict_list,
                        DIST_INFINITY)) < 0)
            return code;

        print_reuse_distances(global_data, rd_list, DIST_INFINITY);
        print_cache_conflicts(global_data, conflict_list);
    }

    if (analysis_flags & ANALYSIS_STRIDES) {
        std::cout << mprefix << "Analyzing records for stride values." <<
                std::endl;

        histogram_list_t stride_list;
        stride_list.resize(num_streams);

        if ((code = stride_analysis(global_data, stride_list)) < 0)
            return code;

        print_strides(global_data, stride_list);
    }

    if (analysis_flags & ANALYSIS_VECTOR_STRIDES) {
        std::cout << mprefix << "Analyzing records for vector stride values." <<
                std::endl;

        histogram_list_t stride_list;
        stride_list.resize(num_streams);

        if ((code = vector_stride_analysis(global_data, stride_list)) < 0)
            return code;

        print_vector_strides(global_data, stride_list);
    }

    return 0;
}
