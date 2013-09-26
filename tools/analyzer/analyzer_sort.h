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

#ifndef ANALYZER_SORT_H_
#define ANALYZER_SORT_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Basic sorting function type */
typedef int (*sort_fn_t)(profile_t *profile);

/* Structure to hold sorting orders */
typedef struct sort {
    const char *name;
    sort_fn_t function;
} sort_t;

/* Function definitions */
int sort_by_relevance(profile_t *profile);
int sort_by_performance(profile_t *profile);
int sort_by_mixed(profile_t *profile);

#ifdef __cplusplus
}
#endif

#endif /* ANALYZER_SORT_H_ */
