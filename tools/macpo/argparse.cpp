
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

#include "argparse.h"

#include <fcntl.h>
#include <unistd.h>

#include <cstring>
#include <cstdlib>
#include <cctype>
#include <iostream>
#include <string>

bool argparse::copy_file(const char *source_file,
        const char *destination_file) {
    if (!source_file || !destination_file)
        return false;

    struct stat stat_buf;

    if (access(source_file, R_OK) < 0) {
        std::cerr << mprefix << "Cannot read file: " << source_file <<
            std::endl;
        return false;
    }

    int src, dst;
    src = open(source_file, O_RDONLY);
    if (src < 0) {
        std::cerr << mprefix << "Cannot open file for reading: " <<
            source_file << std::endl;
        return false;
    }

    dst = open(destination_file, O_WRONLY | O_CREAT | O_TRUNC,
            stat_buf.st_mode & 0777);
    if (dst < 0) {
        std::cerr << mprefix << "Cannot open file for writing: " <<
            destination_file << std::endl;
        close(src);
        return false;
    }

    char buffer[1024];
    size_t n;
    while (n = read(src, buffer, 1024)) {
        write(dst, buffer, n);
    }

    close(src);
    close(dst);
    return true;
}

bool argparse::parse_location(const std::string& argument,
        std::string& function_name, int& line_number) {
    const std::string str_argument = argument;

    // Initialize
    size_t pos = 0;
    function_name = "";
    line_number = 0;

    do {
        // Find the separating ':' character.
        pos = str_argument.find(":", pos);

        // We couldn't find a line number component.
        if (pos == std::string::npos) {
            function_name = str_argument;
            break;
        }

        // We did find a line number component.
        // Make sure that this ':' isn't a part of the '::',
        // which is the separator between the class name and the method name.
        if (pos < str_argument.length() - 1 && isdigit(str_argument[pos+1])) {
            function_name = str_argument.substr(0, pos);
            line_number = atoi(str_argument.substr(pos+1).c_str());
            break;
        } else {
            // Push pos by one character so that we keep making progress.
            pos += 1;
        }
    } while (true);

    return true;
}

#define CHECK_EQUAL_SIGN(eq)    if (!eq || !*(++eq))    return -1;

int argparse::parse_arguments(char* arg, options_t& options) {
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

    if (option == "instrument") {
        if (!value.size())
            return -1;

        set_action(options.action, ACTION_INSTRUMENT);
        parse_location(value, options.function_name, options.line_number);
    } else if (option == "check-alignment") {
        if (!value.size())
            return -1;

        set_action(options.action, ACTION_ALIGNCHECK);
        parse_location(value, options.function_name, options.line_number);
    } else if (option == "record-tripcount") {
        if (!value.size())
            return -1;

        set_action(options.action, ACTION_TRIPCOUNT);
        parse_location(value, options.function_name, options.line_number);
    } else if (option == "record-branchpath") {
        if (!value.size())
            return -1;

        set_action(options.action, ACTION_BRANCHPATH);
        parse_location(value, options.function_name, options.line_number);
    } else if (option == "gen-trace") {
        if (!value.size())
            return -1;

        set_action(options.action, ACTION_GENTRACE);
        parse_location(value, options.function_name, options.line_number);
    } else if (option == "vector-strides") {
        if (!value.size())
            return -1;

        set_action(options.action, ACTION_VECTORSTRIDES);
        parse_location(value, options.function_name, options.line_number);
    } else if (option == "overlap-check") {
        if (!value.size())
            return -1;

        set_action(options.action, ACTION_OVERLAPCHECK);
        parse_location(value, options.function_name, options.line_number);
    } else if (option == "stride-check") {
        if (!value.size())
            return -1;

        set_action(options.action, ACTION_STRIDECHECK);
        parse_location(value, options.function_name, options.line_number);
    } else if (option == "backup-filename") {
        if (!value.size())
            return -1;

        options.backup_filename = value;
    } else if (option == "no-compile") {
        options.no_compile = true;
    } else if (option == "enable-sampling") {
        options.disable_sampling = false;
    } else if (option == "disable-sampling") {
        options.disable_sampling = true;
    } else if (option == "profile-analysis") {
        options.profile_analysis = true;
    } else {
        return -1;
    }

    return 0;
}

void argparse::init_options(options_t& options) {
    options.action = ACTION_NONE;
    options.no_compile = false;
    options.disable_sampling = false;
    options.profile_analysis = false;
    options.line_number = 0;
    options.function_name = "";
    options.backup_filename = "";
}

bool argparse::validate_options(const options_t& options) {
    // All valid actions require a non-empty function name.
    if (options.action != ACTION_NONE && options.function_name == "") {
        return false;
    }

    // Validate actions.
    if (options.action < ACTION_NONE || options.action >= ACTION_LAST) {
        return false;
    }

    // ACTION_INSTRUMENT is an exclusive option.
    if (is_action(options.action, ACTION_INSTRUMENT) &&
            __builtin_popcount(options.action) != 1) {
        return false;
    }

    // ACTION_GENTRACE is an exclusive option.
    if (is_action(options.action, ACTION_GENTRACE) &&
            __builtin_popcount(options.action) != 1) {
        return false;
    }

    // Everything checked out well.
    return true;
}
