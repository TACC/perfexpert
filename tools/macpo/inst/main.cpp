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

#include <rose.h>

#include <string>
#include <vector>

#include "argparse.h"
#include "minst.h"

int main(int argc, char *argv[]) {
    options_t options;
    argparse::init_options(options);

    name_list_t arguments;
    arguments.push_back(argv[0]);

    for (int i = 1; i < argc; i++) {
        // Check if argv[i] starts with "--macpo:".
        if (strstr(argv[i], "--macpo:") == argv[i]) {
            if (argparse::parse_arguments(argv[i], options) < 0) {
                fprintf(stdout, "Unknown parameter: %s, aborting...\n",
                        argv[i]);
                return -1;
            }
        } else {
            arguments.push_back(argv[i]);
        }
    }

    try {
        SgProject *project = project = frontend(arguments);
        ROSE_ASSERT(project != NULL);

        if (midend(project, options) == false) {
            return -1;
        }

        if (options.no_compile == false && options.base_compiler.empty()) {
            // FIXME: ROSE tests seem to be broken,
            // operand of AddressOfOp is not allowed to be an l-value.
            // AstTests::runAllTests(project);
            return backend(project) == 0 ? 0 : 1;
        }

        project->unparse();

        if (options.base_compiler.size() > 0) {
            std::string base_compiler = get_path(options.base_compiler);
            if (base_compiler.empty() ||
                    access(base_compiler.c_str(), R_OK | X_OK) == -1) {
                std::cerr << "Failed to find a valid compiler binary: " <<
                        options.base_compiler << std::endl;
                return -1;
            }

            // Replace the macpo binary with the target compiler.
            arguments[0] = base_compiler;

            // Loop over each file, replacing the name with the modified name.
            for (int i = 0; i < project->numberOfFiles(); i++) {
                SgFile* file = (*project)[i];
                std::string in_filename = file->getFileName();
                std::string out_filename = file->get_unparse_output_filename();

                if (in_filename.size() >= 1 &&
                        in_filename[in_filename.size() - 1] == '/') {
                    // Looks like a directory, skip this name.
                    continue;
                }

                name_list_t::iterator it = std::find(arguments.begin(),
                        arguments.end(), in_filename);
                if (it == arguments.end()) {
                    size_t last_slash = in_filename.rfind('/');
                    std::string base = in_filename.substr(last_slash + 1);
                    it = std::find(arguments.begin(), arguments.end(), base);
                    if (it == arguments.end()) {
                        // Funny, this is a file being compiled but
                        // we cannot find it in the argument list.
                        assert(false && "Cannot find file in argument list!");
                    }
                }

                size_t index = std::distance(arguments.begin(), it);
                arguments[index] = out_filename;
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
        return -1;
    }

    return 0;
}
