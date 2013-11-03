/*
 * Copyright (c) 2011-2013  University of Texas at Austin. All rights reserved.
 *
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * This file is part of PerfExpert.
 *
 * PerfExpert is free software: you can redistribute it and/or modify it under
 * the terms of the The University of Texas at Austin Research License
 *
 * PerfExpert is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.
 *
 * Authors: Leonardo Fialho and Ashay Rane
 *
 * $HEADER$
 */

#ifndef CT_TOOLS_H_
#define CT_TOOLS_H_

#ifdef __cplusplus
extern "C" {
#endif

/* PerfExpert tool headers */
#include "ct.h"

static int apply_transformations(fragment_t *fragment,
    recommendation_t *recommendation);
static int apply_patterns(fragment_t *fragment, transformation_t *transformation);
static int test_transformation(fragment_t *fragment,
    transformation_t *transformation);
static int test_pattern(fragment_t *fragment, pattern_t *pattern);

#ifdef __cplusplus
}
#endif

#endif /* CT_TOOLS_H_ */
