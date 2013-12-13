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

#ifndef INST_DEFS_H_
#define INST_DEFS_H_

#include <rose.h>

#include "macpo_record.h"

typedef struct {
    SgBasicBlock* bb;
    SgStatement* stmt;
    std::vector<SgExpression*> params;

    // Other fields used while inserting the call.
    bool before;
    std::string function_name;
} inst_info_t;

typedef std::vector<inst_info_t> inst_list_t;

typedef struct {
    std::string name;
    short access_type;
    long idx;
    SgNode* node;
} reference_info_t;

typedef std::vector<reference_info_t> reference_list_t;

class attrib {
    public:
        bool access_type, skip;

        attrib() {
            access_type = TYPE_UNKNOWN;
            skip = false;
        }
};

#endif  /* INST_DEFS_H_ */
