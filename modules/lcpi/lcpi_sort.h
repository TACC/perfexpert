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

#ifndef PERFEXPERT_MODULE_LCPI_SORT_H_
#define PERFEXPERT_MODULE_LCPI_SORT_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Tools headers */
#include "tools/perfexpert/perfexpert_types.h"

/* Modules headers */
#include "lcpi_types.h"
#include "modules/hpctoolkit/hpctoolkit_module.h"

/* PerfExpert common headers */
#include "common/perfexpert_hash.h"
#include "common/perfexpert_list.h"

/* Module types */
typedef int (*sort_fn_t)(lcpi_profile_t *profile);

typedef struct sort {
    const char *name;
    sort_fn_t function;
} sort_t;

/* Function declarations */
static int sort_by_relevance(lcpi_profile_t *profile);
static int sort_by_performance(lcpi_profile_t *profile);
static int sort_by_mixed(lcpi_profile_t *profile);

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_MODULE_LCPI_SORT_H_ */
