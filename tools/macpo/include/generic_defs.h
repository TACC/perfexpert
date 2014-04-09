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

// Bitmap for ACTION values.
// XXX: ACTION_NONE and ACTION_LAST are special actions!
// Define all other actions after ACTION_NONE and before ACTION_LAST!

const int16_t ACTION_NONE           = 0;
const int16_t ACTION_INSTRUMENT     = 1 <<  0;
const int16_t ACTION_ALIGNCHECK     = 1 <<  1;
const int16_t ACTION_GENTRACE       = 1 <<  2;
const int16_t ACTION_VECTORSTRIDES  = 1 <<  3;
const int16_t ACTION_TRIPCOUNT      = 1 <<  4;
const int16_t ACTION_BRANCHPATH     = 1 <<  5;
const int16_t ACTION_OVERLAPCHECK   = 1 <<  6;
const int16_t ACTION_STRIDECHECK    = 1 <<  7;
const int16_t ACTION_LAST           = 1 <<  8;

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

// Supporting routines for ACTION bitmap.
inline void set_action(int16_t& bitmap, const int16_t _action) {
    // Quick sanity checks on _action.
    if (__builtin_popcount(_action) != 1)
        return;

    if (_action <= ACTION_NONE || _action >= ACTION_LAST)
        return;

    // Finally, set the bitmap.
    bitmap |= _action;
}

inline bool is_action(int16_t bitmap, const int16_t _action) {
    // Quick sanity checks on _action.
    if (__builtin_popcount(_action) != 1)
        return false;

    if (_action <= ACTION_NONE || _action >= ACTION_LAST)
        return false;

    if (bitmap & _action)
        return true;
}

#endif  // TOOLS_MACPO_INCLUDE_GENERIC_DEFS_H_
