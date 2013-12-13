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

#ifndef CACHE_CONFLICT_ANALYSIS_H_
#define CACHE_CONFLICT_ANALYSIS_H_

#include "analysis_defs.h"
#include "avl_tree.h"

int cache_conflict_analysis(const global_data_t& global_data,
        double_list_t& conflict_list);

int print_cache_conflicts(const global_data_t& global_data,
        double_list_t& conflict_list);

#endif  /* CACHE_CONFLICT_ANALYSIS_H_ */
