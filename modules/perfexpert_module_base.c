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
#include <dirent.h>

#ifdef PROGRAM_PREFIX
#undef PROGRAM_PREFIX
#endif
#define PROGRAM_PREFIX "[perfexpert_module]"

/* Modules headers */
#include "perfexpert_module_base.h"

/* Tool headers */
#include "tools/perfexpert/perfexpert_types.h"

/* PerfExpert common headers */
#include "common/perfexpert_alloc.h"
#include "common/perfexpert_list.h"
#include "common/perfexpert_output.h"
#include "install_dirs.h"

/* Module global variables */
module_globals_t module_globals;

/* PerfExpert phase name */
char *perfexpert_phase_name[] = {
    "compile", "instrument", "measure", "analyze", "recommend", NULL
};

/* my_init (runs automatically in loading time) */
void __attribute__ ((constructor)) my_init(void) {
    perfexpert_list_construct(&(module_globals.modules));
    perfexpert_list_construct(&(module_globals.steps));
}

/* Public functions (should be mentioned in the sym file) */
/* perfexpert_module_load*/
int perfexpert_module_load(const char *name) {
    perfexpert_module_t *m = NULL;
    lt_dlhandle handle = NULL;

    OUTPUT_VERBOSE((9, "loading module [%s]", _CYAN((char *)name)));

    /* Sanity check: is this module already loaded? */
    perfexpert_list_for(m, &(module_globals.modules), perfexpert_module_t) {
        if (0 == strcmp(name, m->name)) {
            OUTPUT_VERBOSE((8, "module already loaded"));
            return PERFEXPERT_SUCCESS;
        }
    }

    /* Load module */
    if (NULL == (handle = perfexpert_module_open(name))) {
        OUTPUT(("%s [%s]", _ERROR("error openning module"), name));
        return PERFEXPERT_ERROR;
    }

    /* Link the basic module interface symbol */
    m = (perfexpert_module_t *)lt_dlsym(handle, "myself_module");
    if (NULL == m) {
        OUTPUT(("%s", _ERROR("'myself_module' symbol not found")));
        lt_dlclose(handle);
        return PERFEXPERT_ERROR;
    }

    /* Setup the module list item */
    perfexpert_list_item_construct((perfexpert_list_item_t *)m);
    PERFEXPERT_ALLOC(char, m->name, (strlen(name) + 1));
    strcpy(m->name, name);

    /* Link and set the minimun required interfaces */
    if (NULL == (m->version = (char *)lt_dlsym(handle, "module_version"))) {
        OUTPUT(("%s", _ERROR("'module_version' symbol not found")));
        goto MODULE_ERROR;
    }
    if (NULL == (m->load = lt_dlsym(handle, "module_load"))) {
        OUTPUT(("%s", _ERROR("'module_load()' not found")));
        goto MODULE_ERROR;
    }
    if (NULL == (m->init = lt_dlsym(handle, "module_init"))) {
        OUTPUT(("%s", _ERROR("'module_init()' not found")));
        goto MODULE_ERROR;
    }
    if (NULL == (m->fini = lt_dlsym(handle, "module_fini"))) {
        OUTPUT(("%s", _ERROR("'module_fini()' not found")));
        goto MODULE_ERROR;
    }

    /* Link the recommender interface */
    if (NULL != (m->recommend = lt_dlsym(handle, "module_recommend"))) {
        if (PERFEXPERT_SUCCESS != perfexpert_phase_add(m,
            PERFEXPERT_PHASE_RECOMMEND)) {
            OUTPUT(("%s [%s]", _ERROR("adding 'recommend' phase of module"),
                m->name));
            goto MODULE_ERROR;
        }
    } else {
        OUTPUT_VERBOSE((8, "   %s does not implement 'recommend'", name));
    }

    /* Link the analysis interface */
    if (NULL != (m->analyze = lt_dlsym(handle, "module_analyze"))) {
        if (PERFEXPERT_SUCCESS != perfexpert_phase_add(m,
            PERFEXPERT_PHASE_ANALYZE)) {
            OUTPUT(("%s [%s]", _ERROR("adding 'analyze' phase of module"),
                m->name));
            goto MODULE_ERROR;
        }
    } else {
        OUTPUT_VERBOSE((8, "   %s does not implement 'analyze'", name));
    }

    /* Link the measurements interface */
    if (NULL != (m->measure = lt_dlsym(handle, "module_measure"))) {
        if (PERFEXPERT_SUCCESS != perfexpert_phase_add(m,
            PERFEXPERT_PHASE_MEASURE)) {
            OUTPUT(("%s [%s]", _ERROR("adding 'measure' phase of module"),
                m->name));
            goto MODULE_ERROR;
        }
    } else {
        OUTPUT_VERBOSE((8, "   %s does not implement 'measure'", name));
    }

    /* Link the instrumentation interface */
    if (NULL != (m->instrument = lt_dlsym(handle, "module_instrument"))) {
        if (PERFEXPERT_SUCCESS != perfexpert_phase_add(m,
            PERFEXPERT_PHASE_INSTRUMENT)) {
            OUTPUT(("%s [%s]", _ERROR("adding 'instrument' phase of module"),
                m->name));
            goto MODULE_ERROR;
        }
    } else {
        OUTPUT_VERBOSE((8, "   %s does not implement 'instrument'", m->name));
    }

    /* Link the compilation interface */
    if (NULL != (m->compile = lt_dlsym(handle, "module_compile"))) {
        if (PERFEXPERT_SUCCESS != perfexpert_phase_add(m,
            PERFEXPERT_PHASE_COMPILE)) {
            OUTPUT(("%s [%s]", _ERROR("adding 'compile' phase of module"),
                m->name));
            goto MODULE_ERROR;
        }
    } else {
        OUTPUT_VERBOSE((8, "   %s does not implement 'compile'", m->name));
    }

    /* Call module's load function */
    if (PERFEXPERT_SUCCESS != m->load()) {
        OUTPUT(("%s [%s]", _ERROR("error running module's load()"), m->name));
        goto MODULE_ERROR;
    } else {
        m->status = PERFEXPERT_MODULE_LOADED;
        m->argv[0] = m->name;
        m->argc = 1;
    }

    /* Append to the lists of modules */
    perfexpert_list_append(&(module_globals.modules),
        (perfexpert_list_item_t *)m);

    OUTPUT_VERBOSE((5, "module %s loaded [%s]", _CYAN(m->name), m->version));

    return PERFEXPERT_SUCCESS;

    MODULE_ERROR:
    PERFEXPERT_DEALLOC(m->name);
    perfexpert_module_close(handle, name);
    return PERFEXPERT_ERROR;
}

/* perfexpert_module_unload*/
int perfexpert_module_unload(const char *name) {
    perfexpert_module_t *m = NULL;
    lt_dlhandle handle = NULL;

    OUTPUT_VERBOSE((9, "unloading module [%s]", _CYAN((char *)name)));

    /* Sanity check: is this module loaded? */
    perfexpert_list_for(m, &(module_globals.modules), perfexpert_module_t) {
        if (0 == strcmp(name, m->name)) {
            break;
        }
    }
    if (NULL == m) {
        OUTPUT(("%s [%s]", _ERROR("module not loaded"), name));
        return PERFEXPERT_ERROR;
    }

    /* Retrieve module handle */
    if (NULL == (handle = perfexpert_module_open(name))) {
        OUTPUT(("%s [%s]", _ERROR("error finding module module"), name));
        return PERFEXPERT_ERROR;
    }

    /* Close module */
    perfexpert_module_close(handle, name);

    return PERFEXPERT_SUCCESS;
}

/* perfexpert_module_set_option */
int perfexpert_module_set_option(const char *module, const char *option) {
    perfexpert_module_t *m = NULL;
    char *fulloption = NULL;

    PERFEXPERT_ALLOC(char, fulloption, (strlen(option) + 3));
    sprintf(fulloption, "--%s", option);

    if (PERFEXPERT_FALSE == perfexpert_module_available(module)) {
        if (PERFEXPERT_FALSE == perfexpert_module_installed(module)) {
            OUTPUT(("%s [%s]", _ERROR("module not installed"), module));
            return PERFEXPERT_ERROR;
        }
        if (PERFEXPERT_SUCCESS != perfexpert_module_load(module)) {
            OUTPUT(("%s [%s]", _ERROR("while adding module"), module));
            return PERFEXPERT_ERROR;
        }
    }

    if (NULL != (m = perfexpert_module_get(module))) {
        OUTPUT_VERBOSE((1, "%s option set [%s]", module, option));

        m->argv[m->argc] = fulloption;
        m->argc++;

        return PERFEXPERT_SUCCESS;
    }

    OUTPUT(("%s [%s]", _ERROR("unable to get module"), module));

    return PERFEXPERT_ERROR;
}

/* perfexpert_module_init */
int perfexpert_module_init(void) {
    perfexpert_step_t *step = NULL;
    perfexpert_module_t *m = NULL;

    OUTPUT_VERBOSE((10, "%s", _BLUE("Initializing modules")));

    /* Initialize each module */
    perfexpert_list_for(m, &(module_globals.modules), perfexpert_module_t) {
        /* Paranoia: skip modules already initialized (is it possible?) */
        if (PERFEXPERT_MODULE_INITIALIZED == m->status) {
            continue;
        }
        if (PERFEXPERT_MODULE_LOADED != m->status) {
            OUTPUT(("%s [%s]", _ERROR("module status different from"
                "PERFEXPERT_MODULE_LOADED"), m->name));
            return PERFEXPERT_ERROR;
        }

        /* Call module's init() */
        if (PERFEXPERT_SUCCESS != m->init()) {
            OUTPUT(("%s [%s]", _ERROR("error initializing module"), m->name));
            return PERFEXPERT_ERROR;
        }

        m->status = PERFEXPERT_MODULE_INITIALIZED;
    }

    /* Print summary */
    if (7 <= globals.verbose) {
        printf("%s %s", PROGRAM_PREFIX, _GREEN("Final list of modules:"));
        perfexpert_list_for(m, &(module_globals.modules), perfexpert_module_t) {
            printf(" [%s]", m->name);
        }
        printf("\n%s %s    ", PROGRAM_PREFIX, _GREEN("Final steps order:"));
        perfexpert_list_for(step, &(module_globals.steps), perfexpert_step_t) {
            printf(" [%s/%d]", step->name, step->phase);
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

        if (PERFEXPERT_SUCCESS != m->fini()) {
            OUTPUT(("%s [%s]", _ERROR("while finalizing module"), m->name));
            return PERFEXPERT_ERROR;
        }

        m->status = PERFEXPERT_MODULE_FINALIZED;
    }
    return PERFEXPERT_SUCCESS;
}

/* perfexpert_module_requires
 *
 * Explanation: this functions should be interpreted as "module A/phase A
 *              REQUIRES module B/phase B ORDER". The module A/phase A will be
 * moved around while the module B/phase B will stay in the same order. See the
 * header file (perfexpert_module_base.h) to know the valid values for 'ap',
 * 'bp', and 'order' variables (module_phase_t and module_order_t enumerations).
 * There are some special cases or 'order' where 'b' and 'bp' are not taken into
 * consideration: PERFEXPERT_MODULE_FIRST and PERFEXPERT_MODULE_LAST. Another
 * special case is when 'b' is NULL. In this case it means "any module which
 * matches the phase criteria" -- very useful to clone the compilation phase.
 */
int perfexpert_module_requires(const char *a, perfexpert_step_phase_t pa,
    const char *b, perfexpert_step_phase_t pb, perfexpert_module_order_t o) {
    /* step A, step B, temporary step, first step, last step */
    perfexpert_step_t *sa = NULL, *sb = NULL, *t = NULL, *f = NULL, *l = NULL;
    perfexpert_module_t *m = NULL;
    int x = 0, xa = 0, xb = 0;

    /* Is the 'b' module already loaded? */
    if ((NULL == (m = perfexpert_module_get(b))) && (NULL != b)) {
        OUTPUT_VERBOSE((1, "module [%s] is required by module [%s]", b, a));
        if (PERFEXPERT_SUCCESS != perfexpert_module_load(b)) {
            /* WARNING: we cannot trust the module will check for the return
             *          code of this function (mainly because module can be
             * developed by contributors). So we should abort the execution here
             * if an error happens when trying to load the pre-requisite module.
             * The problem is that we are not able to clean up all the temporary
             * files and etc, leaving garbage behind. So let's print an error
             * message and let the user decide what to do.
             */
            OUTPUT(("%s", _ERROR("The requested module is not available. "
                "Hopefully, the module will abort the execution or handle this "
                "error, but we cannot guarantee it will happen. PerfExpert will"
                " not abort the execution rigth now, but be aware that "
                "unexpected results may happen if the module does not handle "
                "this error appropriately.")));
            return PERFEXPERT_ERROR;
        } else {
            if (NULL != (m = perfexpert_module_get(b))) {
                /* To avoid ordering problems the loaded module should be
                 * initialized now */
                if (PERFEXPERT_SUCCESS != m->init()) {
                    OUTPUT(("%s [%s]", _ERROR("error initializing module"),
                        m->name));
                    return PERFEXPERT_ERROR;
                }
                m->status = PERFEXPERT_MODULE_INITIALIZED;
            }
        }
    }

    /* Find module/phase and their position on the list of steps */
    perfexpert_list_for(t, &(module_globals.steps), perfexpert_step_t) {
        /* find module A, phase PA */
        if ((t->phase == pa) && (0 == strcmp(a, t->name))) {
            sa = t;
            xa = x;
        }
        /* find module B, phase PB */
        if (NULL != b) {
            if ((t->phase == pb) && (0 == strcmp(b, t->name))) {
                sb = t;
                xb = x;
            }
        }
        /* find module B NULL, phase PB */
        if ((NULL == b) && (t->phase == pb)) {
            sb = t;
            xb = x;
        }
        /* find position FIRST */
        if (0 == x) {
            f = t;
        }
        /* find position LAST */
        l = t;
        /* Move on */
        x++;
    }

    /* Paranoia? Maybe! Anyway, it doesn't hurt. Do both phases exist? */
    if ((NULL == sa) || ((NULL == sb) && (NULL != b))) {
        OUTPUT(("%s [A=%s/%d,B=%s/%d]", _ERROR("The requested module/phase "
            "is not available. Hopefully, the module will abort the execution "
            "or handle this error, but we cannot guarantee it will happen. "
            "PerfExpert will not abort the execution rigth now, but be aware "
            "that unexpected results may happen if the module does not handle "
            "this error appropriately."), sa ? sa->name : "(null)", pa,
            sb ? sb->name : "(null)", pb));
        return PERFEXPERT_ERROR;
    }

    /* Should we reorder or clone some module/phase? */
    if (NULL != b) {
        /* Reorder */
        if ((PERFEXPERT_MODULE_BEFORE == o) && (xb > xa)) {
            OUTPUT_VERBOSE((1, "%s: %s/%d requires %s/%d first",
                _RED("reordering steps"), sa->name, pa, sb->name, pb));
            perfexpert_list_move_after((perfexpert_list_item_t *)sa,
                (perfexpert_list_item_t *)sb);
        } else if ((PERFEXPERT_MODULE_AFTER == o) && (xb < xa)) {
            OUTPUT_VERBOSE((1, "%s: %s/%d requires %s/%d after",
                _RED("reordering steps"), sa->name, pa, sb->name, pb));
            perfexpert_list_move_before((perfexpert_list_item_t *)sa,
                (perfexpert_list_item_t *)sb);
        }
    } else {
        if (PERFEXPERT_MODULE_CLONE_BEFORE == o) {
            t = perfexpert_step_clone(sb);
            if (NULL != t) {
                OUTPUT_VERBOSE((1, "%s: %s/%d requires %s/%d first",
                    _RED("cloning step"), sa->name, pa, t->name, t->phase));
                perfexpert_list_insert(&(module_globals.steps),
                    (perfexpert_list_item_t *)t, xa);
            }
        } else if (PERFEXPERT_MODULE_CLONE_AFTER == o) {
            if (NULL == (t = perfexpert_step_clone(sb))) {
                OUTPUT(("%s", _ERROR("no compile module/phase found")));
                return PERFEXPERT_ERROR;
            }
            if (NULL != t) {
                OUTPUT_VERBOSE((1, "%s: %s/%d requires %s/%d after",
                    _RED("cloning step"), sa->name, pa, t->name, t->phase));
                perfexpert_list_insert(&(module_globals.steps),
                    (perfexpert_list_item_t *)t, xa + 1);
            }
        } else if ((PERFEXPERT_MODULE_FIRST == o) && (0 < xa)) {
            OUTPUT_VERBOSE((1, "%s: %s/%d requires first", _RED("moving step"),
                sa->name, pa));
            perfexpert_list_move_before((perfexpert_list_item_t *)sa,
                (perfexpert_list_item_t *)f);
        } else if ((PERFEXPERT_MODULE_LAST == o) && (x - 1 > xa)) {
            OUTPUT_VERBOSE((1, "%s: %s/%d requires last", _RED("moving step"),
                sa->name, pa));
            perfexpert_list_move_after((perfexpert_list_item_t *)sa,
                (perfexpert_list_item_t *)l);
        }
    }

    /* We are ready for now */
    return PERFEXPERT_SUCCESS;
}

/* perfexpert_module_get */
perfexpert_module_t *perfexpert_module_get(const char *name) {
    perfexpert_module_t *m = NULL;

    /* Basic check */
    if (NULL == name) {
        return NULL;
    }

    /* For each module... */
    perfexpert_list_for(m, &(module_globals.modules), perfexpert_module_t) {
        if (0 == strcmp(name, m->name)) {
            return m;
        }
    }

    /* Module not found */
    return NULL;
}

/* perfexpert_module_available */
int perfexpert_module_available(const char *name) {
    perfexpert_module_t *m = NULL;

    /* Basic check */
    if (NULL == name) {
        return PERFEXPERT_FALSE;
    }

    /* For each module... */
    perfexpert_list_for(m, &(module_globals.modules), perfexpert_module_t) {
        if (0 == strcmp(name, m->name)) {
            return PERFEXPERT_TRUE;
        }
    }

    /* Module not found */
    return PERFEXPERT_FALSE;
}

/* perfexpert_module_installed */
int perfexpert_module_installed(const char *name) {
    lt_dlhandle handle;

    if (NULL != (handle = perfexpert_module_open(name))) {
        perfexpert_module_close(handle, name);
        return PERFEXPERT_TRUE;
    }

    return PERFEXPERT_FALSE;
}

void perfexpert_module_help(const char *name) {
    perfexpert_module_help_fn_t help = NULL;
    lt_dlhandle handle = NULL;
    struct dirent *entry = NULL;
    DIR *directory = NULL;
    int x = 0, y = 0;
    char *m = NULL;

    /* Doesn't hurt, right? */
    if (NULL == name) {
        OUTPUT(("%s", _ERROR("module name is null")));
        return;
    }

    /* Should I display help for all modules? Holy crap... let's go? */
    if (0 == strncmp(name, "all", 3)) {
        if (NULL == (directory = opendir(PERFEXPERT_LIBDIR))) {
            OUTPUT(("%s [%s]", _ERROR("unable to open modules directory"),
                PERFEXPERT_LIBDIR));
            return;
        }
        while (NULL != (entry = readdir(directory))) {
            if ((0 == strncmp(entry->d_name, "libperfexpert_module_", 21)) &&
                ('.' == entry->d_name[strlen(entry->d_name) - 3]) &&
                ('s' == entry->d_name[strlen(entry->d_name) - 2]) &&
                ('o' == entry->d_name[strlen(entry->d_name) - 1]) &&
                (0 != strcmp(entry->d_name, "libperfexpert_module_base.so"))) {

                PERFEXPERT_ALLOC(char, m, (strlen(entry->d_name) - 23));
                for (x = 21, y = 0; x < (strlen(entry->d_name) - 3); x++, y++) {
                    m[y] = entry->d_name[x];
                }

                if (NULL == (handle = perfexpert_module_open(m))) {
                    OUTPUT(("%s [%s]", _ERROR("unable to open module"), m));
                    OUTPUT(("Is %s in your LD_LIBRARY_PATH?",
                        PERFEXPERT_LIBDIR));
                    PERFEXPERT_DEALLOC(m);
                    continue;
                }
                if (NULL != (help = lt_dlsym(handle, "module_help"))) {
                    help();
                    printf("\n To set one of the above options use --module"
                        "-option=%s,OPTION=VALUE\n", m);
                } else {
                    OUTPUT_VERBOSE((8, "module_help() not defined"));
                }
                perfexpert_module_close(handle, m);
                PERFEXPERT_DEALLOC(m);
            }
        }
        if (0 != closedir(directory)) {
            OUTPUT(("%s [%s]", _ERROR("unable to close libdir")));
        }
    } else {
    /* Show module's help messages */
        if (handle != perfexpert_module_open(name)) {
            OUTPUT(("%s [%s]", _ERROR("unable to open module"), name));
            return;
        }
        if (NULL != (help = lt_dlsym(handle, "module_help"))) {
            printf("\n To set one of the following options use --module-option="
                "%s,OPTION=VALUE\n", name);
            help();
        }
        perfexpert_module_close(handle, m);
    }
}

/* perfexpert_phase_available */
int perfexpert_phase_available(perfexpert_step_phase_t phase) {
    perfexpert_step_t *s = NULL;

    /* Find module/phase and their position on the list of steps */
    perfexpert_list_for(s, &(module_globals.steps), perfexpert_step_t) {
        if (PERFEXPERT_PHASE_COMPILE == s->phase) {
            return PERFEXPERT_SUCCESS;
        }
    }

    return PERFEXPERT_ERROR;
}

/* Private functions (should NOT be mentioned in the sym file) */
/* perfexpert_module_open */
static lt_dlhandle perfexpert_module_open(const char *name) {
    static const lt_dlinfo *moduleinfo = NULL;
    lt_dlhandle handle = NULL;
    char *filename = NULL;

    /* Initialize LIBTOOL */
    if (0 != lt_dlinit()) {
        OUTPUT(("%s [%s]", _ERROR("unable to initialize libtool"),
            lt_dlerror()));
        return NULL;
    }
    if (0 != lt_dlsetsearchpath(PERFEXPERT_LIBDIR)) {
        OUTPUT(("%s [%s]", _ERROR("unable to set libtool search path"),
            lt_dlerror()));
        return NULL;
    }

    /* Load module file
     * WARNING: if this function returns "file not found" sometimes, the problem
     *          is not with the search path or so. This error could happens when
     * the module has unresolved symbols. To solve this, check if there is any
     * unresolved symbol in the module file. Sorry, but Libtool debug sucks.
     */
    PERFEXPERT_ALLOC(char, filename, (strlen(name) + 25));
    sprintf(filename, "libperfexpert_module_%s.so", name);
    handle = lt_dlopenext(filename);

    if (NULL == handle) {
        OUTPUT(("%s (%s) [%s] [%s] (Are all the symbols resolved?)",
            _ERROR("cannot load module"), lt_dlerror(), PERFEXPERT_LIBDIR,
            filename));
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

    return handle;

    MODULE_ERROR:
    lt_dlclose(handle);
    return NULL;
}

/* perfexpert_module_close */
static int perfexpert_module_close(lt_dlhandle handle, const char *name) {
    OUTPUT_VERBOSE((9, "closing module [%s]", _CYAN((char *)name)));

    lt_dlclose(handle);

    return PERFEXPERT_SUCCESS;
}

/* perfexpert_phase_add */
static int perfexpert_phase_add(perfexpert_module_t *m,
    perfexpert_step_phase_t p) {
    int compiler_present = PERFEXPERT_FALSE;
    int is_in_list = PERFEXPERT_FALSE;
    perfexpert_step_t *s = NULL;

    /* Paranoia: search for module/phase on glogal list of steps */
    perfexpert_list_for(s, &(module_globals.steps), perfexpert_step_t) {
        if ((0 == strcmp(s->name, m->name)) && (p == s->phase)) {
            is_in_list = PERFEXPERT_TRUE;
        }
        if (PERFEXPERT_PHASE_COMPILE == s->phase) {
            compiler_present = PERFEXPERT_TRUE;
        }
    }

    /* Compilers are 'singleton': only one compile module should be present */
    if ((PERFEXPERT_TRUE == compiler_present) &&
        (PERFEXPERT_PHASE_COMPILE == p)) {
        OUTPUT(("%s %s/%d", _ERROR("module/step cannot be added because there "
            "is already another compiler module/phase loaded"), m->name, p));
        return PERFEXPERT_ERROR;
    }

    /* Add module/phase to global list of steps */
    if (PERFEXPERT_FALSE == is_in_list) {
        PERFEXPERT_ALLOC(perfexpert_step_t, s, sizeof(perfexpert_step_t));
        PERFEXPERT_ALLOC(char, s->name, (strlen(m->name) + 1));
        strcpy(s->name, m->name);
        s->phase = p;
        s->module = m;
        s->status = PERFEXPERT_STEP_UNDEFINED;
        perfexpert_list_item_construct((perfexpert_list_item_t *)s);

        switch (p) {
            case PERFEXPERT_PHASE_COMPILE:
                OUTPUT_VERBOSE((9, "   adding step: %s/compile", s->name));
                s->function = m->compile;
                break;
            case PERFEXPERT_PHASE_INSTRUMENT:
                OUTPUT_VERBOSE((9, "   adding step: %s/instrument", s->name));
                s->function = m->instrument;
                break;
            case PERFEXPERT_PHASE_MEASURE:
                OUTPUT_VERBOSE((9, "   adding step: %s/measure", s->name));
                s->function = m->measure;
                break;
            case PERFEXPERT_PHASE_ANALYZE:
                OUTPUT_VERBOSE((9, "   adding step: %s/analyze", s->name));
                s->function = m->analyze;
                break;
            case PERFEXPERT_PHASE_RECOMMEND:
                OUTPUT_VERBOSE((9, "   adding step: %s/recommend", s->name));
                s->function = m->recommend;
                break;
        }

        /* In which position on the list should this module/phase be? */
        if (PERFEXPERT_TRUE == compiler_present) {
            if (PERFEXPERT_SUCCESS != perfexpert_list_insert(
                &(module_globals.steps), (perfexpert_list_item_t *)s, 1)) {
                /* If the list is empty (or with only the compiler), append */
                perfexpert_list_append(&(module_globals.steps),
                    (perfexpert_list_item_t *)s);
            }
        } else {
            perfexpert_list_prepend(&(module_globals.steps),
                (perfexpert_list_item_t *)s);
        }
    }

    return PERFEXPERT_SUCCESS;
}

/* perfexpert_step_clone */
static perfexpert_step_t* perfexpert_step_clone(perfexpert_step_t *s) {
    perfexpert_step_t *n = NULL;

    /* Sanity check: I can only clone steps which already exists! */
    if (NULL == s) {
        OUTPUT(("%s", _ERROR("attempted to clone unexistent step")));
        return NULL;
    }

    /* Clone the step */
    PERFEXPERT_ALLOC(perfexpert_step_t, n, sizeof(perfexpert_step_t));
    PERFEXPERT_ALLOC(char, n->name, (strlen(s->name) + 1));
    perfexpert_list_item_construct((perfexpert_list_item_t *)n);
    strcpy(n->name, s->name);
    n->function = s->function;
    n->status = s->status;
    n->module = s->module;
    n->phase = s->phase;

    return n;
}

// EOF
