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

/* PerfExpert tool headers */
#include "ct.h"
#include "ct_tools.h"

/* PerfExpert common headers */
#include "common/perfexpert_alloc.h"
#include "common/perfexpert_constants.h"
#include "common/perfexpert_fork.h"
#include "common/perfexpert_list.h"
#include "common/perfexpert_log.h"
#include "common/perfexpert_output.h"

/* apply_recommendations */
int apply_recommendations(fragment_t *f) {
    int rc = PERFEXPERT_NO_TRANS;
    recommendation_t *r = NULL;

    OUTPUT_VERBOSE((4, "%s", _BLUE("Applying recommendations")));

    perfexpert_list_for(r, &(f->recommendations), recommendation_t) {
        /* Skip if other recommendation has already been applied */
        if (PERFEXPERT_SUCCESS == rc) {
            OUTPUT_VERBOSE((8, "   [%s ] [%d] (code already changed)",
                _MAGENTA("SKIP"), r->id));
            continue;
        }

        /* Apply transformations */
        switch (apply_transformations(f, r)) {
            case PERFEXPERT_ERROR:
                OUTPUT_VERBOSE((8, "   [%s] [%d]", _BOLDYELLOW("ERRO"), r->id));
                return PERFEXPERT_ERROR;

            case PERFEXPERT_FAILURE:
                OUTPUT_VERBOSE((8, "   [%s] [%d]", _BOLDRED("FAIL"), r->id));
                break;

            case PERFEXPERT_NO_TRANS:
                OUTPUT_VERBOSE((8, "   [%s ] [%d] (no transformations)",
                    _MAGENTA("SKIP"), r->id));
                break;

            case PERFEXPERT_SUCCESS:
                OUTPUT_VERBOSE((8, "   [ %s ] [%d]", _BOLDGREEN("OK"), r->id));
                rc = PERFEXPERT_SUCCESS;
                break;

            default:
                break;
        }
    }
    return rc;
}

/* apply_transformations */
static int apply_transformations(fragment_t *f, recommendation_t *r) {
    int rc = PERFEXPERT_NO_TRANS;
    transformation_t *t = NULL;

    /* Return when there is no transformations for this recommendation */
    if (0 == perfexpert_list_get_size(&(r->transformations))) {
        return PERFEXPERT_NO_TRANS;
    }

    /* For each transformation for this recommendation... */
    perfexpert_list_for(t, &(r->transformations), transformation_t) {
        /* Skip if other transformation has already been applied */
        if (PERFEXPERT_SUCCESS == rc) {
            OUTPUT_VERBOSE((8, "   [%s ] [%d] (%s)", _MAGENTA("SKIP"),
                t->id, t->program));
            continue;
        }

        /* Apply patterns */
        switch (apply_patterns(f, t)) {
            case PERFEXPERT_ERROR:
                OUTPUT_VERBOSE((8, "   [%s] [%d] (%s)", _BOLDYELLOW("ERROR"),
                    t->id, t->program));
                return PERFEXPERT_ERROR;

            case PERFEXPERT_FAILURE:
                OUTPUT_VERBOSE((8, "   [%s ] [%d] (%s)", _BOLDRED("FAIL"),
                    t->id, t->program));
                continue;

            case PERFEXPERT_NO_PAT:
                OUTPUT_VERBOSE((8, "   [%s ] [%d] (%s)", _MAGENTA("SKIP"),
                    t->id, t->program));
                continue;

            case PERFEXPERT_SUCCESS:
            default:
                break;
        }

        /* Test transformation */
        switch (test_transformation(f, t)) {
            case PERFEXPERT_ERROR:
                OUTPUT_VERBOSE((8, "   [%s] [%d] (%s)", _YELLOW("ERROR"),
                    t->id, t->program));
                return PERFEXPERT_ERROR;

            case PERFEXPERT_FAILURE:
                OUTPUT_VERBOSE((8, "   [%s ] [%d] (%s)", _RED("FAIL"),
                    t->id, t->program));
                break;

            case PERFEXPERT_SUCCESS:
                OUTPUT_VERBOSE((8, "   [ %s  ] [%d] (%s)", _BOLDGREEN("OK"),
                    t->id, t->program));
                rc = PERFEXPERT_SUCCESS;
                LOG(("transformation=%s", t->program));
                break;

            default:
                break;
        }
    }
    return rc;
}

/* apply_patterns */
static int apply_patterns(fragment_t *f, transformation_t *t) {
    int rc = PERFEXPERT_FAILURE;
    pattern_t *p = NULL;

    /* Return when there is no patterns for this transformation */
    if (0 == perfexpert_list_get_size(&(t->patterns))) {
        return PERFEXPERT_NO_PAT;
    }

    /* Did HPCToolkit identified the file */
    if (0 == strcmp("~unknown-file~", f->file)) {
        OUTPUT(("%s (%s)", _ERROR((char *)"unknown source file"), f->file));
        return PERFEXPERT_NO_PAT;
    }

    /* First of all, check if file exists */
    if (-1 == access(f->file, R_OK )) {
        OUTPUT(("%s (%s)", _ERROR((char *)"source file not found"), f->file));
        return PERFEXPERT_NO_PAT;
    }

    /* For each pattern required for this transfomation ... */
    perfexpert_list_for(p, &(t->patterns), pattern_t) {
        /* Extract fragments, for that we have to open ROSE */
        if (PERFEXPERT_ERROR == open_rose(f->file)) {
            OUTPUT(("%s", _RED("starting Rose")));
            return PERFEXPERT_FAILURE;
        } else {
            /* Hey ROSE, here we go... */
            if (PERFEXPERT_ERROR == extract_fragment(f)) {
                OUTPUT(("%s (%s:%d)", _ERROR("extracting fragments for"),
                    f->file, f->line));
            }
            /* Close Rose */
            if (PERFEXPERT_SUCCESS != close_rose()) {
                OUTPUT(("%s", _ERROR("closing Rose")));
                return PERFEXPERT_ERROR;
            }
        }

        /* Test patterns */
        switch (test_pattern(f, p)) {
            case PERFEXPERT_ERROR:
                OUTPUT_VERBOSE((7, "   [%s] [%d] (%s)", _BOLDYELLOW("ERROR"),
                    p->id, p->program));
                return PERFEXPERT_ERROR;

            case PERFEXPERT_FAILURE:
                OUTPUT_VERBOSE((7, "   [%s ] [%d] (%s)", _BOLDRED("FAIL"),
                    p->id, p->program));
                break;

            case PERFEXPERT_SUCCESS:
                OUTPUT_VERBOSE((7, "   [ %s  ] [%d] (%s)", _BOLDGREEN("OK"),
                    p->id, p->program));
                rc = PERFEXPERT_SUCCESS;
                break;

            default:
                break;
        }
    }
    return rc;
}

/* test_transformation */
static int test_transformation(fragment_t *f, transformation_t *t) {
    int rc = PERFEXPERT_SUCCESS;
    char *new_file = NULL, *argv[12];
    test_t *test = NULL;

    /* Set the code transformer arguments. Ok, we have to define an
     * interface to code transformers. Here is a simple one. Each code
     * transformer will be called using the following arguments:
     *
     * -f FUNCTION  Function name were code bottleneck belongs to
     * -l LINE      Line number identified by HPCtoolkit/PerfExpert/etc...
     * -r FILE      File to write the transformation result
     * -s FILE      Source file
     * -w DIR       Use DIR as work directory
     */
    argv[0] = t->program;
    argv[1] = "-f";
    argv[2] = f->name;
    argv[3] = "-l";
    PERFEXPERT_ALLOC(char, argv[4], 10);
    sprintf(argv[4], "%d", f->line);
    argv[5] = "-r";
    PERFEXPERT_ALLOC(char, argv[6], (strlen(f->file) + 5));
    sprintf(argv[6], "%s_new", f->file);
    argv[7] = "-s";
    argv[8] = f->file;
    argv[9] = "-w";
    argv[10] = "./";
    argv[11] = NULL;

    /* The main test */
    PERFEXPERT_ALLOC(test_t, test, sizeof(test_t));
    PERFEXPERT_ALLOC(char, test->output,
        (strlen(globals.workdir) + strlen(FRAGMENTS_DIR) + strlen(f->file) +
            strlen(t->program) + 11));
    sprintf(test->output, "%s/%s/%s.%s.output", globals.workdir,
        FRAGMENTS_DIR, f->file, t->program);
    test->input = NULL;
    test->info = f->file;

    /* Not using OUTPUT_VERBOSE because I want only one line */
    if (8 <= globals.verbose) {
        int i;
        printf("%s    %s", PROGRAM_PREFIX, _YELLOW("command line:"));
        for (i = 0; i < 11; i++) {
            printf(" %s", argv[i]);
        }
        printf("\n");
    }

    /* fork_and_wait_and_pray */
    rc = fork_and_wait(test, argv);

    /* Replace the source code file */
    PERFEXPERT_ALLOC(char, new_file, (strlen(f->file) + 10));
    sprintf(new_file, "%s.old_%d", f->file, getpid());

    if (PERFEXPERT_SUCCESS == rc) {
        if (rename(f->file, new_file)) {
            rc = PERFEXPERT_ERROR;
            goto CLEAN_UP;
        }
        if (rename(argv[6], f->file)) {
            rc = PERFEXPERT_ERROR;
            goto CLEAN_UP;
        }
        fprintf(globals.outputfile_FP, "%s was applied to line %d of %s\n",
            _CYAN(t->program), f->line,
            f->file);
        fprintf(globals.outputfile_FP, "The original file was saved as %s\n",
            _MAGENTA(new_file));
    }

    /* Free memory */
    CLEAN_UP:
    PERFEXPERT_DEALLOC(argv[4]);
    PERFEXPERT_DEALLOC(argv[6]);
    PERFEXPERT_DEALLOC(test->output);
    PERFEXPERT_DEALLOC(new_file);

    return rc;
}

/* test_pattern */
static int test_pattern(fragment_t *f, pattern_t *p) {
    int rc = PERFEXPERT_FAILURE;
    perfexpert_list_t tests;
    test_t *test = NULL;
    char *argv[2];

    /* Considering the outer loops, we can have more than one test */
    perfexpert_list_construct(&tests);

    /* The main test */
    PERFEXPERT_ALLOC(test_t, test, sizeof(test_t));
    perfexpert_list_item_construct((perfexpert_list_item_t *)test);
    perfexpert_list_append(&tests, (perfexpert_list_item_t *)test);
    PERFEXPERT_ALLOC(char, test->output,
        (strlen(f->fragment_file) + strlen(p->program) + 9));
    sprintf(test->output, "%s.%s.output", f->fragment_file, p->program);
    test->input = f->fragment_file;
    test->info = f->fragment_file;

    /* It we're testing for a loop, check for the outer loop */
    if ((PERFEXPERT_HOTSPOT_LOOP == f->type) && (2 <= f->depth) &&
        (NULL != f->outer_loop_fragment_file)) {

        /* The outer loop test */
        PERFEXPERT_ALLOC(test_t, test, sizeof(test_t));
        perfexpert_list_item_construct((perfexpert_list_item_t *)test);
        perfexpert_list_append(&tests, (perfexpert_list_item_t *)test);
        PERFEXPERT_ALLOC(char, test->output,
            (strlen(f->outer_loop_fragment_file) + strlen(p->program) + 9));
        sprintf(test->output, "%s.%s.output",
            f->outer_loop_fragment_file, p->program);
        test->input = f->outer_loop_fragment_file;
        test->info = f->outer_loop_fragment_file;

        /* And test for the outer outer loop too */
        if ((NULL != f->outer_outer_loop_fragment_file) &&
            (3 <= f->depth)) {

            /* The outer outer loop test */
            PERFEXPERT_ALLOC(test_t, test, sizeof(test_t));
            perfexpert_list_item_construct((perfexpert_list_item_t *)test);
            perfexpert_list_append(&tests, (perfexpert_list_item_t *)test);
            PERFEXPERT_ALLOC(char, test->output,
                (strlen(f->outer_outer_loop_fragment_file) +
                    strlen(p->program) + 9));
            sprintf(test->output, "%s.%s.output",
                f->outer_outer_loop_fragment_file, p->program);
            test->input = f->outer_outer_loop_fragment_file;
            test->info = f->outer_outer_loop_fragment_file;
        }
    }

    argv[0] = p->program;
    argv[1] = NULL;

    /* Run all the tests */
    while (0 < perfexpert_list_get_size(&tests)) {
        test = (test_t *)perfexpert_list_get_first(&tests);
        switch (fork_and_wait(test, (char **)argv)) {
            case PERFEXPERT_SUCCESS:
                rc = PERFEXPERT_SUCCESS;
                break;

            case PERFEXPERT_ERROR:
            case PERFEXPERT_UNDEFINED:
            case PERFEXPERT_FAILURE:
            default:
                break;
        }

        perfexpert_list_remove_item(&tests, (perfexpert_list_item_t *)test);
        PERFEXPERT_DEALLOC(test->output);
        PERFEXPERT_DEALLOC(test);
    }

    return rc;
}

#ifdef __cplusplus
}
#endif

// EOF
