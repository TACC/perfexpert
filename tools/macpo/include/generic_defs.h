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

#ifndef TOOLS_MACPO_INCLUDE_GENERIC_DEFS_H_
#define TOOLS_MACPO_INCLUDE_GENERIC_DEFS_H_

#include <stdint.h>

#include <string>
#include <vector>

#define mprefix "[macpo] "

enum { ACTION_NONE = 0, ACTION_INSTRUMENT, ACTION_ALIGNCHECK, ACTION_GENTRACE,
        ACTION_VECTORSTRIDES, ACTION_TRIPCOUNT, ACTION_BRANCHPATH };

typedef struct {
    int16_t action;
    int line_number;
    bool no_compile;
    bool disable_sampling;
    bool profile_analysis;
    std::string function_name;
    std::string backup_filename;
} options_t;

typedef std::vector<std::string> name_list_t;

#endif  // TOOLS_MACPO_INCLUDE_GENERIC_DEFS_H_
