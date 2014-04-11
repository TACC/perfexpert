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

bool midend(SgProject* project, options_t& options) {
    SgFilePtrList files = project->get_fileList();
    if (options.backup_filename.size()) {
        // We need to save the input file to a backup file.
        if (files.size() != 1) {
            std::cerr << mprefix << "Backup option can be specified with only "
                << "a single file for compilation, terminating." << std::endl;
            return false;
        }

        SgSourceFile* file = isSgSourceFile(*(files.begin()));
        std::string source = file->get_file_info()->get_filenameString();

        // Copy the file over.
        if (argparse::copy_file(source.c_str(),
                options.backup_filename.c_str()) < 0) {
            std::cerr << mprefix << "Error backing up file." << std::endl;
            return false;
        }

        std::cerr << mprefix << "Saved " << source << " into " <<
            options.backup_filename << "." << std::endl;
    }

    // Loop over each file
    for (SgFilePtrList::iterator it = files.begin(); it != files.end(); it++) {
        SgSourceFile* file = isSgSourceFile(*it);

        // Start the traversal!
        MINST traversal(options, project);
        traversal.traverseWithinFile(file, preorder);
    }

    return true;
}

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

    if (argparse::validate_options(options) == false) {
        std::cerr << mprefix<< "Invalid actions specified on the command line."
            << std::endl;
        return -1;
    }

    // Check if at least a function or a loop was specified on the command line.
    if (options.action != ACTION_NONE && options.function_name.size() == 0) {
        std::cerr << mprefix << "USAGE: " << argv[0] << " <options>" <<
            std::endl;
        std::cerr << mprefix << "Did not find valid options on the command line"
            << std::endl;
        return -1;
    }

    try {
        SgProject *project = project = frontend(arguments);
        ROSE_ASSERT(project != NULL);

        // Check if we really need to invoke the instrumentation.
        if (options.action != ACTION_NONE) {
            if (midend(project, options) == false) {
                return -1;
            }
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
