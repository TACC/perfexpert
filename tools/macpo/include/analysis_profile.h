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

#ifndef	ANALYSIS_PROFILE_H_
#define	ANALYSIS_PROFILE_H_

#include "inst_defs.h"

class analysis_profile_t {
    public:
        analysis_profile_t();

        void start_timer();
        void end_timer();
        const double get_running_time() const;

        void set_loop_info_list(loop_info_list_t& _loop_info_list);
        const loop_info_list_t& get_loop_info_list() const;

    private:
        double start_time, end_time;
        loop_info_list_t loop_info_list;
};

#endif	/* ANALYSIS_PROFILE_H_ */
