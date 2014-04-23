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

#ifndef PERFEXPERT_CPUINFO_H_
#define PERFEXPERT_CPUINFO_H_

#ifndef _STDLIB_H
#include <stdlib.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Intel CPUs models */
typedef enum {
    NEHALEM_EP      = 26,
    NEHALEM         = 30,
    NEHALEM_2       = 31,
    CLARKDALE       = 37,
    WESTMERE_EP     = 44,
    NEHALEM_EX      = 46,
    WESTMERE_EX     = 47,
    SANDY_BRIDGE    = 42,
    SANDY_BRIDGE_EP = 45,
    IVY_BRIDGE      = 58,
    IVYTOWN         = 62,
    HASWELL         = 60,
    HASWELL_ULT     = 69,
    HASWELL_2       = 70,
} intel_models_t;

/* Function declarations */
static int perfexpert_cpuinfo(void);
int perfexpert_cpuinfo_get_model(void);
int perfexpert_cpuinfo_get_family(void);

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_CPUINFO_H */
