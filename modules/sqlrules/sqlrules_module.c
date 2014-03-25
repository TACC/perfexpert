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

/* System standard headers */
#include <stdio.h>

/* Module headers */
#include "sqlrules_module.h"
#include "sqlrules.h"

/* Tool headers */
#include "tools/perfexpert/perfexpert_types.h"

/* PerfExpert common headers */
#include "common/perfexpert_alloc.h"
#include "common/perfexpert_constants.h"
#include "common/perfexpert_output.h"

/* Global variable to define the module itself */
perfexpert_module_sqlrules_t myself_module;
my_module_globals_t my_module_globals;
char module_version[] = "1.0.0";

/* module_load */
int module_load(void) {
    OUTPUT_VERBOSE((5, "%s", _MAGENTA("loaded")));

    return PERFEXPERT_SUCCESS;
}

/* module_init */
int module_init(void) {
    /* Module pre-requisites */
    if (PERFEXPERT_SUCCESS != perfexpert_module_requires("sqlrules",
        PERFEXPERT_PHASE_RECOMMEND, NULL, PERFEXPERT_PHASE_UNDEFINED,
        PERFEXPERT_MODULE_LAST)) {
        OUTPUT(("%s", _ERROR("unable to add module as last step")));
        return PERFEXPERT_ERROR;
    }
    if (PERFEXPERT_SUCCESS != perfexpert_module_requires("sqlrules",
        PERFEXPERT_PHASE_RECOMMEND, "lcpi", PERFEXPERT_PHASE_ANALYZE,
        PERFEXPERT_MODULE_BEFORE)) {
        OUTPUT(("%s", _ERROR("pre-required module/phase not available")));
        return PERFEXPERT_ERROR;
    }

    /* Initialize some variables */
    my_module_globals.rec_count = 3;
    my_module_globals.no_rec = PERFEXPERT_NO_REC;
    perfexpert_list_construct(&(my_module_globals.rules));

    /* Parse command-line parameters */
    if (PERFEXPERT_SUCCESS != parse_cli_params(myself_module.argc,
        myself_module.argv)) {
        OUTPUT(("%s", _ERROR("parsing command line arguments")));
        return PERFEXPERT_ERROR;
    }

    OUTPUT_VERBOSE((5, "%s", _MAGENTA("initialized")));

    return PERFEXPERT_SUCCESS;
}

/* module_fini */
int module_fini(void) {
    OUTPUT_VERBOSE((5, "%s", _MAGENTA("finalized")));

    return PERFEXPERT_SUCCESS;
}

/* module_recommend */
int module_recommend(void) {
    char *str = NULL;

    OUTPUT(("%s", _YELLOW("Selecting recommendations")));

    /* Open metrics output file */
    PERFEXPERT_ALLOC(char, str, (strlen(globals.moduledir) + 13));
    sprintf(str, "%s/metrics.txt", globals.moduledir);
    if (NULL == (my_module_globals.metrics_FP = fopen(str, "w+"))) {
        OUTPUT(("%s (%s)", _ERROR("unable to open output file"), str));
        PERFEXPERT_DEALLOC(str);
        return PERFEXPERT_ERROR;
    }
    PERFEXPERT_DEALLOC(str);

    /* Select recommendations */
    if (PERFEXPERT_SUCCESS != select_recommendations()) {
        OUTPUT(("%s", _ERROR("selecting recommendations")));
        return PERFEXPERT_ERROR;
    }

    /* Close metrics output file */
    fclose(my_module_globals.metrics_FP);

    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
