/*
 * Copyright (c) 2011-2016  University of Texas at Austin. All rights reserved.
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
 * Authors: Antonio Gomez-Iglesias
 *
 * $HEADER$
 */

#ifndef PERFEXPERT_MODULE_MACPO_TYPES_H_
#define PERFEXPERT_MODULE_MACPO_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "common/perfexpert_constants.h"

typedef struct {
    char *file;
    char *backfile;
    char *names[MAX_COLLECTION];
    long locations[MAX_COLLECTION];
    int maxlocations;
} fileInsts;

typedef struct {
    fileInsts list[MAX_COLLECTION];
    int maxfiles;
} st_insts;
#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_MODULE_MACPO_TYPES_H_ */
