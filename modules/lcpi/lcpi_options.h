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
    { 0, 0, 0, 0, "\n[LCPI module options]", 1 },

    { "architecture=VALUE", 0, 0, OPTION_DOC, "Set the architecture which the "
      "LCPI module should generate metrics for (Valid values: Bonnell, "
      "Broadwell, Haswell, IvyBridge, IvyTown, Jaketown, NehalemEP, NehalemEX, "
      "SandyBridge, Silvermont, WestmereEP-DP, WestmereEP-SP, WestmereEX, and "
      "KnightsCorner or MIC" },
    { "mic", 0, 0, OPTION_DOC, "Generate MIC-compatible LCPI metrics" },
    { "sort-order=VALUE", 0, 0, OPTION_DOC,
      "Order in which hotspots should be sorted (Valid values: relevance, "
      "performance, mixed. Default value: unsorted)" },
    { "verbose=VALUE", 0, 0, OPTION_DOC, "Enable verbose mode (range: 0-10)" },
    { "output=MODE", 0, 0 , OPTION_DOC, "Output for serial/aggregated mode (0), for MPI tasks (1), for MPI and threads (2)"}, 
    { "mic", 'm', 0, OPTION_HIDDEN, 0 },
    { "sort-order", 's', "relevance|performance|mixed", OPTION_HIDDEN, 0 },
    { "architecture", 'a', "Bonnell|Broadwell|Haswell|IvyBridge|IvyTown|"
      "Jaketown|NehalemEP|NehalemEX|SandyBridge|Silvermont|WestmereEP-DP|"
      "WestmereEP-SP|WestmereEX|KnightsCorner (or MIC)", OPTION_HIDDEN, 0 },
    { "verbose", 'v', "VALUE", OPTION_HIDDEN, 0 },
    { "output", 'o', "VALUE", OPTION_HIDDEN, 0},

    { 0 }
};

/* Function declarations */
static int parse_env_vars(void);
static error_t parse_options(int key, char *arg, struct argp_state *state);

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_MODULE_LCPI_OPTIONS_H_ */
