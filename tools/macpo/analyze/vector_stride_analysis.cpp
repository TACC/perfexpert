/*
 * Copyright (c) 2011-2015  University of Texas at Austin. All rights reserved.
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
 * Authors: Antonio Gomez-Iglesias, Leonardo Fialho and Ashay Rane
 *
 * $HEADER$
 */

#include <iostream>
#include <map>

#include "histogram.h"
#include "vector_stride_analysis.h"

int vector_stride_analysis(const global_data_t& global_data,
        histogram_list_t& stride_list) {
    const vector_stride_info_bucket_t& bucket = global_data.vector_stride_info_bucket;
    const int num_cores = sysconf(_SC_NPROCESSORS_CONF);
    const int num_streams = global_data.stream_list.size();

    #pragma omp parallel for
    for (int i=0; i<bucket.size(); i++) {
        const vector_stride_info_list_t& list = bucket.at(i);

        std::map<size_t, size_t> last_addr;
        histogram_list_t local_stride_list;
        local_stride_list.resize(num_streams);

        for (int j=0; j<list.size(); j++) {
            const vector_stride_info_t& vector_stride_info = list.at(j);

            const unsigned short core_id = vector_stride_info.coreID;
            const size_t loop_line_number = vector_stride_info.loop_line_number;
            const size_t var_idx = vector_stride_info.var_idx;
            const size_t address = vector_stride_info.address;
            const int type_size = vector_stride_info.type_size == 0 ? 1 :
                    vector_stride_info.type_size;

            // Quick validation check.
            if (core_id >= 0 && core_id < num_cores &&
                    var_idx >= 0 && var_idx < num_streams) {
                // Check if this address was accessed in the past.
                if (last_addr.find(var_idx) != last_addr.end()) {
                    size_t last_address = last_addr[var_idx];
                    size_t stride = (address - last_address) / type_size;

                    if (create_histogram_if_null(local_stride_list[var_idx],
                                MAX_STRIDE) == 0) {

                        // Occupy the last bin in case of overflow.
                        if (stride >= MAX_STRIDE)
                            stride = MAX_STRIDE - 1;

                        gsl_histogram_increment(local_stride_list[var_idx],
                                stride);
                    }
                }

                // Add this address as the last-seen address.
                last_addr[var_idx] = address;
            }
        }

        // Sum up the histogram values into result histogram.
        #pragma omp critical
        for (int j=0; j<num_streams; j++) {
            if (local_stride_list[j] != NULL) {
                if (create_histogram_if_null(stride_list[j], MAX_STRIDE) == 0) {
                    gsl_histogram_add(stride_list[j], local_stride_list[j]);
                }
            }
        }
    }

    return 0;
}

int print_vector_strides(const global_data_t& global_data,
        histogram_list_t& stride_list) {
    const int num_streams = global_data.stream_list.size();
    for (int i=0; i<num_streams; i++) {
        if(stride_list[i] != NULL) {
            pair_list_t pair_list;
            flatten_and_sort_histogram(stride_list[i], pair_list);

            size_t limit = std::min((size_t) STRIDE_COUNT, pair_list.size());
            bool unit_stride = true;
            bool unroll = false;
            for (size_t j=0; j<limit; j++) {
                size_t max_bin = pair_list[j].first;
                size_t max_val = pair_list[j].second;

                // If non-unit stride was observed at least once,
                // then display gather/scatter message.
                if (max_bin > 1 && max_val > 0) {
                    unit_stride = false;

                    // Should we recommend opt-gather-scatter-unroll?
                    if (max_bin > 4) {
                        unroll = true;
                    }

                    break;
                }
            }

            if (unit_stride == false) {
                std::cout << "gather/scatter operations required for: " <<
                    global_data.stream_list[i] << ",\ttry converting " <<
                    "array-of-structs to struct-of-arrays." << std::endl;

                if (unroll == true) {
                    std::cout << "try using -opt-gather-scatter-unroll=<N> " <<
                        "and -mP2OPT_hlo_pref_indirect_refs=T if compiling " <<
                        "code for the Intel MIC." << std::endl;
                }
            }
        }
    }

    std::cout << std::endl;
    return 0;
}
