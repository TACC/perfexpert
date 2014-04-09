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

#ifndef PERFEXPERT_VTUNE_H_
#define PERFEXPERT_VTUNE_H_

#ifdef __cplusplus
extern "C" {
#endif

#define VTUNE_EXPERIMENT_FILE     "experiment_vtune.conf"
#define VTUNE_MIC_EXPERIMENT_FILE "experiment_vtune_mic.conf"
#define VTUNE_PROFILE_FILE        "database/experiment.xml"

/* HPCToolkit stuff (binaries should be in the path) */
#define VTUNE_AMPLIFIER "amplxe-cl"

/* Function declarations */
int measurements_vtune(void);

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_VTUNE_H_ */
