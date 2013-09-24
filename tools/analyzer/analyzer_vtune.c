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

#ifdef __cplusplus
extern "C" {
#endif

/* System standard headers */

/* PerfExpert headers */
#include "analyzer.h" 
#include "perfexpert_list.h"
#include "perfexpert_constants.h"

/* vtune_parse_file */
int vtune_parse_file(const char *file, perfexpert_list_t *profiles) {
    return PERFEXPERT_SUCCESS;

    OUTPUT_VERBOSE((2, "   %s [VTune Amplifier XE experiment DB version ??]",
        _BLUE("Loading profiles")));
}

#ifdef __cplusplus
}
#endif

// EOF
