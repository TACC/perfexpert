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

#ifndef	GENERIC_DEFS_H_
#define	GENERIC_DEFS_H_

#include "macpo_record.h"

enum { ACTION_NONE=0, ACTION_INSTRUMENT, ACTION_ALIGNCHECK };

const int FLAG_NONE = 0;
const int FLAG_NOCOMPILE = 1 << 0;

typedef struct {
    char* function;
} function_info_t;

typedef struct {
    char* function;
    int line_number;
} loop_info_t;

typedef struct {
    int flags;
    short action;
    loop_info_t loopInfo;
    function_info_t functionInfo;
} options_t;

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
} reference_info_t;

class attrib {
    public:
        bool access_type, skip;

        attrib() {
            access_type = TYPE_UNKNOWN;
            skip = false;
        }
};

#endif	/* GENERIC_DEFS_H_ */
