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

#ifndef PERFEXPERT_HPCTOOLKIT_H_
#define PERFEXPERT_HPCTOOLKIT_H_

#ifdef __cplusplus
extern "C" {
#endif

#define HPCTOOLKIT_EXPERIMENT_FILE     "experiment_hpctoolkit.conf"
#define HPCTOOLKIT_MIC_EXPERIMENT_FILE "experiment_hpctoolkit_mic.conf"
#define HPCTOOLKIT_PROFILE_FILE        "database/experiment.xml"

/* HPCToolkit stuff (binaries should be in the path) */
#define HPCSTRUCT "hpcstruct"
#define HPCRUN    "hpcrun"
#define HPCPROF   "hpcprof"

/* Function declarations */
int measurements_hpctoolkit(void);
int run_hpcstruct(void);
int run_hpcrun(void);
int run_hpcrun_knc(void);
int run_hpcprof(void);

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_HPCTOOLKIT_H_ */
