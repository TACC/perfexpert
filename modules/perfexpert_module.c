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

/* System standard headers */
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <ltdl.h>

/* Fake globals, just to compile the library */
typedef struct {
    int      verbose;
    int      colorful; // These should be the first variables in the structure
} globals_t;

extern globals_t globals; /* This variable is defined in analyzer_main.c */

#ifdef PROGRAM_PREFIX
#undef PROGRAM_PREFIX
#endif
#define PROGRAM_PREFIX "[perfexpert_module]"

/* PerfExpert headers */
#include "perfexpert_module.h"
#include "perfexpert_alloc.h"
#include "perfexpert_output.h"
#include "install_dirs.h"

/* perfexpert_load_module */
int perfexpert_load_module(const char *toolname, perfexpert_module_t *module) {
    static const lt_dlinfo *moduleinfo = NULL;
    lt_dlhandle modulehandle = NULL;
    char *filename = NULL;

    /* Sanity check #0: is the module already loaded? */
    if (NULL != moduleinfo) {
        if (0 < moduleinfo->ref_count) {
            OUTPUT_VERBOSE((10, "   module already loaded"));
            return PERFEXPERT_SUCCESS;
        }        
    }

    OUTPUT_VERBOSE((9, "loading module [%s]", toolname));

    /* Initialize LIBTOOL */
    if (0 != lt_dlinit()) {
        OUTPUT(("%s [%s]", _ERROR("Error: unable to initialize libtool"),
            lt_dlerror()));
        return PERFEXPERT_ERROR;
    }
    if (0 != lt_dlsetsearchpath(PERFEXPERT_LIBDIR)) {
        OUTPUT(("%s [%s]", _ERROR("Error: unable to set libtool search path"),
            lt_dlerror()));
        return PERFEXPERT_ERROR;
    }

    /* Sanity check #1: load only symlinks */
    PERFEXPERT_ALLOC(char, filename, (strlen(toolname) + 25));
    filename = (char *)malloc(strlen(toolname) + 25);
    sprintf(filename, "libperfexpert_module_%s.so", toolname);

    /* Load module file */
    /* WARNING: if this function returns "file not found" sometime, the problem
     *          is not with the search path or with the instalation. This error
     * (which is incorrect) could be returned by the LIBTOOL when the module
     * have unresolved symbols. To solve this problem you should check if there
     * is any unresolved symbol in the module file (use 'nm' command to check).
     */
    modulehandle = lt_dlopenext(filename);

    if (NULL == modulehandle) {
        OUTPUT(("%s [%s] [%s]", _ERROR("Error: cannot load module"), filename,
            lt_dlerror()));
        goto MODULE_ERROR;
    }
    OUTPUT_VERBOSE((9, "module %s loaded", _CYAN(filename)));

    /* Reading module info */
    moduleinfo = lt_dlgetinfo(modulehandle);
    
    if (NULL == moduleinfo) {
        OUTPUT(("cannot get module info: %s", lt_dlerror()));
        goto MODULE_ERROR;
    }
    OUTPUT_VERBOSE((10, "   name: %s", moduleinfo->name));
    OUTPUT_VERBOSE((10, "   filename: %s", moduleinfo->filename));
    OUTPUT_VERBOSE((10, "   reference count: %i", moduleinfo->ref_count));

    /* Sanity check #2: avoid to load the same module twice */
    if (1 < moduleinfo->ref_count) {
        OUTPUT(("   module %s already loaded", moduleinfo->name));
        goto MODULE_ERROR;
    }

    /* Load module interface */
    module->measurements = lt_dlsym(modulehandle,
        "perfexpert_tool_measurements");
    if (NULL == module->measurements) {
        OUTPUT(("%s",
            _ERROR("Error: perfexpert_tool_measurements() not found")));
        goto MODULE_ERROR;       
    }
    module->parse_file = lt_dlsym(modulehandle, "perfexpert_tool_parse_file");
    if (NULL == module->parse_file) {
        OUTPUT(("%s", _ERROR("Error: perfexpert_tool_parse_file() not found")));
        goto MODULE_ERROR;       
    }
    module->profile_file = lt_dlsym(modulehandle,
        "perfexpert_tool_profile_file");
    if (NULL == module->profile_file) {
        OUTPUT(("%s", _ERROR("Error: perfexpert_tool_profile_file not found")));
        goto MODULE_ERROR;       
    }
    module->total_instructions = lt_dlsym(modulehandle,
        "perfexpert_tool_total_instructions");
    if (NULL == module->total_instructions) {
        OUTPUT(("%s",
            _ERROR("Error: perfexpert_tool_total_instructions not found")));
        goto MODULE_ERROR;       
    }
    module->total_cycles = lt_dlsym(modulehandle,
        "perfexpert_tool_total_cycles");
    if (NULL == module->total_cycles) {
        OUTPUT(("%s",
            _ERROR("Error: perfexpert_tool_total_cycles not found")));
        goto MODULE_ERROR;
    }
    module->counter_prefix = lt_dlsym(modulehandle,
        "perfexpert_tool_counter_prefix");
    if (NULL == module->counter_prefix) {
        OUTPUT(("%s",
            _ERROR("Error: perfexpert_tool_counter_prefix not found")));
        goto MODULE_ERROR;       
    }

    OUTPUT_VERBOSE((9, "module loaded [%s]", filename));

    PERFEXPERT_DEALLOC(filename);
    return PERFEXPERT_SUCCESS;

    MODULE_ERROR:
    lt_dlclose(modulehandle);
    PERFEXPERT_DEALLOC(filename);
    return PERFEXPERT_ERROR;
}

// EOF
