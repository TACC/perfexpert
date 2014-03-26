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

#ifndef VECTOR_STRIDE_ANALYSIS_H_
#define VECTOR_STRIDE_ANALYSIS_H_

#include "histogram.h"

#define MAX_STRIDE      128
#define STRIDE_COUNT    3

int vector_stride_analysis(const global_data_t& global_data,
        histogram_list_t& stride_list);

int print_vector_strides(const global_data_t& global_data,
        histogram_list_t& stride_list);

#endif  /* VECTOR_STRIDE_ANALYSIS_H_ */
