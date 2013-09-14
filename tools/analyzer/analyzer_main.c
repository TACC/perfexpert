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
#include "perfexpert_hash.h"
#include "perfexpert_list.h"
#include "perfexpert_md5.h"
#include "perfexpert_output.h"
#include "perfexpert_util.h"

/* Global variables, try to not create them! */
globals_t globals; // Variable to hold global options, this one is OK

/* main, life starts here */
int main(int argc, char **argv) {
    perfexpert_list_t profiles;

    globals.verbose_level = 10;
    globals.colorful = 1;

    perfexpert_list_construct(&profiles);
    hpctoolkit_parse_file(argv[1], &profiles);

    // OK, now we have to test the parser
    check_profiles(&profiles);

    return 0;
}

int check_profiles(perfexpert_list_t *profiles) {
    profile_t *profile = NULL;

    OUTPUT_VERBOSE((5, "%s", _YELLOW("Checking profiles")));

    profile = (profile_t *)perfexpert_list_get_first(profiles);
    while ((perfexpert_list_item_t *)profile != &(profiles->sentinel)) {
        OUTPUT_VERBOSE((10, "   %s", profile->name));
        if (0 < perfexpert_list_get_size(&(profile->callees))) {
           check_callpath(&(profile->callees), 0);
        }
        profile = (profile_t *)perfexpert_list_get_next(profile);
    }

    return PERFEXPERT_SUCCESS;
}

int check_callpath(perfexpert_list_t *calls, int root) {
    callpath_t *callpath;
    char *indent;
    int i;

    indent = (char *)malloc((2 * root) + 1);
    if (NULL == indent) {
        OUTPUT(("%s", _ERROR("Error: out of memory")));
        return PERFEXPERT_ERROR;
    }
    bzero(indent, (2 * root) + 1);
    for (i = 0; i < root; i++) {
        strcat(indent, "  ");
    }

    callpath = (callpath_t *)perfexpert_list_get_first(calls);
    while ((perfexpert_list_item_t *)callpath != &(calls->sentinel)) {
        char *filename = NULL;
        metric_t *metric;

        if (PERFEXPERT_SUCCESS != perfexpert_util_filename_only(
            callpath->file->name, &filename)) {
            OUTPUT(("%s", _ERROR("Error: unable to extract short filename")));
            return PERFEXPERT_ERROR;
        }

        perfexpert_hash_find_str(callpath->metrics_by_name,
            perfexpert_md5_string("PAPI_TOT_INS"), metric);

        if (PERFEXPERT_FUNCTION == callpath->type) {
            OUTPUT_VERBOSE((5, "%s [%d] %s (%s:%d) [PAPI_TOT_INS=%g]", indent,
                callpath->id, _RED(callpath->procedure->name), filename,
                callpath->line, metric ? metric->value : 0));
        } else if (PERFEXPERT_LOOP == callpath->type) {
            OUTPUT_VERBOSE((5, "%s [%d] %s (%s:%d)  [PAPI_TOT_INS=%g]", indent,
                callpath->id, _GREEN("loop"), filename, callpath->line,
                metric ? metric->value : 0));
        } else {
            OUTPUT_VERBOSE((5, "%s", _ERROR("WTF???")));
        }

        if (0 < perfexpert_list_get_size(&(callpath->callees))) {
            root++;
            check_callpath(&(callpath->callees), root);
            root--;
        }
        callpath = (callpath_t *)perfexpert_list_get_next(callpath);
    }

    return PERFEXPERT_SUCCESS;
}

// EOF
