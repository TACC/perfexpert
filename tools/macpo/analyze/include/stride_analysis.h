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

#ifndef STRIDE_ANALYSIS_H_
#define STRIDE_ANALYSIS_H_

#include "histogram.h"

#define MAX_STRIDE      128
#define STRIDE_COUNT    3

static const char* MSG_STRIDE_ANALYSIS = "stride_analysis";

static const char* MSG_STRIDE_VALUE = "stride_value";
static const char* MSG_STRIDE_COUNT = "stride_count";

int stride_analysis(const global_data_t& global_data,
        histogram_list_t& stride_list);

int print_strides(const global_data_t& global_data,
        histogram_list_t& stride_list, bool bot);

#endif  /* STRIDE_ANALYSIS_H_ */
