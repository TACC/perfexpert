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

#ifndef PERFEXPERT_ALLOC_H_
#define PERFEXPERT_ALLOC_H_

#ifndef _STDLIB_H
#include <stdlib.h>
#endif

#include "perfexpert_output.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PERFEXPERT_ALLOC(type, ptr, size)                                      \
    ptr = (type *)malloc(size);                                                \
    if (NULL == ptr) {                                                         \
        OUTPUT(("%s (%s:%d) (%s)", _ERROR("Error: unable to allocate memory"), \
            __FILE__, __LINE__, __FUNCTION__));                                \
        return PERFEXPERT_ERROR;                                               \
    }                                                                          \
    bzero(ptr, size)

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_ALLOC_H */
