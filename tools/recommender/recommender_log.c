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

#ifdef __cplusplus
extern "C" {
#endif

/* System standard headers */
#include <string.h>

/* PerfExpert headers */
#include "recommender.h"
#include "perfexpert_log.h"

/* log_bottleneck */
int log_bottleneck(void *var, int count, char **val, char **names) {
    char temp_str[MAX_LOG_ENTRY];
    int  i;

    bzero(temp_str, MAX_LOG_ENTRY);
    for (i = 0; i < count; i++) {
        if (NULL != val[i]) {
            strcat(temp_str, names[i]);
            strcat(temp_str, "=");
            strcat(temp_str, val[i]);
            strcat(temp_str, ";");
        }
    }
    LOG(("%s", temp_str));

    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
