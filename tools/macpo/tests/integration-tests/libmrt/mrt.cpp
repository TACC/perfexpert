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

#include <cstdarg>
#include <iostream>

#include "mrt.h"

void indigo__exit() {
    std::cerr << test_prefix << "exit:" << std::endl;
}

void indigo__record_branch_c(int line_number, int loop_line_number,
        int true_branch_count, int false_branch_count) {
    std::cerr << test_prefix << "record_branch:" << line_number << ":" <<
        loop_line_number << ":" << true_branch_count << ":" <<
        false_branch_count << ":" << std::endl;
}

void indigo__vector_stride_c(int loop_line_number, int var_idx, void* addr,
        int type_size) {
    std::cerr << test_prefix << "vector_stride:" << loop_line_number << ":" <<
        var_idx << ":" << addr << ":" << type_size << ":" << std::endl;
}

int indigo__aligncheck_c(int line_number, int stream_count, ...) {
    std::cerr << test_prefix << "aligncheck:" << line_number << ":" <<
        stream_count << ":";

    va_list args;
    va_start(args, stream_count);

    for (int i = 0; i < stream_count; i++) {
        void* address = va_arg(args, void*);
        std::cerr << address << ":";
    }

    va_end(args);
    return 0;
}

int indigo__sstore_aligncheck_c(int line_number, int stream_count, ...) {
    std::cerr << test_prefix << "sstore_aligncheck:" << line_number << ":" <<
        stream_count << ":";

    va_list args;
    va_start(args, stream_count);

    for (int i = 0; i < stream_count; i++) {
        void* address = va_arg(args, void*);
        std::cerr << address << ":";
    }

    va_end(args);
    return 0;
}

void indigo__tripcount_check_c(int line_number, int64_t trip_count) {
    std::cerr << test_prefix << "tripcount_check:" << line_number << ":" <<
        trip_count << std::endl;
}

void indigo__init_(int16_t create_file, int16_t enable_sampling) {
    std::cerr << test_prefix << "init:" << create_file << ":" <<
        enable_sampling << ":" << std::endl;
}

void indigo__gen_trace_c(int read_write, int line_number, void* base,
        void* addr, int var_idx) {
    std::cerr << test_prefix << "gen_trace:" << read_write << ":" <<
        line_number << ":" << base << ":" << addr << ":" << var_idx << ":" <<
        std::endl;
}

void indigo__record_c(int read_write, int line_number, void* addr,
        int var_idx, int type_size) {
    std::cerr << test_prefix << "record:" << read_write << ":" << line_number
        << ":" << addr << ":" << var_idx << ":" << type_size << ":" <<
        std::endl;
}

void indigo__write_idx_c(const char* var_name, const int length) {
    std::cerr << test_prefix << "write_idx:" << var_name << ":" << length <<
        ":" << std::endl;
}

void indigo__create_map() {
    std::cerr << test_prefix << "create_map:" << std::endl;
}

void indigo__overlap_check_c(int line_number, int stream_count, ...) {
    std::cerr << test_prefix << "overlap_check:" << line_number << ":" <<
        stream_count << ":";

    va_list args;
    va_start(args, stream_count);

    for (int i = 0; i < stream_count; i++) {
        void* start = va_arg(args, void*);
        void* end = va_arg(args, void*);
        std::cerr << start << ":" << end << ":";
    }

    va_end(args);
    std::cerr << std::endl;
}

void indigo__unknown_stride_check_c(int line_number) {
    std::cerr << test_prefix << "unknown_stride_check:" << line_number << ":" <<
        std::endl;
}

void indigo__stride_check_c(int line_number, int stride) {
    std::cerr << test_prefix << "stride_check:" << line_number << ":" <<
        stride << ":" << std::endl;
}

void indigo__reuse_dist_c(int var_id, void* address) {
    std::cerr << test_prefix << "reuse_dist:" << var_id << ":" <<
        address << ":" << std::endl;
}
