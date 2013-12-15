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

#include "argp_custom.h"
#include "cache_info.h"
#include "err_codes.h"
#include "macpo_record.h"
#include "record_io.h"
#include "record_analysis.h"

int main(int argc, char *argv[]) {
    int code = 0;
    struct arg_info info;
    global_data_t global_data;
    memset(&global_data, 0, sizeof(global_data));

    memset (&info, 0, sizeof(struct arg_info));
    argp_parse (&argp, argc, argv, 0, 0, &info);

    if (info.arg2 == NULL) {
        info.threshold = 0.1;   // Default threshold of 10%
        info.location = info.arg1;
    } else {
        sscanf(info.arg1, "%f", &info.threshold);
        info.location = info.arg2;
    }

    if ((code = load_cache_info(global_data)) < 0) {
        std::cerr << "Failed to load cache information, terminating." <<
            std::endl;

        return code;
    }

    if ((code = read_file(info.location, global_data)) < 0) {
        std::cerr << "Failed to read records from file, terminating." <<
            std::endl;

        return code;
    }

    if ((code = filter_low_freq_records(global_data)) < 0) {
        std::cerr << "Failed to filter low-frequency records, terminating." <<
            std::endl;

        return code;
    }

    int analysis_flags = ANALYSIS_ALL;

    // TODO: Set analysis_flags based on analyses selected via arguments.

    if ((code = analyze_records(global_data, analysis_flags)) < 0) {
        std::cerr << "Failed to analyze records, terminating." << std::endl;
        return code;
    }

    return 0;
}
