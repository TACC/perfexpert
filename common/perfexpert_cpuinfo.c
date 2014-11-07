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

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>

#include "common/perfexpert_fake_globals.h"
#include "common/perfexpert_alloc.h"
#include "common/perfexpert_constants.h"
#include "common/perfexpert_output.h"

static int family;
static int model;

/* perfexpert_cpuinfo */
static int perfexpert_cpuinfo(void) {
    FILE *cpuinfo = NULL;
    char line[1024];

    if (NULL == (cpuinfo = fopen("/proc/cpuinfo", "r"))) {
      OUTPUT(("%s", _ERROR("unable to open /proc/cpuinfo")));
      return PERFEXPERT_ERROR;
    }

    while (0 != fgets(line, 1024, cpuinfo)) {
        if (0 == strncmp(line, "cpu family", sizeof("cpu family") - 1)) {
            sscanf(line, "cpu family\t: %d", &family);
            continue;
        }
        if (0 == strncmp(line, "model", sizeof("model") - 1)) {
            sscanf(line, "model\t\t: %d", &model);
            continue;
        }
    }

    fclose(cpuinfo);

    OUTPUT_VERBOSE((4, "CPUINFO Processor family: [%d]", family));
    OUTPUT_VERBOSE((4, "CPUINFO Processor model:  [%d]", model));

    return PERFEXPERT_SUCCESS;
}

/* perfexpert_cpuinfo_get_model */
int perfexpert_cpuinfo_get_model(void) {
    if (0 != model) {
      return model;
    }

    if (PERFEXPERT_SUCCESS != perfexpert_cpuinfo()) {
        OUTPUT(("%s", _ERROR("unable to get CPU info")));
        return -1;
    }

    return model;
}

/* perfexpert_cpuinfo_get_family */
int perfexpert_cpuinfo_get_family(void) {
    if (0 != family) {
      return family;
    }

    if (PERFEXPERT_SUCCESS != perfexpert_cpuinfo()) {
        OUTPUT(("%s", _ERROR("unable to get CPU info")));
        return -1;
    }

    return family;
}

/* perfexpert_cpuinfo_set_model */
int perfexpert_cpuinfo_set_model(int value) {
    model = value;

    OUTPUT_VERBOSE((4, "CPUINFO Processor model:  [%d]", model));

    return PERFEXPERT_SUCCESS;
}

/* perfexpert_cpuinfo_set_family */
int perfexpert_cpuinfo_set_family(int value) {
    family = value;

    OUTPUT_VERBOSE((4, "CPUINFO Processor family: [%d]", family));

    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif
