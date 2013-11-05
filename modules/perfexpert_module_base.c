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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <ltdl.h>

/* Fake globals, just to compile the library */
typedef struct {
    int      verbose;
    int      colorful;
} globals_t;

extern globals_t globals; /* This variable is defined in analyzer_main.c */

#ifdef PROGRAM_PREFIX
#undef PROGRAM_PREFIX
#endif
#define PROGRAM_PREFIX "[perfexpert_module]"

/* Modules headers */
#include "perfexpert_module_base.h"

/* PerfExpert common headers */
#include "common/perfexpert_alloc.h"
#include "common/perfexpert_list.h"
#include "common/perfexpert_output.h"
#include "install_dirs.h"

/* Module global variables */
module_globals_t module_globals;

/* my_init (runs automatically in loading time) */
void __attribute__ ((constructor)) my_init(void) {
    perfexpert_list_construct(&(module_globals.modules));
    perfexpert_list_construct(&(module_globals.compile));
    perfexpert_list_construct(&(module_globals.measurements));
    perfexpert_list_construct(&(module_globals.analysis));
}

/* perfexpert_module_load*/
int perfexpert_module_load(const char *name) {
    perfexpert_ordered_module_t *ordered = NULL;
    perfexpert_module_t *module = NULL;
    lt_dlhandle modulehandle = NULL;

    /* Sanity check: is this module already loaded? */
    perfexpert_list_for(module, &(module_globals.modules), perfexpert_module_t) {
        if (0 == strcmp(name, module->name)) {
            OUTPUT_VERBOSE((8, "module already loaded"));
            return PERFEXPERT_SUCCESS;
        }
    }

    /* Load module */
    if (NULL == (modulehandle = perfexpert_module_open(name))) {
        return PERFEXPERT_ERROR;
    }

    /* Link the basic module interface symbol */
    module = (perfexpert_module_t *)lt_dlsym(modulehandle, "myself_module");
    if (NULL == module) {
        OUTPUT(("%s", _ERROR("Error: 'myself_module' symbol not found")));
        goto MODULE_ERROR;
    }

    /* Link/Set default values for the basic module interface */
    perfexpert_list_item_construct((perfexpert_list_item_t *)module);

    PERFEXPERT_ALLOC(char, module->name, (strlen(name) + 1));
    strcpy(module->name, name);

    module->version = (char *)lt_dlsym(modulehandle, "module_version");
    if (NULL == module->version) {
        OUTPUT(("%s", _ERROR("Error: 'module_version' symbol not found")));
        goto MODULE_ERROR;
    }

    module->load = lt_dlsym(modulehandle, "module_load");
    if (NULL == module->load) {
        OUTPUT(("%s", _ERROR("Error: 'module_load()' not found")));
        goto MODULE_ERROR;
    }

    module->init = lt_dlsym(modulehandle, "module_init");
    if (NULL == module->init) {
        OUTPUT(("%s", _ERROR("Error: 'module_init()' not found")));
        goto MODULE_ERROR;
    }

    module->fini = lt_dlsym(modulehandle, "module_fini");
    if (NULL == module->fini) {
        OUTPUT(("%s", _ERROR("Error: 'module_fini()' not found")));
        goto MODULE_ERROR;
    }

    module->compile = lt_dlsym(modulehandle, "module_compile");
    if (NULL == module->compile) {
        module->compile = PERFEXPERT_MODULE_NOT_IMPLEMENTED;
        OUTPUT_VERBOSE((8, "   %s does not implement module_compile", name));
    }

    module->measurements = lt_dlsym(modulehandle, "module_measurements");
    if (NULL == module->measurements) {
        module->measurements = PERFEXPERT_MODULE_NOT_IMPLEMENTED;
        OUTPUT_VERBOSE((8, "   %s does not implement module_measurements",
            name));
    }

    module->analysis = lt_dlsym(modulehandle, "module_analysis");
    if (NULL == module->analysis) {
        module->analysis = PERFEXPERT_MODULE_NOT_IMPLEMENTED;
        OUTPUT_VERBOSE((8, "   %s does not implement module_analysis", name));
    }

    /* Call module's load function */
    if (PERFEXPERT_SUCCESS != module->load()) {
        goto MODULE_ERROR;
    }

    /* Set module status */
    module->status = PERFEXPERT_MODULE_LOADED;

    /* Set module arguments */
    module->argc = 0;

    /* Add to the lists of modules */
    perfexpert_list_append(&(module_globals.modules),
        (perfexpert_list_item_t *)module);

    /* Set argv[0] */
    if (PERFEXPERT_SUCCESS != perfexpert_module_set_option(module->name, "")) {
        OUTPUT(("%s", _ERROR("Error: setting module argv[0]")));
        exit(0);
    }

    OUTPUT_VERBOSE((5, "module %s loaded [version %s]", _CYAN(module->name),
        module->version));

    return PERFEXPERT_SUCCESS;

    MODULE_ERROR:
    lt_dlclose(modulehandle);
    return PERFEXPERT_ERROR;
}

/* perfexpert_module_open */
static lt_dlhandle perfexpert_module_open(const char *name) {
    static const lt_dlinfo *moduleinfo = NULL;
    lt_dlhandle handle = NULL;
    char *filename = NULL;

    OUTPUT_VERBOSE((9, "loading module [%s]", _CYAN((char *)name)));

    /* Initialize LIBTOOL */
    if (0 != lt_dlinit()) {
        OUTPUT(("%s [%s]", _ERROR("Error: unable to initialize libtool"),
            lt_dlerror()));
        return NULL;
    }
    if (0 != lt_dlsetsearchpath(PERFEXPERT_LIBDIR)) {
        OUTPUT(("%s [%s]", _ERROR("Error: unable to set libtool search path"),
            lt_dlerror()));
        return NULL;
    }

    /* Load module file
     * WARNING: if this function returns "file not found" sometimes, the problem
     *          is not with the search path or so. This error could happens when
     *          the module has unresolved symbols. To solve this, check if there
     *          is any unresolved symbol in the module file.
     */
    PERFEXPERT_ALLOC(char, filename, (strlen(name) + 25));
    filename = (char *)malloc(strlen(name) + 25);
    sprintf(filename, "libperfexpert_module_%s.so", name);
    handle = lt_dlopenext(filename);

    if (NULL == handle) {
        OUTPUT(("%s [%s] [%s] %s", _ERROR("Error: cannot load module"),
            filename, lt_dlerror(), PERFEXPERT_LIBDIR));
        goto MODULE_ERROR;
    }
    PERFEXPERT_DEALLOC(filename);

    /* Reading module info */
    moduleinfo = lt_dlgetinfo(handle);

    if (NULL == moduleinfo) {
        OUTPUT(("cannot get module info: %s", lt_dlerror()));
        goto MODULE_ERROR;
    }
    OUTPUT_VERBOSE((10, "   name: %s", moduleinfo->name));
    OUTPUT_VERBOSE((10, "   filename: %s", _YELLOW(moduleinfo->filename)));
    OUTPUT_VERBOSE((10, "   reference count: %i", moduleinfo->ref_count));

    /* Sanity check: avoid to load the same module twice */
    if (1 < moduleinfo->ref_count) {
        OUTPUT(("%s [%s]", _RED("module already loaded"), moduleinfo->name));
        goto MODULE_ERROR;
    }

    return handle;

    MODULE_ERROR:
    lt_dlclose(handle);
    return NULL;
}

/* perfexpert_module_set_option */
int perfexpert_module_set_option(const char *module, const char *option) {
    perfexpert_module_t *m = NULL;
    char *fulloption = NULL;

    PERFEXPERT_ALLOC(char, fulloption, (strlen(option) + 3));
    sprintf(fulloption, "--%s", option);

    /* For each module... */
    SEARCH_MODULE:
    perfexpert_list_for(m, &(module_globals.modules), perfexpert_module_t) {
        if ((PERFEXPERT_MODULE_LOADED == m->status) &&
            (0 == strcmp(m->name, module))) {

            m->argv[m->argc] = fulloption;
            m->argc++;

            OUTPUT_VERBOSE((1, "%s option set [%s]", module, option));

            return PERFEXPERT_SUCCESS;
        }
    }

    if (PERFEXPERT_SUCCESS != perfexpert_module_load(module)) {
        OUTPUT(("%s [%s]", _ERROR("Error: while adding module"), module));
        return PERFEXPERT_ERROR;
    }
    goto SEARCH_MODULE;
}

/* perfexpert_module_init */
int perfexpert_module_init(void) {
    perfexpert_ordered_module_t *om = NULL, *prev = NULL;
    perfexpert_module_t *m = NULL;

    /* Load missing modules, set its pointer, and remove non-implemented ones */
    perfexpert_list_for(om, &(module_globals.compile),
        perfexpert_ordered_module_t) {
        if (NULL == (m = perfexpert_module_available(om->name))) {
            if (PERFEXPERT_SUCCESS != perfexpert_module_load(om->name)) {
                OUTPUT(("%s [%s]", _ERROR("Error: while adding module"),
                    om->name));
                return PERFEXPERT_ERROR;
            }
            m = perfexpert_module_available(om->name);
        }
        om->module = m;
        if (PERFEXPERT_MODULE_NOT_IMPLEMENTED == m->compile) {
            prev = (perfexpert_ordered_module_t *)om->prev;
            OUTPUT(("%s [%s] it does not implement compile phase",
                _RED("removing module"), om->name));
            perfexpert_list_remove_item(&(module_globals.compile),
                (perfexpert_list_item_t *)om);
            PERFEXPERT_DEALLOC(om->name);
            PERFEXPERT_DEALLOC(om);
            om = prev;
        }
    }

    perfexpert_list_for(om, &(module_globals.measurements),
        perfexpert_ordered_module_t) {
        if (NULL == (m = perfexpert_module_available(om->name))) {
            if (PERFEXPERT_SUCCESS != perfexpert_module_load(om->name)) {
                OUTPUT(("%s [%s]", _ERROR("Error: while adding module"),
                    om->name));
                return PERFEXPERT_ERROR;
            }
            m = perfexpert_module_available(om->name);
        }
        om->module = m;
        if (PERFEXPERT_MODULE_NOT_IMPLEMENTED == m->measurements) {
            prev = (perfexpert_ordered_module_t *)om->prev;
            OUTPUT(("%s [%s] it does not implement measurements phase",
                _RED("removing module"), om->name));
            perfexpert_list_remove_item(&(module_globals.measurements),
                (perfexpert_list_item_t *)om);
            PERFEXPERT_DEALLOC(om->name);
            PERFEXPERT_DEALLOC(om);
            om = prev;
        }
    }

    perfexpert_list_for(om, &(module_globals.analysis),
        perfexpert_ordered_module_t) {
        if (NULL == (m = perfexpert_module_available(om->name))) {
            if (PERFEXPERT_SUCCESS != perfexpert_module_load(om->name)) {
                OUTPUT(("%s [%s]", _ERROR("Error: while adding module"),
                    om->name));
                return PERFEXPERT_ERROR;
            }
            m = perfexpert_module_available(om->name);
        }
        om->module = m;
        if (PERFEXPERT_MODULE_NOT_IMPLEMENTED == m->analysis) {
            prev = (perfexpert_ordered_module_t *)om->prev;
            OUTPUT(("%s [%s] it does not implement analysis phase",
                _RED("removing module"), om->name));
            perfexpert_list_remove_item(&(module_globals.analysis),
                (perfexpert_list_item_t *)om);
            PERFEXPERT_DEALLOC(om->name);
            PERFEXPERT_DEALLOC(om);
            om = prev;
        }
    }

    /* If a module is not in a list but implements such phase, add it to list */
    perfexpert_list_for(m, &(module_globals.modules), perfexpert_module_t) {
        int is_in_list = PERFEXPERT_FALSE;
        perfexpert_list_for(om, &(module_globals.compile),
            perfexpert_ordered_module_t) {
            if (0 == strcmp(om->name, m->name)) {
                is_in_list = PERFEXPERT_TRUE;
            }
        }
        if ((is_in_list == PERFEXPERT_FALSE) &&
            (PERFEXPERT_MODULE_NOT_IMPLEMENTED != m->compile)) {
            PERFEXPERT_ALLOC(perfexpert_ordered_module_t, om,
                sizeof(perfexpert_ordered_module_t));
            PERFEXPERT_ALLOC(char, om->name, (strlen(m->name) + 1));
            strcpy(om->name, m->name);
            om->module = m;
            perfexpert_list_item_construct((perfexpert_list_item_t *)om);
            perfexpert_list_append(&(module_globals.compile),
                (perfexpert_list_item_t *)om);
        }

        is_in_list = PERFEXPERT_FALSE;
        perfexpert_list_for(om, &(module_globals.measurements),
            perfexpert_ordered_module_t) {
            if (0 == strcmp(om->name, m->name)) {
                is_in_list = PERFEXPERT_TRUE;
            }
        }
        if ((is_in_list == PERFEXPERT_FALSE) &&
            (PERFEXPERT_MODULE_NOT_IMPLEMENTED != m->measurements)) {
            PERFEXPERT_ALLOC(perfexpert_ordered_module_t, om,
                sizeof(perfexpert_ordered_module_t));
            PERFEXPERT_ALLOC(char, om->name, (strlen(m->name) + 1));
            strcpy(om->name, m->name);
            om->module = m;
            perfexpert_list_item_construct((perfexpert_list_item_t *)om);
            perfexpert_list_append(&(module_globals.measurements),
                (perfexpert_list_item_t *)om);
        }

        is_in_list = PERFEXPERT_FALSE;
        perfexpert_list_for(om, &(module_globals.analysis),
            perfexpert_ordered_module_t) {
            if (0 == strcmp(om->name, m->name)) {
                is_in_list = PERFEXPERT_TRUE;
            }
        }
        if ((is_in_list == PERFEXPERT_FALSE) &&
            (PERFEXPERT_MODULE_NOT_IMPLEMENTED != m->analysis)) {
            PERFEXPERT_ALLOC(perfexpert_ordered_module_t, om,
                sizeof(perfexpert_ordered_module_t));
            PERFEXPERT_ALLOC(char, om->name, (strlen(m->name) + 1));
            strcpy(om->name, m->name);
            om->module = m;
            perfexpert_list_item_construct((perfexpert_list_item_t *)om);
            perfexpert_list_append(&(module_globals.analysis),
                (perfexpert_list_item_t *)om);
        }
    }

    /* Initialize each module */
    perfexpert_list_for(m, &(module_globals.modules), perfexpert_module_t) {
        if (PERFEXPERT_MODULE_INITIALIZED == m->status) {
            continue;
        }

        if (PERFEXPERT_MODULE_LOADED != m->status) {
            OUTPUT(("%s [%s]", _ERROR("Error: module status different from"
                "PERFEXPERT_MODULE_LOADED"), m->name));
            return PERFEXPERT_ERROR;
        }

        if (PERFEXPERT_SUCCESS != m->init()) {
            OUTPUT(("%s [%s]", _ERROR("Error: while initializing module"),
                m->name));
            return PERFEXPERT_ERROR;
        }

        m->status = PERFEXPERT_MODULE_INITIALIZED;
    }

    /* Print summary */
    if (7 <= globals.verbose) {
        printf("%s    %s            ", PROGRAM_PREFIX, _YELLOW("modules:"));
        perfexpert_list_for(m, &(module_globals.modules), perfexpert_module_t) {
            printf(" [%s]", m->name);
        }
        printf("\n%s    %s      ", PROGRAM_PREFIX, _YELLOW("compile order:"));
        perfexpert_list_for(om, &(module_globals.compile),
            perfexpert_ordered_module_t) {
            printf(" >> %s", om->name);
        }
        printf("\n%s    %s ", PROGRAM_PREFIX, _YELLOW("measurements order:"));
        perfexpert_list_for(om, &(module_globals.measurements),
            perfexpert_ordered_module_t) {
            printf(" >> %s", om->name);
        }
        printf("\n%s    %s     ", PROGRAM_PREFIX, _YELLOW("analysis order:"));
        perfexpert_list_for(om, &(module_globals.analysis),
            perfexpert_ordered_module_t) {
            printf(" >> %s", om->name);
        }
        printf("\n");
        fflush(stdout);
    }

    return PERFEXPERT_SUCCESS;
}

/* perfexpert_module_fini */
int perfexpert_module_fini(void) {
    perfexpert_module_t *m = NULL;

    /* For each module... */
    perfexpert_list_for(m, &(module_globals.modules), perfexpert_module_t) {
        if (PERFEXPERT_MODULE_FINALIZED == m->status) {
            continue;
        }

        if (PERFEXPERT_MODULE_INITIALIZED != m->status) {
            OUTPUT(("%s [%s]", _ERROR("Error: module status different from"
                "PERFEXPERT_MODULE_INITIALIZED"), m->name));
            return PERFEXPERT_ERROR;
        }

        if (PERFEXPERT_SUCCESS != m->fini()) {
            OUTPUT(("%s [%s]", _ERROR("Error: while finalizing module"),
                m->name));
            return PERFEXPERT_ERROR;
        }

        m->status = PERFEXPERT_MODULE_FINALIZED;

        // TODO: free argv!
    }
    return PERFEXPERT_SUCCESS;
}

/* perfexpert_module_compile */
int perfexpert_module_compile(void) {
    perfexpert_ordered_module_t *m = NULL;

    /* For each module... */
    perfexpert_list_for(m, &(module_globals.compile),
        perfexpert_ordered_module_t) {
        if (PERFEXPERT_MODULE_INITIALIZED != m->module->status) {
            OUTPUT(("%s [%s]", _ERROR("Error: module status different from "
                "PERFEXPERT_MODULE_INITIALIZED"), m->module->name));
            return PERFEXPERT_ERROR;
        }

        if (PERFEXPERT_SUCCESS != m->module->compile()) {
            OUTPUT(("%s [%s]", _ERROR("Error: compiling"), m->module->name));
            return PERFEXPERT_ERROR;
        }
    }
    return PERFEXPERT_SUCCESS;
}

/* perfexpert_module_measurements */
int perfexpert_module_measurements(void) {
    perfexpert_ordered_module_t *m = NULL;

    /* For each module... */
    perfexpert_list_for(m, &(module_globals.measurements),
        perfexpert_ordered_module_t) {
        if (PERFEXPERT_MODULE_INITIALIZED != m->module->status) {
            OUTPUT(("%s [%s]", _ERROR("Error: module status different from "
                "PERFEXPERT_MODULE_INITIALIZED"), m->module->name));
            return PERFEXPERT_ERROR;
        }

        if (PERFEXPERT_SUCCESS != m->module->measurements()) {
            OUTPUT(("%s [%s]", _ERROR("Error: collecting measurements"),
                m->module->name));
            return PERFEXPERT_ERROR;
        }
    }
    return PERFEXPERT_SUCCESS;
}

/* perfexpert_module_analysis */
int perfexpert_module_analysis(void) {
    perfexpert_ordered_module_t *m = NULL;

    /* For each module... */
    perfexpert_list_for(m, &(module_globals.analysis),
        perfexpert_ordered_module_t) {
        if (PERFEXPERT_MODULE_INITIALIZED != m->module->status) {
            OUTPUT(("%s [%s]", _ERROR("Error: module status different from "
                "PERFEXPERT_MODULE_INITIALIZED"), m->module->name));
            return PERFEXPERT_ERROR;
        }

        if (PERFEXPERT_SUCCESS != m->module->analysis()) {
            OUTPUT(("%s [%s]", _ERROR("Error: analysing"), m->module->name));
            return PERFEXPERT_ERROR;
        }
    }
    return PERFEXPERT_SUCCESS;
}

/* perfexpert_module_requires (module A, requires B order in phase) */
int perfexpert_module_requires(const char *a, const char *b,
    module_order_t order, module_phase_t phase) {
    perfexpert_ordered_module_t *oma = NULL, *omb = NULL, *t = NULL;
    perfexpert_module_t *mb = NULL;
    int x = 0, xa = 0, xb = 0;

    if (NULL == (mb = perfexpert_module_available(b))) {
        OUTPUT_VERBOSE((1, "%s [%s], it's required by module [%s]",
            _RED("loading module"), b, a));
        if (PERFEXPERT_SUCCESS != perfexpert_module_load(b)) {
            return PERFEXPERT_ERROR;
        }
    }

    if (PERFEXPERT_MODULE_COMPILE == phase) {
        oma = NULL; omb = NULL; t = NULL; x = 0; xa = 0, xb = 0;

        perfexpert_list_for(t, &(module_globals.compile),
            perfexpert_ordered_module_t) {
            if (0 == strcmp(a, t->name)) {
                oma = t;
                xa = x;
            }
            if (0 == strcmp(b, t->name)) {
                omb = t;
                xb = x;
            }
            x++;
        }

        if ((PERFEXPERT_MODULE_BEFORE == order) && (xb > xa)) {
            OUTPUT_VERBOSE((1, "%s %s, it requires module %s first in compile "
                "phase", _RED("reordering module"), oma->name, omb->name));
            perfexpert_list_move_before((perfexpert_list_item_t *)omb,
                (perfexpert_list_item_t *)oma);
        } else if ((PERFEXPERT_MODULE_AFTER == order) && (xb < xa)) {
            OUTPUT_VERBOSE((1, "%s %s, it requires module %s after in compile "
                "phase", _RED("reordering module"), oma->name, omb->name));
            perfexpert_list_move_after((perfexpert_list_item_t *)omb,
                (perfexpert_list_item_t *)oma);
        } else if (PERFEXPERT_MODULE_AVAILABLE == order) {
            if (PERFEXPERT_MODULE_NOT_IMPLEMENTED == mb->compile) {
                return PERFEXPERT_ERROR;
            }
        }
    }

    if (PERFEXPERT_MODULE_MEASUREMENTS == phase) {
        oma = NULL; omb = NULL; t = NULL; x = 0; xa = 0, xb = 0;

        perfexpert_list_for(t, &(module_globals.measurements),
            perfexpert_ordered_module_t) {
            if (0 == strcmp(a, t->name)) {
                oma = t;
                xa = x;
            } else if (0 == strcmp(b, t->name)) {
                omb = t;
                xb = x;
            }
            x++;
        }

        if ((PERFEXPERT_MODULE_BEFORE == order) && (xb > xa)) {
            OUTPUT_VERBOSE((1, "%s %s, it requires module %s first in measureme"
                "nts phase", _RED("reordering module"), oma->name, omb->name));
            perfexpert_list_move_before((perfexpert_list_item_t *)omb,
                (perfexpert_list_item_t *)oma);
        } else if ((PERFEXPERT_MODULE_AFTER == order) && (xb < xa)) {
            OUTPUT_VERBOSE((1, "%s %s, it requires module %s after in measureme"
                "nts phase", _RED("reordering module"), oma->name, omb->name));
            perfexpert_list_move_after((perfexpert_list_item_t *)omb,
                (perfexpert_list_item_t *)oma);
        } else if (PERFEXPERT_MODULE_AVAILABLE == order) {
            if (PERFEXPERT_MODULE_NOT_IMPLEMENTED == mb->compile) {
                return PERFEXPERT_ERROR;
            }
        }
    }

    if (PERFEXPERT_MODULE_ANALYSIS == phase) {
        oma = NULL; omb = NULL; t = NULL; x = 0; xa = 0, xb = 0;

        perfexpert_list_for(t, &(module_globals.analysis),
            perfexpert_ordered_module_t) {
            if (0 == strcmp(a, t->name)) {
                oma = t;
                xa = x;
            } else if (0 == strcmp(b, t->name)) {
                omb = t;
                xb = x;
            }
            x++;
        }

        if ((PERFEXPERT_MODULE_BEFORE == order) && (xb > xa)) {
            OUTPUT_VERBOSE((1, "%s %s, it requires module %s first in analysis "
                "phase", _RED("reordering module"), oma->name, omb->name));
            perfexpert_list_move_before((perfexpert_list_item_t *)omb,
                (perfexpert_list_item_t *)oma);
        } else if ((PERFEXPERT_MODULE_AFTER == order) && (xb < xa)) {
            OUTPUT_VERBOSE((1, "%s %s, it requires module %s after in analysis "
                "phase", _RED("reordering module"), oma->name, omb->name));
            perfexpert_list_move_after((perfexpert_list_item_t *)omb,
                (perfexpert_list_item_t *)oma);
        } else if (PERFEXPERT_MODULE_AVAILABLE == order) {
            if (PERFEXPERT_MODULE_NOT_IMPLEMENTED == mb->analysis) {
                return PERFEXPERT_ERROR;
            }
        }
    }

    return PERFEXPERT_ERROR;
}

/* perfexpert_module_available */
perfexpert_module_t *perfexpert_module_available(const char *name) {
    perfexpert_module_t *m = NULL;

    /* For each module... */
    perfexpert_list_for(m, &(module_globals.modules), perfexpert_module_t) {
        if (0 == strcmp(name, m->name)) {
            return m;
        }
    }
    return NULL;
}

// EOF
