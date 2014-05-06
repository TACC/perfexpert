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
#include <VariableRenaming.h>

#include <string>
#include <vector>

#include "argparse.h"
#include "minst.h"

int main(int argc, char *argv[]) {
    options_t options;
    argparse::init_options(options);

    std::vector<std::string> arguments;
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

        if (!options.no_compile) {
            // FIXME: ROSE tests seem to be broken,
            // operand of AddressOfOp is not allowed to be an l-value.
            // AstTests::runAllTests(project);
            return backend(project) == 0 ? 0 : 1;
        }

        project->unparse();
    } catch (rose_exception e) {
        // The frontend() call will print messages to the console,
        // hence no other error message needs to be printed.
        return -1;
    }

    return 0;
}
