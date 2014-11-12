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

#ifndef PERFEXPERT_MODULE_LCPI_OPTIONS_H_
#define PERFEXPERT_MODULE_LCPI_OPTIONS_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _ARGP_H
#include <argp.h>
#endif

/* PerfExpert common headers */
#include "config.h"

/* Structure to handle command line arguments */
static struct argp_option options[] = {
    { 0, 0, 0, 0, "\n[MACVEC module options]", 1 },

    { "threshold=VALUE", 0, 0, OPTION_DOC, "Threshold (relevance % of "
      "runtime) to take hotspots into consideration (range: fractional number "
      "greater than 0 and smaller than 1)" },
    { "verbose=VALUE", 0, 0, OPTION_DOC, "Enable verbose mode (range: 0-10)" },

    { "threshold", 't', "VALUE", OPTION_HIDDEN, 0 },
    { "architecture", 'a', "Bonnell|Broadwell|Haswell|IvyBridge|IvyTown|"
      "Jaketown|NehalemEP|NehalemEX|SandyBridge|Silvermont|WestmereEP-DP|"
      "WestmereEP-SP|WestmereEX|KnightsCorner (or MIC)", OPTION_HIDDEN, 0 },
    { "verbose", 'v', "VALUE", OPTION_HIDDEN, 0 },

    { 0 }
};

/* Function declarations */
static int parse_env_vars(void);
static error_t parse_options(int key, char *arg, struct argp_state *state);

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_MODULE_LCPI_OPTIONS_H_ */
