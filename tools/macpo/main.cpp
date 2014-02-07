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

#include "argparse.h"
#include "minst.h"

int main (int argc, char *argv[]) {
    options_t options;
    argparse::init_options(options);

    std::vector<std::string> arguments;
    arguments.push_back(argv[0]);

    for (int i=1; i<argc; i++) {
        if (strstr(argv[i], "--macpo:") == argv[i]) { // If "--macpo:" was found at the start of argv[i]
            if (argparse::parse_arguments(argv[i], options) < 0) {
                fprintf (stdout, "Unknown parameter: %s, aborting...\n", argv[i]);
                return -1;
            }
        } else {
            arguments.push_back(argv[i]);
        }
    }

    // Check if we have explicit instructions not to change anything
    if (options.action != ACTION_NONE) {
        // Check if at least a function or a loop was specified on the command line
        if (options.function_name.size() == 0) {
            std::cerr << mprefix << "USAGE: " << argv[0] << " <options>" <<
                std::endl;
            std::cerr << mprefix << "Did not find valid options on the command "
                << "line" << std::endl;
            return -1;
        }

        SgProject *project = frontend (arguments);
        ROSE_ASSERT (project != NULL);

        SgFilePtrList files = project->get_fileList();

        if (options.backup_filename.size()) {
            // We need to save the input file to a backup file.
            if (files.size() != 1) {
                std::cerr << mprefix << "Backup option can be specified with "
                    << "only a single file for compilation, terminating." <<
                    std::endl;
                return -1;
            }

            SgSourceFile* file = isSgSourceFile(*(files.begin()));
            std::string source = file->get_file_info()->get_filenameString();

            // Copy the file over.
            if (argparse::copy_file(source.c_str(),
                        options.backup_filename.c_str()) < 0) {
                std::cerr << mprefix << "Error backing up file." << std::endl;
                return -1;
            }

            std::cerr << mprefix << "Saved " << source << " into " <<
                    options.backup_filename << "." << std::endl;
        }

        VariableRenaming var_renaming(project);
        if (options.action == ACTION_ALIGNCHECK) {
            // If we are about to check alignment, run the VariableRenaming pass.
            var_renaming.run();
        }

        // Loop over each file
        for (SgFilePtrList::iterator it=files.begin(); it!=files.end(); it++) {
            SgSourceFile* file = isSgSourceFile(*it);
            std::string filename = file->get_file_info()->get_filenameString();
            std::string basename = filename.substr(filename.find_last_of("/"));

            // Start the traversal!
            MINST traversal (options.action, options.line_number,
                    options.function_name, options.profile_analysis,
                    &var_renaming);
            traversal.traverseWithinFile (file, preorder);
        }

        if (!options.no_compile) {
            // FIXME: ROSE tests seem to be broken, operand of AddressOfOp is not allowed to be an l-value
            // AstTests::runAllTests (project);
            return backend (project) == 0 ? 0 : 1;
        } else {
            project->unparse();
            return 0;
        }
    } else {
        SgProject *project = frontend (arguments);
        if (!options.no_compile) {
            return backend (project) == 0 ? 0 : 1;
        } else {
            project->unparse();
            return 0;
        }
    }
}
