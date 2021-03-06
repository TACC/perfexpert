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

#ifndef TOOLS_MACPO_INST_INCLUDE_REUSE_DIST_H_
#define TOOLS_MACPO_INST_INCLUDE_REUSE_DIST_H_

#include <rose.h>

#include "inst_defs.h"
#include "traversal.h"

class reuse_dist_t : public traversal_t {
 public:
    explicit reuse_dist_t(const du_table_t& def_table) :
        traversal_t(def_table) {
        set_deep_search(false);
    }

 private:
    bool instrument_loop(loop_info_t& loop_info);
};

#endif  // TOOLS_MACPO_INST_INCLUDE_REUSE_DIST_H_
