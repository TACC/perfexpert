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
#include <stdlib.h>
#include <string.h>
#include <argp.h>
#include <dirent.h>

/* Tools headers */
#include "perfexpert.h"
#include "perfexpert_types.h"
#include "perfexpert_options.h"

/* Modules headers */
#include "modules/perfexpert_module_base.h"

/* PerfExpert common headers */
#include "common/perfexpert_alloc.h"
#include "common/perfexpert_constants.h"
#include "common/perfexpert_list.h"
#include "common/perfexpert_output.h"
#include "common/perfexpert_string.h"
#include "common/perfexpert_util.h"
#include "install_dirs.h"

static struct argp argp = { options, parse_options, args_doc, doc };
static arg_options_t arg_options = { 0 };

/* parse_cli_params */
int parse_cli_params(int argc, char *argv[]) {
    int i = 0, k = 0, have_compiler_phase = PERFEXPERT_FALSE;
    perfexpert_module_t *m = NULL;
    perfexpert_step_t *s = NULL;

    /* Set default values for globals */
    arg_options = (arg_options_t) {
        .modules           = NULL,
        .program           = NULL,
        .program_argv      = NULL,
        .program_argv_temp = { 0 }
    };

    /* If some environment variable is defined, use it! */
    if (PERFEXPERT_SUCCESS != parse_env_vars()) {
        OUTPUT(("%s", _ERROR("parsing environment variables")));
        return PERFEXPERT_ERROR;
    }

    /* Parse arguments */
    argp_parse(&argp, argc, argv, 0, 0, NULL);

    /* Expand program arguments */
    while (NULL != arg_options.program_argv[i]) {
        int j = 0;

        perfexpert_string_split(perfexpert_string_remove_spaces(
            arg_options.program_argv[i]), arg_options.program_argv_temp, ' ');
        while (NULL != arg_options.program_argv_temp[j]) {
            globals.program_argv[k] = arg_options.program_argv_temp[j];
            j++;
            k++;
        }
        i++;
    }

    /* Sanity check: verbose level should be between 1-10 */
    if ((0 > globals.verbose) || (10 < globals.verbose)) {
        OUTPUT(("%s", _ERROR("invalid verbose level")));
        return PERFEXPERT_ERROR;
    }

    /* Sanity check: NULL program */
    if (NULL != arg_options.program) {
        if (PERFEXPERT_SUCCESS != perfexpert_util_filename_only(
            arg_options.program, &(globals.program))) {
            OUTPUT(("%s", _ERROR("unable to extract program name")));
            return PERFEXPERT_ERROR;
        }
        OUTPUT_VERBOSE((1, "   program only=[%s]", globals.program));

        if (PERFEXPERT_SUCCESS != perfexpert_util_path_only(arg_options.program,
            &(globals.program_path))) {
            OUTPUT(("%s", _ERROR("unable to extract program path")));
            return PERFEXPERT_ERROR;
        }
        OUTPUT_VERBOSE((1, "   program path=[%s]", globals.program_path));

        PERFEXPERT_ALLOC(char, globals.program_full,
            (strlen(globals.program) + strlen(globals.program_path) + 2));
        sprintf(globals.program_full, "%s/%s", globals.program_path,
            globals.program);
        OUTPUT_VERBOSE((1, "   program full path=[%s]", globals.program_full));

        /* If there is no compile module, the binary should already exist */
        perfexpert_list_for(s, &(module_globals.steps), perfexpert_step_t) {
            if (PERFEXPERT_PHASE_COMPILE == s->phase) {
                have_compiler_phase = PERFEXPERT_TRUE;
            }
        }
        if ((PERFEXPERT_PHASE_COMPILE == PERFEXPERT_FALSE) &&
            (PERFEXPERT_SUCCESS != perfexpert_util_file_is_exec(
                globals.program_full))) {
            OUTPUT(("%s (%s)", _ERROR("unable to find program"),
                globals.program_full));
            return PERFEXPERT_ERROR;
        }
    } else {
        OUTPUT(("%s", _ERROR("undefined program")));
        return PERFEXPERT_ERROR;
    }

    OUTPUT(("%s", _BLUE("Summary of options")));
    OUTPUT_VERBOSE((7, "   verbose level:      %d", globals.verbose));
    OUTPUT_VERBOSE((7, "   colorful verbose?   %s", globals.colorful ?
        "yes" : "no"));
    OUTPUT_VERBOSE((7, "   leave garbage?      %s", globals.remove_garbage ?
        "yes" : "no"));
    OUTPUT_VERBOSE((7, "   database file:      %s", globals.dbfile));
    OUTPUT(("   %s %s", _YELLOW("program executable:"), globals.program));
    OUTPUT(("   %s       %s", _YELLOW("program path:"), globals.program_path));
    OUTPUT(("   %s  %s", _YELLOW("program full path:"), globals.program_full));

    printf("%s    %s ", PROGRAM_PREFIX, _YELLOW("program arguments:"));
    if (NULL == globals.program_argv[0]) {
        printf(" (null)");
    } else {
        i = 0;
        while (NULL != globals.program_argv[i]) {
            printf(" [%s]",
                globals.program_argv[i] ? globals.program_argv[i] : "(null)");
            i++;
        }
    }
    printf("\n");

    if (7 <= globals.verbose) {
        printf("%s    Modules:           ", PROGRAM_PREFIX);
        perfexpert_list_for(m, &(module_globals.modules), perfexpert_module_t) {
            printf(" [%s]", m->name);
        }
        printf("\n%s    Steps order:       ", PROGRAM_PREFIX);
        perfexpert_list_for(s, &(module_globals.steps), perfexpert_step_t) {
            printf(" [%s/%d]", s->name, s->phase);
        }
        printf("\n");
    }

    /* Not using OUTPUT_VERBOSE because I want only one line */
    if (8 <= globals.verbose) {
        i = 0;
        printf("%s    %s", PROGRAM_PREFIX, _YELLOW("command line:"));
        for (i = 0; i < argc; i++) {
            printf(" [%s]", argv[i] ? argv[i] : "(null)");
        }
        printf("\n");
    }
    fflush(stdout);

    return PERFEXPERT_SUCCESS;
}

/* parse_options */
static error_t parse_options(int key, char *arg, struct argp_state *state) {
    perfexpert_module_t *m = NULL;
    char str[MAX_BUFFER_SIZE];

    switch (key) {
        /* Activate colorful mode */
        case 'c':
            globals.colorful = PERFEXPERT_TRUE;
            OUTPUT_VERBOSE((1, "option 'c' set"));
            break;

        /* Which database file? */
        case 'd':
            globals.dbfile = arg;
            OUTPUT_VERBOSE((1, "option 'd' set [%s]", globals.dbfile ?
                globals.dbfile : "(null)"));
            break;

        /* Leave the garbage there? */
        case 'g':
            globals.remove_garbage = PERFEXPERT_TRUE;
            OUTPUT_VERBOSE((1, "option 'g' set"));
            break;

        /* Show help */
        case 'h':
            OUTPUT_VERBOSE((1, "option 'h' set"));
            argp_state_help(state, stdout, ARGP_HELP_STD_HELP);
            break;

        case 'i':
            bzero(str, MAX_BUFFER_SIZE);
            sprintf(str, "hpctoolkit,inputfile=%s", arg ? arg : "(null)");
            if (PERFEXPERT_SUCCESS != set_module_option(str)) {
                return PERFEXPERT_ERROR;
            }
            break;

        /* Verbose level (has an alias: v) */
        case 'l':
            globals.verbose = arg ? atoi(arg) : 5;
            OUTPUT_VERBOSE((1, "option 'l' set [%d]", globals.verbose));
            break;

        /* List of modules */
        case 'M':
            OUTPUT_VERBOSE((1, "option 'M' set [%s]", arg ? arg : "(null)"));
            if (PERFEXPERT_SUCCESS != load_module(arg)) {
                exit(1);
            }
            break;

        /* Set module option */
        case 'O':
            OUTPUT_VERBOSE((1, "option 'O' set [%s]", arg ? arg : "(null)"));
            if (PERFEXPERT_SUCCESS != set_module_option(arg)) {
                exit(1);
            }
            break;

        /* Verbose level (has an alias: l) */
        case 'v':
            globals.verbose = arg ? atoi(arg) : 5;
            OUTPUT_VERBOSE((1, "option 'v' set [%d]", globals.verbose));
            break;

        /* Show modules help */
        case -4:
            OUTPUT_VERBOSE((1, "option 'module-help' set [%s]",
                arg ? arg : "(null)"));
            module_help(arg);
            break;

        /* Shortcuts for compile modules (m) */
        case 'r':
            bzero(str, MAX_BUFFER_SIZE);
            sprintf(str, "sqlrules,recommendations=%s", arg ? arg : "");
            if (PERFEXPERT_SUCCESS != set_module_option(str)) {
                return PERFEXPERT_ERROR;
            }
            break;

        case 'm':
            bzero(str, MAX_BUFFER_SIZE);
            sprintf(str, "make,target=%s", arg ? arg : "");
            if (PERFEXPERT_SUCCESS != set_module_option(str)) {
                return PERFEXPERT_ERROR;
            }
            break;

        case 's':
            if (NULL == (m = guess_compile_module())) {
                OUTPUT(("%s", _ERROR("unable to guess compiler, set CC")));
                return PERFEXPERT_ERROR;
            } else {

            }
            bzero(str, MAX_BUFFER_SIZE);
            sprintf(str, "%s,source=%s", m->name, arg ? arg : "");
            if (PERFEXPERT_SUCCESS != set_module_option(str)) {
                return PERFEXPERT_ERROR;
            }
            break;

        /* Shortcuts for HPCToolkit module (a, A, b, B, C, i, p, P) */
        case 'a':
            bzero(str, MAX_BUFFER_SIZE);
            sprintf(str, "hpctoolkit,after=%s", arg ? arg : "");
            if (PERFEXPERT_SUCCESS != set_module_option(str)) {
                return PERFEXPERT_ERROR;
            }
            break;

        case 'A':
            bzero(str, MAX_BUFFER_SIZE);
            sprintf(str, "hpctoolkit,mic-after=%s", arg ? arg : "(null)");
            if (PERFEXPERT_SUCCESS != set_module_option(str)) {
                return PERFEXPERT_ERROR;
            }
            break;

        case 'b':
            bzero(str, MAX_BUFFER_SIZE);
            sprintf(str, "hpctoolkit,before=%s", arg ? arg : "");
            if (PERFEXPERT_SUCCESS != set_module_option(str)) {
                return PERFEXPERT_ERROR;
            }
            break;

        case 'B':
            bzero(str, MAX_BUFFER_SIZE);
            sprintf(str, "hpctoolkit,mic-before=%s", arg ? arg : "");
            if (PERFEXPERT_SUCCESS != set_module_option(str)) {
                return PERFEXPERT_ERROR;
            }
            break;

        case 'C':
            bzero(str, MAX_BUFFER_SIZE);
            sprintf(str, "hpctoolkit,mic-card=%s", arg ? arg : "");
            if (PERFEXPERT_SUCCESS != set_module_option(str)) {
                return PERFEXPERT_ERROR;
            }
            break;

        case 'p':
            bzero(str, MAX_BUFFER_SIZE);
            sprintf(str, "hpctoolkit,prefix=%s", arg ? arg : "");
            if (PERFEXPERT_SUCCESS != set_module_option(str)) {
                return PERFEXPERT_ERROR;
            }
            break;

        case 'P':
            bzero(str, MAX_BUFFER_SIZE);
            sprintf(str, "hpctoolkit,mic-prefix=%s", arg ? arg : "");
            if (PERFEXPERT_SUCCESS != set_module_option(str)) {
                return PERFEXPERT_ERROR;
            }
            break;

        /* Arguments: threshold and target program and it's arguments */
        case ARGP_KEY_ARG:
            if ((('0' == arg[0]) || ('1' == arg[0])) && (0 == state->arg_num)) {
                bzero(str, MAX_BUFFER_SIZE);
                sprintf(str, "lcpi,threshold=%s", arg ? arg : "(null)");
                OUTPUT_VERBOSE((1, "option 'threshold' set [%s]",
                    arg ? arg : "(null)"));
                if (PERFEXPERT_SUCCESS != set_module_option(str)) {
                    return PERFEXPERT_ERROR;
                }
                break;
            }
            arg_options.program = arg ? arg : NULL;
            OUTPUT_VERBOSE((1, "option 'target_program' set [%s]",
                arg_options.program));
            arg_options.program_argv = &state->argv[state->next];
            OUTPUT_VERBOSE((1, "option 'program_arguments' set"));
            state->next = state->argc;
            break;

        /* If the arguments are missing */
        case ARGP_KEY_NO_ARGS:
            argp_usage(state);

        /* Too few options */
        case ARGP_KEY_END:
            break;

        /* Unknown option */
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

/* parse_env_vars */
static int parse_env_vars(void) {
    if (NULL != getenv("PERFEXPERT_VERBOSE_LEVEL")) {
        globals.verbose = atoi(getenv("PERFEXPERT_VERBOSE_LEVEL"));
        OUTPUT_VERBOSE((1, "ENV: verbose_level=%d", globals.verbose));
    }

    if (NULL != getenv("PERFEXPERT_DATABASE_FILE")) {
        globals.dbfile = getenv("PERFEXPERT_DATABASE_FILE");
        OUTPUT_VERBOSE((1, "ENV: dbfile=%s", globals.dbfile));
    }

    if ((NULL != getenv("PERFEXPERT_COLORFUL")) &&
        (1 == atoi(getenv("PERFEXPERT_COLORFUL")))) {
        globals.colorful = PERFEXPERT_FALSE;
        OUTPUT_VERBOSE((1, "ENV: colorful=YES"));
    }

    if (NULL != getenv("PERFEXPERT_MODULES")) {
        arg_options.modules = getenv("PERFEXPERT_MODULES");
        OUTPUT_VERBOSE((1, "ENV: tool=%s", arg_options.modules));
    }

    return PERFEXPERT_SUCCESS;
}

/* load_module */
static int load_module(char *module) {
    char *names[MAX_ARGUMENTS_COUNT];
    int i = 0;

    /* Expand list of modules */
    perfexpert_string_split(module, names, ',');
    while (NULL != names[i]) {
        if (PERFEXPERT_FALSE == perfexpert_module_available(names[i])) {
            if (PERFEXPERT_SUCCESS != perfexpert_module_load(names[i])) {
                OUTPUT(("%s [%s]", _ERROR("while adding module"), names[i]));
                return PERFEXPERT_ERROR;
            }
        }
        PERFEXPERT_DEALLOC(names[i]);
        i++;
    }
    return PERFEXPERT_SUCCESS;
}

/* set_module_option */
static int set_module_option(char *option) {
    char *options[MAX_ARGUMENTS_COUNT];
    int i = 0;

    /* Expand list of modules options */
    perfexpert_string_split(option, options, ',');
    while ((NULL != options[i]) && (NULL != options[i + 1])) {
        if (PERFEXPERT_SUCCESS != perfexpert_module_set_option(options[i],
            options[i + 1])) {
            OUTPUT(("%s [%s,%s]", _ERROR("while setting module options"),
                options[i], options[i + 1]));
            return PERFEXPERT_ERROR;
        }
        OUTPUT_VERBOSE((1, "module [%s] option '%s' set", _CYAN(options[i]),
            options[i + 1]));
        PERFEXPERT_DEALLOC(options[i]);
        PERFEXPERT_DEALLOC(options[i + 1]);
        i = i + 2;
    }
    return PERFEXPERT_SUCCESS;
}

/* guess_compile_module */
static perfexpert_module_t* guess_compile_module(void) {
    perfexpert_module_t *m = NULL;
    char str[MAX_BUFFER_SIZE], name[MAX_BUFFER_SIZE], *cc;
    FILE *fp;

    /* Read the environment variable */
    bzero(cc, MAX_BUFFER_SIZE);
    if (NULL == (cc = getenv("CC"))) {
        return NULL;
    }

    /* Identify the compiler from the --version output */
    bzero(str, MAX_BUFFER_SIZE);
    sprintf(str, "%s --version 2>&1", cc);
    fp = popen(str, "r");

    if (NULL == fp) {
        OUTPUT(("%s", _ERROR("error running 'CC --version'")));
        return NULL;
    }

    bzero(str, MAX_BUFFER_SIZE);
    bzero(name, MAX_BUFFER_SIZE);
    if (NULL != fgets(str, sizeof(str), fp)) {
        if (NULL != strstr(str, "gcc (GCC)")) {
            strcpy(name, "gcc");
        }
        if (NULL != strstr(str, "g++ (GCC)")) {
            strcpy(name, "gpp");
        }
        if (NULL != strstr(str, "GNU Fortran (GCC)")) {
            strcpy(name, "gcc");
        }
        if (NULL != strstr(str, "icc (ICC)")) {
            strcpy(name, "icc");
        }
        if (NULL != strstr(str, "icpc (ICC)")) {
            strcpy(name, "icpc");
        }
        if (NULL != strstr(str, "ifort (IFORT)")) {
            strcpy(name, "ifort");
        }
    }

    pclose(fp);

    if (0 == strlen(name)) {
        OUTPUT(("%s", _ERROR("unknown or unsupported compiler")));
        return NULL;
    }

    /* Check if module is installed */
    if (PERFEXPERT_FALSE == perfexpert_module_installed(name)) {
        OUTPUT(("%s [%s]", _ERROR("module not installed"), name));
        return NULL;
    }

    /* Load module */
    if (PERFEXPERT_SUCCESS != perfexpert_module_load(name)) {
        OUTPUT(("%s [%s]", _ERROR("error loading compile module"), name));
        return NULL;
    }

    /* Get module pointer */
    if (NULL == (m = perfexpert_module_get(name))) {
        OUTPUT(("%s [%s]", _ERROR("compile module not available"), name));
        return NULL;
    }

    /* Set compiler program */
    bzero(str, MAX_BUFFER_SIZE);
    sprintf(str, "%s,program=%s", m->name, cc);
    if (PERFEXPERT_SUCCESS != set_module_option(str)) {
        return NULL;
    }

    return m;
}

/* module_help */
static void module_help(const char *name) {
    struct dirent *entry = NULL;
    DIR *directory = NULL;
    int x = 0, y = 0;
    char *m = NULL;

    /* Show my help message */
    argp_help(&argp, stdout, ARGP_HELP_STD_HELP, "perfexpert");

    /* Show module's help messages */
    if (0 == strcmp("all", name)) {
        if (NULL == (directory = opendir(PERFEXPERT_LIBDIR))) {
            OUTPUT(("%s [%s]", _ERROR("unable to open libdir"),
                PERFEXPERT_LIBDIR));
            exit(0);
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

                if (PERFEXPERT_SUCCESS != perfexpert_module_load(m)) {
                    OUTPUT(("%s [%s]", _ERROR("unable to load module"), m));
                    OUTPUT(("Is %s in your LD_LIBRARY_PATH?",
                        PERFEXPERT_LIBDIR));
                    exit(0);
                }

                if (PERFEXPERT_SUCCESS != perfexpert_module_set_option(m,
                    "help")) {
                    OUTPUT(("%s", _ERROR("setting module help")));
                    exit(0);
                }

                PERFEXPERT_DEALLOC(m);
            }
        }
        if (0 != closedir(directory)) {
            OUTPUT(("%s [%s]", _ERROR("unable to close libdir")));
            exit(0);
        }
    } else {
        if (PERFEXPERT_SUCCESS != perfexpert_module_load(name)) {
            OUTPUT(("%s [%s]", _ERROR("unable to load module"), name));
            OUTPUT(("Is %s in your LD_LIBRARY_PATH?", PERFEXPERT_LIBDIR));
            exit(0);
        }
        if (PERFEXPERT_SUCCESS != perfexpert_module_set_option(name, "help")) {
            OUTPUT(("%s", _ERROR("setting module help"), name));
            exit(0);
        }
    }

    printf("\n Modules options: to set these options use "
        "--module-option=MODULE,OPTION=VALUE\n ================================"
        "=============================================\n");
    if (PERFEXPERT_SUCCESS != perfexpert_module_init()) {
        OUTPUT(("%s", _ERROR("initializing modules")));
        exit(0);
    }
    printf(" =================================================================="
        "===========\n\n");

    exit(0);
}

#ifdef __cplusplus
}
#endif

// EOF
