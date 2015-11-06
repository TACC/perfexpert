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

#ifndef TOOLS_MACPO_COMMON_MACPO_RECORD_H_
#define TOOLS_MACPO_COMMON_MACPO_RECORD_H_

#include <stdint.h>
#include <time.h>

#define STRING_LENGTH   256
#define STREAM_LENGTH   256

enum { TYPE_UNKNOWN = 0, TYPE_READ, TYPE_WRITE, TYPE_READ_AND_WRITE };
enum { MSG_TERMINAL = 0, MSG_STREAM_INFO, MSG_MEM_INFO, MSG_METADATA,
        MSG_TRACE_INFO, MSG_VECTOR_STRIDE_INFO };

typedef struct {
    uint16_t coreID;
    uint16_t read_write:2;
    size_t line_number;
    size_t base;
    size_t address;
    size_t var_idx;
} trace_info_t;

typedef struct {
    uint16_t coreID;
    size_t loop_line_number;
    size_t var_idx;
    size_t address;
    int type_size;
} vector_stride_info_t;

typedef struct {
    uint16_t coreID;
    uint16_t read_write:2;
    size_t line_number;
    size_t address;
    size_t var_idx;
    int type_size;
} mem_info_t;

typedef struct {
    char binary_name[STRING_LENGTH];
    time_t execution_timestamp;
} metadata_info_t;

typedef struct {
    char stream_name[STREAM_LENGTH];
} stream_info_t;

typedef struct node {
    uint16_t type_message;
    union {
        mem_info_t mem_info;
        trace_info_t trace_info;
        stream_info_t stream_info;
        metadata_info_t metadata_info;
        vector_stride_info_t vector_stride_info;
    };
} node_t;

#endif  // TOOLS_MACPO_COMMON_MACPO_RECORD_H_
