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

/* Module headers */
#include "make_module.h"
#include "make.h"

/* PerfExpert common headers */
#include "common/perfexpert_output.h"
#include "common/perfexpert_alloc.h"
#include "common/perfexpert_constants.h"

/* Global variable to define the module itself */
perfexpert_module_make_t myself_module;
// my_module_globals_t my_module_globals;
char module_version[] = "1.0.0";

/* module_load */
int module_load(void) {
    /* Disable extended interface */
    myself_module.set_target = NULL;
    myself_module.set_env    = NULL;
    myself_module.unset_env  = NULL;
    myself_module.get_env    = NULL;

    OUTPUT_VERBOSE((5, "%s", _MAGENTA("loaded")));

    return PERFEXPERT_SUCCESS;
}

/* module_init */
int module_init(void) {
    int i = 0;

    /* Enable extended interface */
    myself_module.set_target = &module_set_target;
    myself_module.set_env    = &module_set_env;
    myself_module.unset_env  = &module_unset_env;
    myself_module.get_env    = &module_get_env;

    OUTPUT_VERBOSE((5, "%s", _MAGENTA("initialized")));

    return PERFEXPERT_SUCCESS;
}

/* module_fini */
int module_fini(void) {
    int i = 0;

    /* Disable extended interface */
    myself_module.set_target = NULL;
    myself_module.set_env    = NULL;
    myself_module.unset_env  = NULL;
    myself_module.get_env    = NULL;

    OUTPUT_VERBOSE((5, "%s", _MAGENTA("finalized")));

    return PERFEXPERT_SUCCESS;
}

/* module_compile */
int module_compile(void) {
    OUTPUT(("%s [%s]", _YELLOW("Compiling"), globals.program));

    if (PERFEXPERT_SUCCESS != run_make()) {
        OUTPUT(("%s", _ERROR("error running 'make'")));
        return PERFEXPERT_ERROR;
    }

    return PERFEXPERT_SUCCESS;
}

/* module_set_target */
int module_set_target(const char *target) {
    return PERFEXPERT_SUCCESS;
}

/* module_set_env */
int module_set_env(const char *name, const char *value) {
    return PERFEXPERT_SUCCESS;
}

/* module_unset_env */
int module_unset_env(const char *name) {
    return PERFEXPERT_SUCCESS;
}

/* module_get_env */
char* module_get_env(const char *name) {
    return NULL;
}

#ifdef __cplusplus
}
#endif

// EOF
