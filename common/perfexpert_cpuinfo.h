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

/* Function declarations */
static int perfexpert_cpuinfo(void);
int perfexpert_cpuinfo_get_model(void);
int perfexpert_cpuinfo_get_family(void);
int perfexpert_cpuinfo_set_model(int value);
int perfexpert_cpuinfo_set_family(int value);

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_CPUINFO_H */
