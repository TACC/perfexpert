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

#ifndef CACHE_INFO_H_
#define CACHE_INFO_H_

#include "analysis_defs.h"

int load_cache_info(global_data_t& global_data);

int update_cache_fields(cache_data_t& cache_data, size_t cache_level,
        size_t cache_size, size_t line_size, size_t associativity);

int insert_cache_type(global_data_t& global_data, size_t cache_level,
        size_t cache_size, size_t line_size, size_t associativity);

#endif  /* CACHE_INFO_H_ */
