/*
 * Copyright (C) 2013 The University of Texas at Austin
 *
 * This file is part of PerfExpert.
 *
 * PerfExpert is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * PerfExpert is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with PerfExpert. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Ashay Rane and Leonardo Fialho
 */

/* System standard headers */

/* PerfExpert headers */
#include "analyzer.h" 
#include "perfexpert_constants.h"
#include "perfexpert_list.h"
#include "perfexpert_output.h"

/* Global variables, try to not create them! */
globals_t globals; // Variable to hold global options, this one is OK

/* main, life starts here */
int main(int argc, char **argv) {
    perfexpert_list_t profiles;

    /* Set default values for globals */
    globals = (globals_t) {
        .tool          = NULL, // char *
        .aggregate     = 0,    // int
        .thread        = -1,   // int
        .verbose_level = 0,    // int
        .inputfile     = NULL, // char *
        .colorful      = 0     // int
    };

    perfexpert_list_construct(&profiles);

    /* Parse command-line parameters */
    if (PERFEXPERT_SUCCESS != parse_cli_params(argc, argv)) {
        OUTPUT(("%s", _ERROR("Error: parsing command line arguments")));
        return PERFEXPERT_ERROR;
    }

    /* Parse input file and check, flatten, and validate profiles */
    if (PERFEXPERT_SUCCESS != hpctoolkit_parse_file(globals.inputfile,
        &profiles)) {
        OUTPUT(("%s (%s)", _ERROR("Error: it is not a valid HPCToolkit file"),
            globals.inputfile));
        return PERFEXPERT_ERROR;
    }
    if (PERFEXPERT_SUCCESS != profile_check_all(&profiles)) {
        OUTPUT(("%s", _ERROR("Error: checking profile")));
        return PERFEXPERT_ERROR;
    }
    if (PERFEXPERT_SUCCESS != profile_flatten_all(&profiles)) {
        OUTPUT(("%s (%s)", _ERROR("Error: flatening profiles"),
            globals.inputfile));
        return PERFEXPERT_ERROR;
    }

    // TODO: hash metrics by name
    // TODO: parse LCPI metrics
    // TODO: parse machine.properties
    // TODO: output analysis
    // TODO: output metrics to recommender


    return 0;
}

// EOF
