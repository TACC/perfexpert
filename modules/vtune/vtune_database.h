/*
 * Copyright (c) 2011-2015  University of Texas at Austin. All rights reserved.
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
 * Authors: Antonio Gomez-Iglesias, Leonardo Fialho and Ashay Rane
 *
 * $HEADER$
 */

#ifndef PERFEXPERT_MODULE_VTUNE_DATABASE_H_
#define PERFEXPERT_MODULE_VTUNE_DATABASE_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Modules headers */
#include "vtune_types.h"

/* Function declarations */
static int database_hw_events(vtune_hw_profile_t *profile);
//static int database_metrics(hpctoolkit_procedure_t *hotspot);

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_MODULE_VTUNE_DATABASE_H_ */
