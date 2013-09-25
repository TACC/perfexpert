/*
 * Copyright (c) 2013  University of Texas at Austin. All rights reserved.
 *
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
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
 * Author: Leonardo Fialho
 *
 * $HEADER$
 */

#ifndef ANALYZER_PROFILE_H_
#define ANALYZER_PROFILE_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _STDLIB_H
#include <stdlib.h>
#endif

#include "analyzer_hpctoolkit.h"
#include "analyzer_vtune.h"

/* Basic profile file parser function type */
typedef int (*tool_parse_fn_t)(const char *file, perfexpert_list_t *profiles);

/* Structure to hold tools */
typedef struct tool {
    const char *name;
    tool_parse_fn_t function;
    const char *tot_ins;
    const char *tot_cyc;
    const char *prefix;
} tool_t;

/* Tools definition */
extern tool_t tools[]; /* This variable is defined in analyzer_profile.c */

/* perfexpert_tool_get_tot_cyc */
static inline char* perfexpert_tool_get_tot_cyc(const char* tool) {
    int i = 0;

    while (NULL != tools[i].name) {
        if (0 == strcmp(tool, tools[i].name)) {
            return (char *)(tools[i].tot_cyc);
        }
        i++;
    }
    return NULL;
}

/* perfexpert_tool_get_tot_ins */
static inline char* perfexpert_tool_get_tot_ins(const char* tool) {
    int i = 0;

    while (NULL != tools[i].name) {
        if (0 == strcmp(tool, tools[i].name)) {
            return (char *)(tools[i].tot_ins);
        }
        i++;
    }
    return NULL;
}

/* perfexpert_tool_get_prefix */
static inline char* perfexpert_tool_get_prefix(const char* tool) {
    int i = 0;

    while (NULL != tools[i].name) {
        if (0 == strcmp(tool, tools[i].name)) {
            return (char *)(tools[i].prefix);
        }
        i++;
    }
    return NULL;
}

#ifdef __cplusplus
}
#endif

#endif /* ANALYZER_PROFILE_H_ */
