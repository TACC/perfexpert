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

#define test_prefix "\n[macpo-integration-test]:"

#ifndef LIBMRT_H_
#define LIBMRT_H_

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
void indigo__record_branch_c(int line_number, int loop_line_number,
    int true_branch_count, int false_branch_count);
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
int indigo__aligncheck_c(int line_number, int stream_count, ...);
#if defined (__cplusplus)
}
#endif

#if defined(__cplusplus)
extern "C" {
#endif
int indigo__sstore_aligncheck_c(int line_number, int stream_count, ...);
#if defined (__cplusplus)
}
#endif

#if defined(__cplusplus)
extern "C" {
#endif
void indigo__tripcount_check_c(int line_number, long trip_count);
#if defined (__cplusplus)
}
#endif

#if defined(__cplusplus)
extern "C" {
#endif
void indigo__init_(short create_file, short enable_sampling);
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

#endif  /* LIBMRT_H_ */
