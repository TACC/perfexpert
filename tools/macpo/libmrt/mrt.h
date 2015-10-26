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

#ifndef TOOLS_MACPO_TESTS_LIBMRT_MRT_H_
#define TOOLS_MACPO_TESTS_LIBMRT_MRT_H_

#include <stdint.h>

#define test_prefix "\n[macpo-integration-test]:"

#if defined(__cplusplus)
extern "C" {
#endif
void indigo__exit();
#if defined (__cplusplus)
}
#endif

#if defined(__cplusplus)
extern "C" {
#endif
void indigo__record_branch_c(int line_number, void* func_addr,
    int loop_line_number, int true_branch_count, int false_branch_count);
#if defined (__cplusplus)
}
#endif

#if defined(__cplusplus)
extern "C" {
#endif
void indigo__vector_stride_c(int loop_line_number, int var_idx, void* addr,
    int type_size);
#if defined (__cplusplus)
}
#endif

#if defined(__cplusplus)
extern "C" {
#endif
int indigo__aligncheck_c(int line_number, void* func_addr, int stream_count,
    int dep_status, ...);
#if defined (__cplusplus)
}
#endif

#if defined(__cplusplus)
extern "C" {
#endif
int indigo__sstore_aligncheck_c(int line_number, void* func_addr,
    int stream_count, int dep_status, ...);
#if defined (__cplusplus)
}
#endif

#if defined(__cplusplus)
extern "C" {
#endif
void indigo__tripcount_check_c(int line_number, void* func_addr,
    int64_t trip_count);
#if defined (__cplusplus)
}
#endif

#if defined(__cplusplus)
extern "C" {
#endif
void indigo__init_(int16_t create_file, int16_t enable_sampling);
#if defined (__cplusplus)
}
#endif

#if defined(__cplusplus)
extern "C" {
#endif
void indigo__gen_trace_c(int read_write, int line_number, void* base,
        void* addr, int var_idx);
#if defined (__cplusplus)
}
#endif

#if defined(__cplusplus)
extern "C" {
#endif
void indigo__record_c(int read_write, int line_number, void* addr,
        int var_idx, int type_size);
#if defined (__cplusplus)
}
#endif

#if defined(__cplusplus)
extern "C" {
#endif
void indigo__write_idx_c(const char* var_name, const int length);
#if defined (__cplusplus)
}
#endif

#if defined(__cplusplus)
extern "C" {
#endif
void indigo__create_map() __attribute__((weak));
#if defined (__cplusplus)
}
#endif

#if defined(__cplusplus)
extern "C" {
#endif
void indigo__overlap_check_c(int line_number, void* func_addr,
    int stream_count, ...);
#if defined (__cplusplus)
}
#endif

#if defined(__cplusplus)
extern "C" {
#endif
void indigo__unknown_stride_check_c(int line_number, void* func_addr);
#if defined (__cplusplus)
}
#endif

#if defined(__cplusplus)
extern "C" {
#endif
void indigo__stride_check_c(int line_number, void* func_addr, int stride);
#if defined (__cplusplus)
}
#endif

#if defined(__cplusplus)
extern "C" {
#endif
void indigo__reuse_dist_c(int var_id, void* address);
#if defined (__cplusplus)
}
#endif
#endif  // TOOLS_MACPO_TESTS_LIBMRT_MRT_H_
