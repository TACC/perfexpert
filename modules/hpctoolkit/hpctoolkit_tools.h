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

#ifndef PERFEXPERT_MODULE_HPCTOOLKIT_TOOLS_H_
#define PERFEXPERT_MODULE_HPCTOOLKIT_TOOLS_H_

/* Constants */
#define MIC_EVENTS_PER_RUN 2

#ifdef __cplusplus
extern "C" {
#endif

/* PerfExpert common headers */
#include "common/perfexpert_fork.h"

/* Private module types */
typedef struct experiment {
    volatile perfexpert_list_item_t *next;
    volatile perfexpert_list_item_t *prev;
    test_t test;
    char *argv[MAX_ARGUMENTS_COUNT];
    int  argc;
} experiment_t;

/* Function declarations */
int run_hpcstruct(void);
int run_hpcrun(void);
int run_hpcrun_mic(void);
int run_hpcprof(char **file);

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_MODULE_HPCTOOLKIT_TOOLS_H_ */
