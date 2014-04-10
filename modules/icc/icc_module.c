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

/* Module headers */
#include "icc.h"
#include "icc_module.h"

/* PerfExpert common headers */
#include "common/perfexpert_output.h"
#include "common/perfexpert_alloc.h"
#include "common/perfexpert_constants.h"

/* Global variable to define the module itself */
perfexpert_module_icc_t myself_module;
my_module_globals_t my_module_globals;
char module_version[] = "1.0.0";

/* module_load */
int module_load(void) {
    /* Disable extended interface */
    myself_module.set_compiler = NULL;
    myself_module.set_source   = NULL;
    myself_module.set_flag     = NULL;
    myself_module.set_library  = NULL;
    myself_module.get_flag     = NULL;
    myself_module.get_library  = NULL;

    OUTPUT_VERBOSE((5, "%s", _MAGENTA("loaded")));

    return PERFEXPERT_SUCCESS;
}

/* module_init */
int module_init(void) {
    /* Initialize variables */
    my_module_globals.compiler  = NULL;
    my_module_globals.source    = NULL;
    my_module_globals.help_only = PERFEXPERT_FALSE;

    /* Parse module options */
    if (PERFEXPERT_SUCCESS != parse_module_args(myself_module.argc,
        myself_module.argv)) {
        OUTPUT(("%s", _ERROR("parsing module arguments")));
        return PERFEXPERT_ERROR;
    }

    /* Enable extended interface */
    myself_module.set_compiler = &module_set_compiler;
    myself_module.set_source   = &module_set_source;
    myself_module.set_flag     = &module_set_flag;
    myself_module.set_library  = &module_set_library;
    myself_module.get_flag     = &module_get_flag;
    myself_module.get_library  = &module_get_library;

    OUTPUT_VERBOSE((5, "%s", _MAGENTA("initialized")));

    return PERFEXPERT_SUCCESS;
}

/* module_fini */
int module_fini(void) {
    int i = 0;

    /* Disable extended interface */
    myself_module.set_compiler = NULL;
    myself_module.set_source   = NULL;
    myself_module.set_flag     = NULL;
    myself_module.set_library  = NULL;
    myself_module.get_flag     = NULL;
    myself_module.get_library  = NULL;

    OUTPUT_VERBOSE((5, "%s", _MAGENTA("finalized")));

    return PERFEXPERT_SUCCESS;
}

/* module_compile */
int module_compile(void) {
    OUTPUT(("%s [%s]", _YELLOW("Compiling"), globals.program));

    if (PERFEXPERT_SUCCESS != run_icc()) {
        OUTPUT(("%s", _ERROR("error running 'make'")));
        return PERFEXPERT_ERROR;
    }

    return PERFEXPERT_SUCCESS;
}

/* module_set_compiler */
int module_set_compiler(const char *compiler) {
    return PERFEXPERT_SUCCESS;
}

/* module_set_source */
int module_set_source(const char *source) {
    return PERFEXPERT_SUCCESS;
}

/* module_set_flag */
int module_set_flag(const char *flag) {
    return PERFEXPERT_SUCCESS;
}

/* module_set_library */
int module_set_library(const char *library) {
    return PERFEXPERT_SUCCESS;
}

/* module_get_flag */
char* module_get_flag(void) {
    return NULL;
}

/* module_get_library */
char* module_get_library(void) {
    return NULL;
}

#ifdef __cplusplus
}
#endif

// EOF
