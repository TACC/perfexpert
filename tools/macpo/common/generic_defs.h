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
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sstream>
#include <vector>

#include "actions.h"

#define mprefix "[macpo] "

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

static bool is_same_file(const std::string& file_1, const std::string& file_2) {
    char path_1[PATH_MAX+1], path_2[PATH_MAX+1];

    std::string canonical_file_1 = (file_1[0] != '/' && file_1[0] != '.' ? "./"
            : "") + file_1;

    std::string canonical_file_2 = (file_2[0] != '/' && file_2[0] != '.' ? "./"
            : "") + file_2;

    if (realpath(canonical_file_1.c_str(), path_1) == NULL) {
        return false;
    }

    if (realpath(canonical_file_2.c_str(), path_2) == NULL) {
        return false;
    }

    return strncmp(path_1, path_2, PATH_MAX) == 0;
}

typedef struct _tag_options_t {
    bool no_compile;
    bool disable_sampling;
    bool profile_analysis;
    bool dynamic_inst;
    std::string base_compiler;
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
        int16_t action = ACTION_NONE;

        while (st != en) {
            const location_t& location = *st;

            bool same_file = is_same_file(location.function_name,
                _location.function_name);
            bool same_function = location.function_name ==
                _location.function_name;

            if (location.line_number == _location.line_number &&
                (same_file || same_function)) {
                action |= location.action;
            }

            st += 1;
        }

        return action;
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
        dynamic_inst = false;

        backup_filename.clear();
        base_compiler.clear();
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

inline void split(const std::string& string, char delim, name_list_t& list) {
    std::stringstream stream(string);
    std::string item;

    while (std::getline(stream, item, delim)) {
        list.push_back(item);
    }
}

static std::string get_path(std::string filename) {
    if (filename.empty() || filename[0] == '/' || filename[0] == '.') {
        return filename;
    }

    std::string path = getenv("PATH");
    name_list_t dirs;
    split(path, ':', dirs);

    for (name_list_t::iterator it = dirs.begin(); it != dirs.end(); it++) {
        std::string& dir = *it;
        std::string filepath = dir + "/" + filename;

        if (access(filepath.c_str(), R_OK | X_OK) == 0) {
            return filepath;
        }
    }

    return filename;
}

static bool execute_command(name_list_t arguments) {
    if (arguments.empty()) {
        return false;
    }

    // Construct argument list.
    int pid;
    const int num_args = arguments.size();
    char* argv[num_args + 1];
    for (int i = 0; i < num_args; i++) {
        size_t malloc_size = arguments[i].size() + 1 * sizeof(char);
        argv[i] = reinterpret_cast<char*>(malloc(malloc_size));
        strncpy(argv[i], arguments[i].c_str(), arguments[i].size());
        argv[i][arguments[i].size()] = '\0';
    }

    argv[num_args] = NULL;
    switch (pid = fork()) {
        case -1:
            return false;

        case 0:
            execvp(argv[0], argv);
            return false;

        default:
            wait(NULL);
            break;
    }

    for (int i = 0; i < num_args; i++) {
        free(argv[i]);
        argv[i] = NULL;
    }

    return true;
}

#define ADDR_TO_CACHE_LINE(x)   (x >> 6)

#endif  // TOOLS_MACPO_COMMON_GENERIC_DEFS_H_
