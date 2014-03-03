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

#ifndef RECORD_ANALYSIS_H_
#define RECORD_ANALYSIS_H_

#include "argp_custom.h"
#include "generic_defs.h"
#include "macpo_record.h"

#define CUT             0.8f
#define LINE_NUM_LIMIT  10000

int analyze_records(const global_data_t& global_data, int analysis_flags,
        const struct arg_info& info);
int filter_low_freq_records(global_data_t& global_data);

#endif  /* RECORD_ANALYSIS_H_ */
