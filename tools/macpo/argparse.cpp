
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

#include <fcntl.h>
#include <string.h>

#include "argparse.h"

bool argparse::copy_file(const char *source_file,
        const char *destination_file) {
    if (!source_file || !destination_file)
        return false;

    struct stat stat_buf; 

    if (stat(source_file, &stat_buf) == -1) {
        std::cerr << "Cannot read permissions of file: " << source_file << "\n";
        return false;
    }

    int src, dst;
    src = open(source_file, O_RDONLY);
    if (src < 0) {
        std::cerr << "Cannot open file for reading: " << source_file << "\n";
        return false;
    }

    dst = open(destination_file, O_WRONLY | O_CREAT | O_TRUNC,
            stat_buf.st_mode & 0777);
    if (dst < 0) {
        std::cerr << "Cannot open file for writing: " << destination_file <<
            "\n";
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

bool argparse::parse_location(std::string& argument, std::string& function_name,
        int& line_number) {
    std::string str_argument = argument;

    // Initialize
    function_name = "";
    line_number = 0;

    size_t pos;
    if ((pos = str_argument.find_first_of("#")) != std::string::npos) {
        if (pos > 0 && pos < str_argument.size()-1) {
            function_name = str_argument.substr(0, pos);
            line_number = atoi(str_argument.substr(pos+1).c_str());
        } else {
            return false;
        }
    } else {
        function_name = str_argument;
        line_number = 0;
    }

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

        options.action = ACTION_INSTRUMENT;
        parse_location(value, options.function_name, options.line_number);
    } else if (option == "check-alignment") {
        if (!value.size())
            return -1;

        options.action = ACTION_ALIGNCHECK;
        parse_location(value, options.function_name, options.line_number);
    } else if (option == "backup-filename") {
        if (!value.size())
            return -1;

        options.backup_filename = value;
    } else if (option == "no-compile") {
        options.no_compile = true;
    } else
        return -1;

    return 0;
}

void argparse::init_options(options_t& options) {
    options.action = ACTION_NONE;
    options.no_compile = false;
    options.line_number = 0;
    options.function_name = "";
    options.backup_filename = "";
}
