/*
 * Copyright (c) 2011-2015  University of Texas at Austin. All rights reserved.
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
 * Authors: Antonio Gomez-Iglesias, Leonardo Fialho and Ashay Rane
 *
 * $HEADER$
 */

#include <rose.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "argparse.h"
#include "libmacpo.h"
#include "minst.h"

extern "C" {
uint8_t get_no_compile_flag(const macpo_options_t* macpo_options) {
    if (macpo_options == NULL) {
        return -1;
    }

    return macpo_options->no_compile_flag;
}

void set_no_compile_flag(macpo_options_t* macpo_options, uint8_t flag) {
    if (macpo_options == NULL) {
        return;
    }

    macpo_options->no_compile_flag = flag;
}

uint8_t get_disable_sampling_flag(const macpo_options_t* macpo_options) {
    if (macpo_options == NULL) {
        return -1;
    }

    return macpo_options->disable_sampling_flag;
}

void set_disable_sampling_flag(macpo_options_t* macpo_options, uint8_t flag) {
    if (macpo_options == NULL) {
        return;
    }

    macpo_options->disable_sampling_flag = flag;
}

void set_dynamic_instrumentation_flag(macpo_options_t *macpo_options, uint8_t flag) {
    if (macpo_options == NULL) {
        return;
    }

    macpo_options->dynamic_inst_flag = flag;
}

uint8_t get_profiling_flag(const macpo_options_t* macpo_options) {
    if (macpo_options == NULL) {
        return -1;
    }

    return macpo_options->profiling_flag;
}

void set_profiling_flag(macpo_options_t* macpo_options, uint8_t flag) {
    if (macpo_options == NULL) {
        return;
    }

    macpo_options->profiling_flag = flag;
}

const char* get_base_compiler(const macpo_options_t* macpo_options) {
    if (macpo_options == NULL) {
        return NULL;
    }

    return macpo_options->base_compiler;
}

void set_base_compiler(macpo_options_t* macpo_options,
        const char* compiler_path) {
    if (macpo_options == NULL) {
        return;
    }

    macpo_options->base_compiler = compiler_path;
}

const char* get_backup_filename(const macpo_options_t* macpo_options) {
    if (macpo_options == NULL) {
        return NULL;
    }

    return macpo_options->backup_filename;
}

void set_backup_filename(macpo_options_t* macpo_options, const char* filename) {
    if (macpo_options == NULL) {
        return;
    }

    macpo_options->backup_filename = filename;
}

uint8_t instrument_block(macpo_options_t* macpo_options, uint16_t action,
        const char* name, uint32_t line_number) {
    if (macpo_options == NULL) {
        return -1;
    }

    if (action <= ACTION_NONE || action >= ACTION_LAST) {
        return -2;
    }

    // Check if this location has already been added to the list.
    code_location_t* head = macpo_options->location;
    while (head != NULL && (strcmp(head->name, name) != 0 ||
            (strcmp(head->name, name) == 0 &&
            head->line_number != line_number))) {
        head = head->next;
    }

    if (head != NULL) {
        // Found a location, update the bitmap and return.
        head->action |= action;
        return 0;
    }

    // Otherwise, create a new node and add it to the list.
    void* new_node = malloc(sizeof(code_location_t));
    if (new_node == NULL) {
        return -3;
    }

    code_location_t* location = reinterpret_cast<code_location_t*>(new_node);
    location->action = action;
    location->name = name;
    location->line_number = line_number;

    // Add this node to the head of the list.
    head = macpo_options->location;
    macpo_options->location = location;
    location->next = head;
    return 0;
}

uint8_t instrument_function(macpo_options_t* macpo_options, uint16_t action,
        const char* name) {
    return instrument_block(macpo_options, action, name, 0);
}

uint8_t destroy_options(macpo_options_t* macpo_options) {
    if (macpo_options == NULL) {
        return -1;
    }

    // Iterate over the location list and free the nodes.
    code_location_t* head = macpo_options->location;
    while (head != NULL) {
        code_location_t* next = head->next;
        free(head);
        head = next;
    }

    memset(macpo_options, 0, sizeof(macpo_options_t));
    return 0;
}

static options_t convert_macpo_options(macpo_options_t* macpo_options) {
    options_t options;
    argparse::init_options(options);

    if (macpo_options == NULL) {
        return options;
    }

    options.no_compile          = macpo_options->no_compile_flag == 1;
    options.disable_sampling    = macpo_options->disable_sampling_flag == 1;
    options.profile_analysis    = macpo_options->profiling_flag == 1;
    options.dynamic_inst        = macpo_options->dynamic_inst_flag == 1;

    if (macpo_options->backup_filename) {
        options.backup_filename = std::string(macpo_options->backup_filename);
    }

    if (macpo_options->base_compiler) {
        options.base_compiler = std::string(macpo_options->base_compiler);
    }

    code_location_t* location = macpo_options->location;
    while (location != NULL) {
        std::string name = std::string(location->name);
        options.add_location(location->action, name, location->line_number);

        location = location->next;
    }

    return options;
}

uint8_t instrument(macpo_options_t* macpo_options, const char* args[],
        uint32_t arg_count) {
    if (macpo_options == NULL) {
        return -1;
    }

    if (args == NULL || arg_count == 0) {
        return -2;
    }

    options_t options = convert_macpo_options(macpo_options);
    name_list_t arguments;
    for (int i = 0; i < arg_count; i++) {
        arguments.push_back(args[i]);
    }

    try {
        SgProject *project = project = frontend(arguments);
        ROSE_ASSERT(project != NULL);

        // uncomment to generate dot file to view AST
        //generateDOT(*project);

        if (midend(project, options) == false) {
            return -3;
        }

        if (options.no_compile == false && options.base_compiler.empty()) {
            // FIXME: ROSE tests seem to be broken,
            // operand of AddressOfOp is not allowed to be an l-value.
            // AstTests::runAllTests(project);
            /* Disabling backend of macpo.sh. Need to compile separately. */
            // return backend(project) == 0 ? 0 : -4;
        }
        project->unparse();

        // If no_compile is specified, exit
        if (options.no_compile == true) {
            return 0;
        }

         if (options.base_compiler.size() > 0) {
            std::string base_compiler = get_path(options.base_compiler);
            if (base_compiler.empty() ||
                    access(base_compiler.c_str(), R_OK | X_OK) == -1) {
                std::cerr << "Failed to find a valid compiler binary: " <<
                        options.base_compiler << std::endl;
                return -5;
            }

            // Replace the macpo binary with the target compiler.
            arguments[0] = base_compiler;

            // Loop over each file, creating a backup of the original file
            // and renaming the unparsed file to the name of the original file.
            for (int i = 0; i < project->numberOfFiles(); i++) {
                SgFile* file = (*project)[i];
                std::string in_filename = file->getFileName();
                std::string out_filename = file->get_unparse_output_filename();

                if (in_filename.size() >= 1 &&
                        in_filename[in_filename.size() - 1] == '/') {
                    // Looks like a directory, skip this name.
                    continue;
                }

                size_t last_slash = in_filename.rfind('/');
                std::string base = in_filename.substr(last_slash + 1);
                std::string dir = in_filename.substr(0, last_slash);
                std::string backup_filename = dir + "/backup." + base;

                // Backup the original file.
                argparse::copy_file(in_filename.c_str(),
                        backup_filename.c_str());

                // Now overwrite the original file with the unparsed file.
                argparse::copy_file(out_filename.c_str(), in_filename.c_str());
                unlink(out_filename.c_str());
            }

            // Remove the -rose:openmp:ast_only option as
            // the target compiler will not likely understand this flag.
            name_list_t::iterator it = std::find(arguments.begin(),
                    arguments.end(), "-rose:openmp:ast_only");
            if (it != arguments.end()) {
                arguments.erase(it);
            }

            execute_command(arguments);
        }
    } catch (rose_exception e) {
        // The frontend() call will print messages to the console,
        // hence no other error message needs to be printed.
        return -6;
    }

    return 0;
}
}   // extern "C"
