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

#ifndef TOOLS_MACPO_COMMON_GENERIC_DEFS_H_
#define TOOLS_MACPO_COMMON_GENERIC_DEFS_H_

#include <stdint.h>

#include <algorithm>
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
const int16_t ACTION_REUSEDISTANCE  = 1 <<  8;
const int16_t ACTION_LAST           = 1 <<  9;

typedef struct {
    int16_t action;
    int line_number;
    std::string function_name;
} location_t;

typedef std::vector<location_t> location_list_t;

static bool operator==(const location_t& val_01, const location_t& val_02) {
    return val_01.line_number == val_02.line_number &&
        val_01.function_name == val_02.function_name;
}

typedef struct _tag_options_t {
    bool no_compile;
    bool disable_sampling;
    bool profile_analysis;
    std::string backup_filename;
    location_list_t location_list;

    _tag_options_t() {
        reset();
    }

    void add_location(int16_t action, const location_t& _location) {
        // Check if this location is already in the list.
        location_list_t::iterator st = location_list.begin();
        location_list_t::iterator en = location_list.end();
        location_list_t::iterator it = std::find(st, en, _location);

        // If we find it, add this action to the list of actions.
        if (it != en) {
            it->action |= action;
        }

        // If not, add it to the list.
        location_t location = _location;
        location.action = action;
        location_list.push_back(location);
    }

    void add_location(int16_t action, const std::string& _function_name,
            const int& _line_number) {
        location_t location;
        location.function_name = _function_name;
        location.line_number = _line_number;
        add_location(action, location);
    }

    void add_location(int16_t action, const std::string& _function_name) {
        add_location(action, _function_name, 0);
    }

    int16_t get_action(const location_t& _location) const {
        location_list_t::const_iterator st = location_list.begin();
        location_list_t::const_iterator en = location_list.end();
        location_list_t::const_iterator it = std::find(st, en, _location);

        if (it != en) {
            return it->action;
        }

        return ACTION_NONE;
    }

    int16_t get_action(const std::string& _function_name,
            const int& _line_number) const {
        location_t location;
        location.function_name = _function_name;
        location.line_number = _line_number;
        return get_action(location);
    }

    int16_t get_action(const std::string& _function_name) const {
        return get_action(_function_name, 0);
    }

    void reset() {
        no_compile = false;
        disable_sampling = false;
        profile_analysis = false;

        backup_filename.clear();
        location_list.clear();
    }
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

#define ADDR_TO_CACHE_LINE(x)   (x >> 6)

#endif  // TOOLS_MACPO_COMMON_GENERIC_DEFS_H_
