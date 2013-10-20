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

#ifndef ANALYZER_PROFILE_H_
#define ANALYZER_PROFILE_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _STDLIB_H
#include <stdlib.h>
#endif

/* Functions declarations */
static int profile_aggregate_hotspots(profile_t *profile);
static int profile_aggregate_metrics(profile_t *profile, procedure_t *hotspot);
static int profile_flatten_hotspots(profile_t *profile);
static int profile_check_callpath(perfexpert_list_t *calls, int root);

#ifdef __cplusplus
}
#endif

#endif /* ANALYZER_PROFILE_H_ */