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

#ifndef CT_DATABASE_H_
#define CT_DATABASE_H_

#ifdef __cplusplus
extern "C" {
#endif

static int accumulate_transformations(void *recommendation, int c, char **val,
    char **names);
static int accumulate_patterns(void *transformation, int c, char **val,
    char **names);

#ifdef __cplusplus
}
#endif

#endif /* CT_DATABASE_H_ */
