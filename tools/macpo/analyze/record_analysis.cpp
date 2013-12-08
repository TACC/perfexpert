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

#include <set>
#include <gsl/gsl_histogram.h>

#include "analysis_defs.h"
#include "avl_tree.h"
#include "err_codes.h"
#include "record_analysis.h"

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

        // Read the top CUT (typically, 80%) samples.
        bool change = true;
        std::set<size_t> good_line_numbers;
        size_t count = 0, good_sample_count = list.size() * CUT;
        while (count <= good_sample_count && change) {
            change = false;
            int max_val = gsl_histogram_max_val(hist);
            size_t max_bin = gsl_histogram_max_bin(hist);

            if (max_val > 0) {
                count += max_val;
                good_line_numbers.insert(max_bin);
                change = true;
            }
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
    // Loop over all buckets.
    // Process them in parallel.
    const mem_info_bucket_t& bucket = global_data.mem_info_bucket;

    {
        // Declare thread-local storage.
        for (int i=0; i<bucket.size(); i++) {
            const mem_info_list_t& list = bucket.at(i);

            avl_tree tree;
            // Create a new histogram.
            gsl_histogram *hist = gsl_histogram_alloc (20);
            if (hist == NULL) {
                return -ERR_NO_MEM;
            }

            gsl_histogram_set_ranges_uniform (hist, 0, 20);
            for (int j=0; j<list.size(); j++) {
                const mem_info_t& mem_info = list.at(j);
                if (tree.contains(mem_info.address)) {
                    size_t distance = tree.get_distance(mem_info.address);
                    // TODO: Add distance to histogram.
                }

                tree.insert(&(list.at(j)));
            }
        }
    }

    return 0;
}
