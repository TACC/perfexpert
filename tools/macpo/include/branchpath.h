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

#ifndef TOOLS_MACPO_INCLUDE_BRANCHPATH_H_
#define TOOLS_MACPO_INCLUDE_BRANCHPATH_H_

#include <rose.h>
#include <VariableRenaming.h>

#include "inst_defs.h"
#include "traversal.h"

class branchpath_t : public traversal_t {
 public:
    explicit branchpath_t(VariableRenaming*& _var_renaming) :
        traversal_t(_var_renaming) {}

 private:
    bool instrument_loop(loop_info_t& loop_info);
};

#endif  // TOOLS_MACPO_INCLUDE_BRANCHPATH_H_
