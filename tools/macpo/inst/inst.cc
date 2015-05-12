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

#include <cstdio>
#include <string>

#include "libmacpo.h"
#include "generic_defs.h"

bool parse_location(const std::string& argument, location_t& location) {
    const std::string str_argument = argument;

    // Initialize
    size_t pos = 0;
    location.function_name = "";
    location.line_number = 0;

    do {
        // Find the separating ':' character.
        pos = str_argument.find(":", pos);

        // We couldn't find a line number component.
        if (pos == std::string::npos) {
            location.function_name = str_argument;
            break;
        }

        // We did find a line number component.
        // Make sure that this ':' isn't a part of the '::',
        // which is the separator between the class name and the method name.
        if (pos < str_argument.length() - 1 && isdigit(str_argument[pos+1])) {
            location.function_name = str_argument.substr(0, pos);
            location.line_number = atoi(str_argument.substr(pos+1).c_str());
            break;
        } else {
            // Push pos by one character so that we keep making progress.
            pos += 1;
        }
    } while (true);

    return true;
}

int parse_argument(char* arg, macpo_options_t& macpo_options) {
    std::string argument = arg, option, value;

    if (argument.compare(0, 8, "--macpo:") != 0)
        return -1;

    size_t start_position = -1, end_position;
    if ((start_position = argument.find(":")) == std::string::npos)
        return -1;

    end_position = argument.find("=", start_position + 1);
    if (end_position == std::string::npos) {
        option = argument.substr(start_position + 1);
        value = "";
    } else {
        option = argument.substr(start_position + 1,
            end_position - start_position - 1);
        value = argument.substr(end_position + 1);
    }

    location_t location;
    if (option == "instrument") {
        if (!value.size())
            return -1;

        parse_location(value, location);
        void* alloc = malloc(sizeof(char) * location.function_name.size() + 1);
        char* name = reinterpret_cast<char*>(alloc);
        snprintf(name, location.function_name.size() + 1, "%s",
                location.function_name.c_str());
        instrument_block(&macpo_options, ACTION_INSTRUMENT, name,
                location.line_number);
    } else if (option == "check-alignment") {
        if (!value.size())
            return -1;

        parse_location(value, location);
        void* alloc = malloc(sizeof(char) * location.function_name.size() + 1);
        char* name = reinterpret_cast<char*>(alloc);
        snprintf(name, location.function_name.size() + 1, "%s",
                location.function_name.c_str());
        instrument_block(&macpo_options, ACTION_ALIGNCHECK, name,
                location.line_number);
    } else if (option == "record-tripcount") {
        if (!value.size())
            return -1;

        parse_location(value, location);
        void* alloc = malloc(sizeof(char) * location.function_name.size() + 1);
        char* name = reinterpret_cast<char*>(alloc);
        snprintf(name, location.function_name.size() + 1, "%s",
                location.function_name.c_str());
        instrument_block(&macpo_options, ACTION_TRIPCOUNT, name,
                location.line_number);
    } else if (option == "record-branchpath") {
        if (!value.size())
            return -1;

        parse_location(value, location);
        void* alloc = malloc(sizeof(char) * location.function_name.size() + 1);
        char* name = reinterpret_cast<char*>(alloc);
        snprintf(name, location.function_name.size() + 1, "%s",
                location.function_name.c_str());
        instrument_block(&macpo_options, ACTION_BRANCHPATH, name,
                location.line_number);
    } else if (option == "gen-trace") {
        if (!value.size())
            return -1;

        parse_location(value, location);
        void* alloc = malloc(sizeof(char) * location.function_name.size() + 1);
        char* name = reinterpret_cast<char*>(alloc);
        snprintf(name, location.function_name.size() + 1, "%s",
                location.function_name.c_str());
        instrument_block(&macpo_options, ACTION_GENTRACE, name,
                location.line_number);
    } else if (option == "vector-strides") {
        if (!value.size())
            return -1;

        parse_location(value, location);
        void* alloc = malloc(sizeof(char) * location.function_name.size() + 1);
        char* name = reinterpret_cast<char*>(alloc);
        snprintf(name, location.function_name.size() + 1, "%s",
                location.function_name.c_str());
        instrument_block(&macpo_options, ACTION_VECTORSTRIDES, name,
                location.line_number);
    } else if (option == "overlap-check") {
        if (!value.size())
            return -1;

        parse_location(value, location);
        void* alloc = malloc(sizeof(char) * location.function_name.size() + 1);
        char* name = reinterpret_cast<char*>(alloc);
        snprintf(name, location.function_name.size() + 1, "%s",
                location.function_name.c_str());
        instrument_block(&macpo_options, ACTION_OVERLAPCHECK, name,
                location.line_number);
    } else if (option == "stride-check") {
        if (!value.size())
            return -1;

        parse_location(value, location);
        void* alloc = malloc(sizeof(char) * location.function_name.size() + 1);
        char* name = reinterpret_cast<char*>(alloc);
        snprintf(name, location.function_name.size() + 1, "%s",
                location.function_name.c_str());
        instrument_block(&macpo_options, ACTION_STRIDECHECK, name,
                location.line_number);
    } else if (option == "reuse-distance") {
        if (!value.size())
            return -1;

        parse_location(value, location);
        void* alloc = malloc(sizeof(char) * location.function_name.size() + 1);
        char* name = reinterpret_cast<char*>(alloc);
        snprintf(name, location.function_name.size() + 1, "%s",
                location.function_name.c_str());
        instrument_block(&macpo_options, ACTION_REUSEDISTANCE, name,
                location.line_number);
    } else if (option == "backup-filename") {
        if (!value.size())
            return -1;

        void* alloc = malloc(sizeof(char) * value.size() + 1);
        char* name = reinterpret_cast<char*>(alloc);
        snprintf(name, value.size() + 1, "%s", value.c_str());
        macpo_options.backup_filename = name;
    } else if (option == "no-compile") {
        set_no_compile_flag(&macpo_options, 1);
    } else if (option == "enable-sampling") {
        set_disable_sampling_flag(&macpo_options, 0);
    } else if (option == "disable-sampling") {
        set_disable_sampling_flag(&macpo_options, 1);
    } else if (option == "profile-analysis") {
        set_profiling_flag(&macpo_options, 1);
    } else if (option == "compiler") {
        // Check if we were passed a valid executable.
        if (!value.size()) {
            return -1;
        }

        void* alloc = malloc(sizeof(char) * value.size() + 1);
        char* name = reinterpret_cast<char*>(alloc);
        snprintf(name, value.size() + 1, "%s", value.c_str());
        macpo_options.base_compiler = name;
    } else if (option == "dynamic_inst") {
        set_dynamic_instrumentation_flag(&macpo_options, 1);
    } else {
        return -1;
    }

    return 0;
}

int main(int argc, char* argv[]) {
    name_list_t arguments;
    arguments.push_back(argv[0]);

    macpo_options_t macpo_options={0};

    for (int i = 1; i < argc; i++) {
        // Check if argv[i] starts with "--macpo:".
        if (strstr(argv[i], "--macpo:") == argv[i]) {
            if (parse_argument(argv[i], macpo_options) < 0) {
                fprintf(stdout, "Unknown parameter: %s, aborting...\n",
                        argv[i]);
                return -1;
            }
        } else {
            arguments.push_back(argv[i]);
        }
    }

    // Convert the arguments vector back into char* for libmacpo functions.
    uint32_t size = arguments.size();
    const char** args = new const char*[size];
    for (int i = 0; i < size; i++) {
        args[i] = arguments[i].c_str();
    }

    int ret = instrument(&macpo_options, args, arguments.size());
    destroy_options(&macpo_options);

    delete args;
    args = NULL;

    return ret;
}
