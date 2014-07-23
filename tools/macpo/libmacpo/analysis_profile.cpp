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

#include <ctime>

#include "analysis_profile.h"
#include "inst_defs.h"

using namespace SageBuilder;
using namespace SageInterface;

analysis_profile_t::analysis_profile_t() {
    start_time = 0.0;
    end_time = 0.0;

    loop_info_list.clear();
}

void analysis_profile_t::start_timer() {
    start_time = clock();
}

void analysis_profile_t::end_timer() {
    end_time = clock();
}

const double analysis_profile_t::get_running_time() const {
    return (end_time - start_time) / CLOCKS_PER_SEC;
}

const loop_info_list_t& analysis_profile_t::get_loop_info_list() const {
    return loop_info_list;
}

void analysis_profile_t::set_loop_info_list(loop_info_list_t& _loop_info_list) {
    loop_info_list = _loop_info_list;
}
