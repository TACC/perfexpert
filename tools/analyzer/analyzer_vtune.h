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

#ifndef ANALYZER_VTUNE_H_
#define ANALYZER_VTUNE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "perfexpert_list.h"

/* VTune stuff */
#define PERFEXPERT_TOOL_VTUNE_PROFILE_FILE  "database/experiment.xml"
#define PERFEXPERT_TOOL_VTUNE_COUNTERS      "vtune"
#define PERFEXPERT_TOOL_VTUNE_TOT_INS       "PAPI_TOT_INS"
#define PERFEXPERT_TOOL_VTUNE_TOT_CYC       "PAPI_TOT_CYC"

/* Function declarations */
int vtune_parse_file(const char *file, perfexpert_list_t *profiles);

#ifdef __cplusplus
}
#endif

#endif /* ANALYZER_VTUNE_H_ */
