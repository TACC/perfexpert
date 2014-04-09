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

#ifndef TOOLS_MACPO_INCLUDE_INST_DEFS_H_
#define TOOLS_MACPO_INCLUDE_INST_DEFS_H_

#include <rose.h>

#include <string>
#include <vector>

#include "macpo_record.h"

typedef struct {
    SgStatement *statement, *reference_statement;
    bool before;
} statement_info_t;

typedef std::vector<statement_info_t> statement_list_t;

typedef struct {
    std::string name;
    int16_t access_type;
    int64_t idx;
    SgNode* node;
} reference_info_t;

typedef std::vector<reference_info_t> reference_list_t;

struct tag_loop_info_t;
typedef std::vector<struct tag_loop_info_t> loop_info_list_t;

typedef std::vector<SgExpression*> expr_list_t;

typedef struct tag_loop_info_t {
    SgScopeStatement* loop_stmt;
    SgExpression* idxv_expr;
    SgExpression* init_expr;
    SgExpression* test_expr;
    SgExpression* incr_expr;

    int incr_op, processed;
    reference_list_t reference_list;
    std::vector<loop_info_list_t> child_loop_info;
} loop_info_t;

class attrib {
 public:
    bool skip;
    int16_t access_type;

    attrib() {
        access_type = TYPE_UNKNOWN;
        skip = false;
    }
};

#endif  // TOOLS_MACPO_INCLUDE_INST_DEFS_H_
