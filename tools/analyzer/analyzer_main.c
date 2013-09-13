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
// #include "perfexpert_constants.h"
// #include "uthash.h"
// #include "perfexpert_hash.h"
// #include "perfexpert_output.h"
// #include "perfexpert_util.h"

// TODO: Wrong file!
/* Global variables, try to not create them! */
globals_t globals; // Variable to hold global options, this one is OK

/* main, life starts here */
int main(int argc, char **argv) {
    profile_t *profiles = NULL;

    globals.verbose_level = 10;
    globals.colorful = 1;

    hpctoolkit_parse_file(argv[1], profiles);
    return 0;
}

// EOF
