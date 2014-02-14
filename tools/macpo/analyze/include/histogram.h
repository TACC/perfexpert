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

#ifndef HISTOGRAM_H_
#define HISTOGRAM_H_

#include <algorithm>
#include <gsl/gsl_histogram.h>

#include "analysis_defs.h"

bool pair_sort(const pair_t& p1, const pair_t& p2);

int flatten_and_sort_histogram(gsl_histogram*& hist, pair_list_t& pair_list);

int create_histogram_if_null(gsl_histogram*& hist, size_t bins);

#endif  /* HISTOGRAM_H_ */
