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
    globals.verbose_level = 10;
    globals.colorful = 1;
    perfexpert_list_construct(&(globals.profiles));

    /* Parse input file, check it, and flatten profiles */
    if (PERFEXPERT_SUCCESS != hpctoolkit_parse_file(argv[1],
        &(globals.profiles))) {
        OUTPUT(("%s (%s)",
            _ERROR("Error: are you sure this is a valid HPCToolkit file?"),
            argv[1]));
        return PERFEXPERT_ERROR;
    }

    if (PERFEXPERT_SUCCESS != profile_check_all(&(globals.profiles))) {
        OUTPUT(("%s (%s)",
            _ERROR("Error: are you sure this is a valid HPCToolkit file?"),
            argv[1]));
        return PERFEXPERT_ERROR;
    }

    // if (PERFEXPERT_SUCCESS != profile_flatten_all(&profiles)) {
    //     OUTPUT(("%s (%s)",
    //         _ERROR("Error: are you sure this is a valid HPCToolkit file?"),
    //         argv[1]));
    //     return PERFEXPERT_ERROR;
    // }

    return 0;
}

// EOF
