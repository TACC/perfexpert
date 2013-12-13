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

#include <algorithm>
#include <gsl/gsl_histogram.h>

#include "analysis_defs.h"
#include "histogram.h"

bool pair_sort(const pair_t& p1, const pair_t& p2) {
    size_t val_1 = p1.second;
    size_t val_2 = p2.second;

    return val_1 > val_2;
}

int flatten_and_sort_histogram(gsl_histogram*& hist, pair_list_t& pair_list) {
    // Initialize.
    pair_list.clear();

    size_t bin_count = gsl_histogram_bins(hist);
    pair_list.resize(bin_count);

    for (int i=0; i<bin_count; i++) {
        size_t count = gsl_histogram_get(hist, i);
        pair_list[i] = pair_t(i, count);
    }

    // Sort based on values.
    std::sort(pair_list.begin(), pair_list.end(), pair_sort);
}

