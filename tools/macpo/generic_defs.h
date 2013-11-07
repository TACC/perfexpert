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

enum { FALSE=0, TRUE };
enum { SUCCESS=0, ERR_PARAMS };
enum { ACTION_NONE=0, ACTION_INSTRUMENT, ACTION_ALIGNCHECK };

const int FLAG_NONE = 0;
const int FLAG_NOCOMPILE = 1 << 0;

typedef	unsigned char BOOL;

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
        char* inst_func;
        int line_number;
        BOOL read, skip, atomic;

        attrib(BOOL _read, BOOL _skip, char* _inst_func, int _line_number) {
            atomic=false, read=_read, skip=_skip, inst_func=_inst_func, line_number=_line_number;
        }
};

class attrib_temp {
    public:
        char* inst_func;
        int line_number;
        BOOL read, skip;

        SgNode* currNode;
        std::string var_name;

        attrib_temp(BOOL _read, BOOL _skip, char* _inst_func, int _line_number) {
            var_name="", currNode=NULL, read=_read, skip=_skip, inst_func=_inst_func, line_number=_line_number;
        }
};

#endif	/* GENERIC_DEFS_H_ */
