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
 * Authors: Antonio Gomez-Iglesias, Leonardo Fialho and Ashay Rane
 *
 * $HEADER$
 */

#ifndef PERFEXPERT_MODULE_LCPI_DATABASE_H_
#define PERFEXPERT_MODULE_LCPI_DATABASE_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Modules headers */
#include "lcpi_types.h"

/* PerfExpert common headers */
#include "common/perfexpert_list.h"

/* Function declarations */
static int select_profiles(perfexpert_list_t *profiles, const char *table);
static int import_profiles(void *profiles, int n, char **val, char **names);
static int select_modules(lcpi_profile_t *profile, const char *table);
static int import_modules(void *profile, int n, char **val, char **names);
static int select_hotspots(perfexpert_list_t *hotspots, const char *table);
static int import_hotspots(void *hotspots, int n, char **val, char **names);
static int map_modules_to_hotspots(lcpi_hotspot_t *h, lcpi_module_t *m,
    const char *table);
static int calculate_metadata(lcpi_profile_t *profile, const char *table);
static int import_instructions(void *hotspot, int n, char **val, char **names);
static int import_experiment(void *hotspot, int n, char **val, char **names);

int import_hound(lcpi_hound_t *);
#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_MODULE_LCPI_DATABASE_H_ */
