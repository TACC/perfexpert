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

#include "err_codes.h"
#include "generic_defs.h"
#include "record_io.h"

static int handle_stream_msg(const stream_info_t& stream_info,
        global_data_t& global_data) {
    global_data.stream_list.push_back(stream_info.stream_name);
    return 0;
}

static int handle_trace_msg(const trace_info_t& trace_info,
        global_data_t& global_data) {
    // If the bucket does not have any lists,
    // then create a new (empty) list and push items there.
    trace_info_bucket_t& bucket = global_data.trace_info_bucket;
    if (bucket.size() == 0) {
        trace_info_list_t info_list;
        bucket.push_back(info_list);
    }

    // Get the last list from the bucket and
    // insert the mem_info struct there.
    trace_info_list_t& last_list = bucket.at(bucket.size() - 1);
    last_list.push_back(trace_info);

    return 0;
}

static int handle_mem_msg(const mem_info_t& mem_info, global_data_t& global_data) {
    // If the bucket does not have any lists,
    // then create a new (empty) list and push items there.
    mem_info_bucket_t& bucket = global_data.mem_info_bucket;
    if (bucket.size() == 0) {
        mem_info_list_t info_list;
        bucket.push_back(info_list);
    }

    // Get the last list from the bucket and
    // insert the mem_info struct there.
    mem_info_list_t& last_list = bucket.at(bucket.size() - 1);
    last_list.push_back(mem_info);

    return 0;
}

static int handle_metadata_msg(const metadata_info_t& metadata_info) {
    // We don't need this metadata after we've read it,
    // so use it and discard it instead of saving it for later.
    std::cout << mprefix << "Analyzing logs created from the binary " <<
        metadata_info.binary_name << " at " <<
        ctime(&metadata_info.execution_timestamp) << std::endl;

    return 0;
}

static int handle_terminal_msg(global_data_t& global_data) {
    mem_info_bucket_t& bucket = global_data.mem_info_bucket;

    // Create an empty list within the bucket.
    // Subsequent messages go into this empty bucket.
    mem_info_list_t info_list;
    bucket.push_back(info_list);

    return 0;
}

int print_trace_records(const global_data_t& global_data) {
    const trace_info_bucket_t& bucket = global_data.trace_info_bucket;

    // Loop over all elements of the bucket.
    for (int i=0; i<bucket.size(); i++) {
        const trace_info_list_t& list = bucket.at(i);

        for (int j=0; j<list.size(); j++) {
            const trace_info_t& trace_info = list.at(j);
            switch (trace_info.read_write) {
                case TYPE_READ:             std::cout << "R : "; break;
                case TYPE_WRITE:            std::cout << "W : "; break;
                case TYPE_READ_AND_WRITE:   std::cout << "RW: "; break;
                case TYPE_UNKNOWN:          std::cout << "??: "; break;
                default:                    assert(0);
            }

            assert(trace_info.var_idx < global_data.stream_list.size());
            std::cout << global_data.stream_list[trace_info.var_idx] << "+";
            std::cout << trace_info.address - trace_info.base << std::endl;
        }
    }

    return 0;
}

int read_file(const char* filename, global_data_t& global_data) {
    int code = 0;

    int fd;
    if ((fd = open(filename, O_RDONLY)) < 0)
        return -ERR_FILE;

    node_t data_node;
    while (read(fd, &data_node, sizeof(data_node)) == sizeof(data_node)) {
        switch(data_node.type_message) {
            case MSG_STREAM_INFO:
                if ((code = handle_stream_msg(data_node.stream_info,
                        global_data)) < 0)
                    return code;
                break;

            case MSG_MEM_INFO:
                if ((code = handle_mem_msg(data_node.mem_info,
                        global_data)) < 0)
                    return code;
                break;

            case MSG_TRACE_INFO:
                if ((code = handle_trace_msg(data_node.trace_info,
                        global_data)) < 0)
                    return code;
                break;

            case MSG_METADATA:
                if ((code = handle_metadata_msg(data_node.metadata_info)) < 0)
                    return code;
                break;

            case MSG_TERMINAL:
                if ((code = handle_terminal_msg(global_data)) < 0)
                    return code;
                break;

            default:
                return -ERR_UNKNOWN_MSG;
        }
    }

    close(fd);
    return 0;
}                        
