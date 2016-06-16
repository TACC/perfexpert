/*
 * Copyright (c) 2011-2016  University of Texas at Austin. All rights reserved.
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
 * Authors: Antonio Gomez-Iglesias, Leonardo Fialho and Ashay Rane
 *
 * $HEADER$
 */

#ifdef __cplusplus
extern "C" {
#endif

/* System standard headers */
#include <stdio.h>
#include <math.h>
#include <unistd.h>

/* Utility headers */

/* Modules headers */
#include "lcpi.h"
#include "lcpi_database.h"
#include "lcpi_output.h"

/* PerfExpert common headers */
#include "common/perfexpert_alloc.h"
#include "common/perfexpert_constants.h"
#include "common/perfexpert_hash.h"
#include "common/perfexpert_list.h"
#include "common/perfexpert_string.h"
#include "common/perfexpert_util.h"

/* some pre-defined headers */
#define PRETTY_OK_BAR      "good.......okay........fair........poor........bad"
#define PRETTY_PERCENT_BAR "0..........25..........50...........75.........100"
#define PRETTY_SLOWDOWN    "Slowdown Caused By      LCPI    (interpretation varies according to the metric)"
#define PRETTY_ASSESSMENT  "Performance Assessment  LCPI  "

/* prety_print */
#define PRETTY_PRINT(size, symbol)            \
    {                                         \
        int i = size;                         \
        while (i > 0) {                       \
            printf("%s", symbol);             \
            fprintf(report_FP, "%s", symbol); \
            i--;                              \
        }                                     \
        printf("\n");                         \
        fprintf(report_FP, "\n");             \
    }

/* prety_print_bar */
#define PRETTY_PRINT_BAR(size, symbol)                \
    {                                                 \
        int n = 0;                                    \
        printf("[");                                  \
        fprintf(report_FP, "[");                      \
        for (n = 0; n < 50; n++) {                    \
            if (size > n) {                           \
                if ((n != 49) &&                      \
                    (size <= 50)) {                   \
                    printf("%s", symbol);             \
                    fprintf(report_FP, "%s", symbol); \
                } else {                              \
                    printf("+");                      \
                    fprintf(report_FP, "+");          \
                }                                     \
            } else {                                  \
                if ((0 == (n+1) % 5)                  \
                    && (n != 49)) {                   \
                    printf("%s", _BLUE("."));         \
                    fprintf(report_FP, ".");          \
                } else {                              \
                    printf(" ");                      \
                    fprintf(report_FP, " ");          \
                }                                     \
            }                                         \
        }                                             \
        printf("]\n");                                \
        fprintf(report_FP, "]\n");                    \
    }

/* output_analysis */
int output_analysis(perfexpert_list_t *profiles) {
    lcpi_hotspot_t *h = NULL;
    lcpi_profile_t *p = NULL;
    lcpi_module_t *m = NULL, *t = NULL;
    char *shortname = NULL;
    FILE *report_FP;
    char *report_FP_file;
    int task, thread, mpi_tasks, threads;

    mpi_tasks = database_get_mpi_tasks();
    threads = database_get_threads();

    OUTPUT_VERBOSE((4, "%s", _YELLOW("Printing analysis report")));

    /* Save the report on a file inside the tempdir */
    PERFEXPERT_ALLOC(char, report_FP_file, (strlen(globals.moduledir) + 15));
    sprintf(report_FP_file, "%s/report.txt", globals.moduledir);

    if (NULL == (report_FP = fopen(report_FP_file, "w"))) {
        OUTPUT(("%s (%s)", _ERROR("unable to open file"), report_FP_file));
        return PERFEXPERT_ERROR;
    }
    PERFEXPERT_DEALLOC(report_FP_file);

    for (task = 0; task < mpi_tasks; task++) {
        for (thread = 0; thread < threads; thread++) {
            /* For each profile in the list of profiles... */
            perfexpert_list_for(p, profiles, lcpi_profile_t) {
                /* Print total runtime for this profile */
                PRETTY_PRINT(81, "-");
                // printf(
                //     "Total running time for %s is %.2f seconds between all %d cores\n",
                //     _CYAN(p->name), p->cycles / database_get_hound("CPU_freq"),
                //     (int)sysconf(_SC_NPROCESSORS_ONLN));
                // printf("The wall-clock time for %s is approximately %.2f seconds\n\n",
                //     _CYAN(p->name), (p->cycles / database_get_hound("CPU_freq") /
                //         sysconf(_SC_NPROCESSORS_ONLN)));

                /* For each module in the profile's list of modules... */
                perfexpert_hash_iter_str(p->modules_by_name, m, t) {
                    perfexpert_util_filename_only(m->name, &shortname);
                    m->importance = m->cycles / p->cycles;
                    printf("Module %s takes %.2f%% of the total runtime\n",
                        _MAGENTA(shortname), m->importance * 100);
                    fprintf(report_FP, "Module %s takes %.2f%% of the total runtime\n",
                        shortname, m->importance * 100);
                }
                printf("\n");
                fprintf(report_FP, "\n");

                /* For each hotspot in the profile's list of hotspots... */
                perfexpert_list_for(h, &(p->hotspots), lcpi_hotspot_t) {
                    if (my_module_globals.threshold <= h->importance) {
                        if ((0 == strcmp("jaketown", perfexpert_string_to_lower(
                            my_module_globals.architecture))) || (0 ==strcmp("haswell", perfexpert_string_to_lower(
                            my_module_globals.architecture))) || (0 ==strcmp("knightslanding", perfexpert_string_to_lower(
                            my_module_globals.architecture)))) {
                            if (PERFEXPERT_SUCCESS != output_profile(h, report_FP, 20, task, thread)) {
                                OUTPUT(("%s (%s)", _ERROR("printing hotspot analysis"),
                                    h->name));
                               return PERFEXPERT_ERROR;
                            }
                        }
                        if (0 == strcmp("mic", perfexpert_string_to_lower(
                            my_module_globals.architecture))) {
                            if (PERFEXPERT_SUCCESS != output_profile(h, report_FP, 40, task, thread)) {
                                OUTPUT(("%s (%s)", _ERROR("printing hotspot analysis"),
                                    h->name));
                                return PERFEXPERT_ERROR;
                            }
                        }
                    }
                }
            }
            if (globals.output_mode==SERIAL_OUTPUT) {
                break;
            }
        } /* threads */
        if (globals.output_mode!=SERIAL_OUTPUT) {
            break;
        }
    } /* tasks */

    /* Close file */
    fclose(report_FP);


    return PERFEXPERT_SUCCESS;
}

/* output_profile */
static int output_profile(lcpi_hotspot_t *h, FILE *report_FP, const int scale, const int task, const int thread) {
    int print_ratio = PERFEXPERT_TRUE, warn_fp_ratio = PERFEXPERT_FALSE;
    lcpi_metric_t *l = NULL, *t = NULL;
    char *shortname = NULL;
    int donotshowtop = PERFEXPERT_FALSE;

    OUTPUT_VERBOSE((4, "   [%d] %s", h->id, _YELLOW(h->name)));
    /* Print the runtime of this hotspot */
    if (PERFEXPERT_SUCCESS != perfexpert_util_file_exists(h->file)) {
        OUTPUT(("   [%d -- %s] file %s does not exist. Is it a system file?", h->id, h->name, _YELLOW(h->file)));
        donotshowtop = PERFEXPERT_TRUE;
    }



    if (!donotshowtop) {
        perfexpert_util_filename_only(h->file, &shortname);
        switch (h->type) {
            case PERFEXPERT_HOTSPOT_PROGRAM:
                printf("Aggregate (%.2f%% of the total runtime)\n",
                    h->importance * 100);
                fprintf(report_FP, "Aggregate (%.2f%% of the total runtime)\n",
                    h->importance * 100);
                break;

            case PERFEXPERT_HOTSPOT_FUNCTION:
                if (globals.output_mode==SERIAL_OUTPUT) {
                    printf(
                        "Function %s in line %d of %s (%.2f%% of the total runtime)\n",
                        _CYAN(h->name), h->line, shortname, h->importance * 100);
                    fprintf(report_FP,
                        "Function %s in line %d of %s (%.2f%% of the total runtime)\n",
                        h->name, h->line, shortname, h->importance * 100);
                }
                else {
                    printf(
                        "[MPI %d / Thread %d] Function %s in line %d of %s (%.2f%% of the total runtime)\n",
                        task, thread, _CYAN(h->name), h->line, shortname, h->importance * 100);
                    fprintf(report_FP,
                        "[MPI %d / Thread %d] Function %s in line %d of %s (%.2f%% of the total runtime)\n",
                        task, thread, h->name, h->line, shortname, h->importance * 100);
                }
                break;

            case PERFEXPERT_HOTSPOT_LOOP:
                if (globals.output_mode==SERIAL_OUTPUT) {
                    printf(
                        "Loop in function %s in %s:%d (%.2f%% of the total runtime)\n",
                        _CYAN(h->name), shortname, h->line, h->importance * 100);
                    fprintf(report_FP,
                        "Loop in function %s in %s:%d (%.2f%% of the total runtime)\n",
                        h->name, shortname, h->line, h->importance * 100);
                }
                else {
                    printf(
                        "[MPI %d / Thread %d] Loop in function %s in %s:%d (%.2f%% of the total runtime)\n",
                        task, thread, _CYAN(h->name), shortname, h->line, h->importance * 100);
                    fprintf(report_FP,
                        "[MPI %d / Thread%d] Loop in function %s in %s:%d (%.2f%% of the total runtime)\n",
                        task, thread, h->name, shortname, h->line, h->importance * 100);
                }
                break;

            case PERFEXPERT_HOTSPOT_UNKNOWN:
            default:
                return PERFEXPERT_ERROR;
        }
        PERFEXPERT_DEALLOC(shortname);
    }

    /* Print an horizontal double-line */
    PRETTY_PRINT(81, "=");

    /* For each metric... */
    perfexpert_hash_iter_str(h->metrics_by_name, l, t) {
        char *temp = NULL, *cat = NULL, *subcat = NULL, desc[24];

        if (((l->mpi_task != task) || (l->thread_id != thread)) && globals.output_mode != SERIAL_OUTPUT)
            continue;
        
        PERFEXPERT_ALLOC(char, temp, (strlen(l->name) + 1));
        strcpy(temp, l->name);
        perfexpert_string_replace_char(temp, '_', ' ');

        cat = strtok(temp, ".");
        subcat = strtok(NULL, ".");

        /* Format LCPI description */
        bzero(desc, 24);
        if (NULL != strstr(l->name, "overall")) {
            sprintf(desc, "* %s", cat);
        } else {
            sprintf(desc, " - %s", subcat);
        }
        while (23 > strlen(desc)) {
            strcat(desc, " ");
        }

        /* Print ratio and GFLOPS section (percentage: scale = 100) */
        if ((0 == strcmp(cat, "ratio")) ||
            (0 == strncmp(cat, "GFLOPS", 6))) {
            if (PERFEXPERT_TRUE == print_ratio) {
                printf("%s", _WHITE("Instructions Ratio        %   "));
                printf("%s\n", _CYAN(PRETTY_PERCENT_BAR));
                fprintf(report_FP, "Instructions Ratio        %%   ");
                fprintf(report_FP, "%s\n", PRETTY_PERCENT_BAR);
                print_ratio = PERFEXPERT_FALSE;
            }
            if (100 > (l->value * 100)) {
                printf("%s %4.1f ", desc, (l->value * 100));
                fprintf(report_FP, "%s %4.1f ", desc, (l->value * 100));
                PRETTY_PRINT_BAR((int)rint((l->value * 50)), ">");
            } else {
                printf("%s%4.1f ", desc, (l->value * 100));
                fprintf(report_FP, "%s%4.1f ", desc, (l->value * 100));
                PRETTY_PRINT_BAR(50, ">");
                warn_fp_ratio = PERFEXPERT_TRUE;
            }
        }

        /* Print LCPI section: special colors for overall */
        if (0 == strcmp(cat, "overall")) {
            printf("\n%s", _WHITE(PRETTY_ASSESSMENT));
            printf("%s\n", _CYAN(PRETTY_OK_BAR));
            fprintf(report_FP, "\n%s", PRETTY_ASSESSMENT);
            fprintf(report_FP, "%s\n", PRETTY_OK_BAR);
            if (0.5 >= l->value) {
                printf("%s%5.2f ", _GREEN(desc), l->value);
                fprintf(report_FP, "%s%5.2f ", desc, l->value);
                PRETTY_PRINT_BAR((int)rint((l->value * scale)), _GREEN(">"));
            } else if ((0.5 < l->value) && (1.5 >= l->value)) {
                printf("%s%5.2f ", _YELLOW(desc), l->value);
                fprintf(report_FP, "%s%5.2f ", desc, l->value);
                PRETTY_PRINT_BAR((int)rint((l->value * scale)), _YELLOW(">"));
            } else if ((1.5 < l->value) && (2.5 >= l->value)) {
                printf("%s%5.2f ", _RED(desc), l->value);
                fprintf(report_FP, "%s%5.2f ", desc, l->value);
                PRETTY_PRINT_BAR((int)rint((l->value * scale)), _RED(">"));
            } else {
                printf("%s%5.2f ", _BOLDRED(desc), l->value);
                fprintf(report_FP, "%s%5.2f ", desc, l->value);
                PRETTY_PRINT_BAR((int)rint((l->value * scale)), _BOLDRED(">"));
            }
            printf("\n%s\n", _WHITE(PRETTY_SLOWDOWN));
            fprintf(report_FP, "\n%s\n", PRETTY_SLOWDOWN);
        } else if ((0 == strcmp(cat, "data accesses")) ||
            (0 == strcmp(cat, "vectorization level")) ||
            (0 == strcmp(cat, "instruction accesses")) ||
            (0 == strcmp(cat, "data TLB")) ||
            (0 == strcmp(cat, "instruction TLB")) ||
            (0 == strcmp(cat, "branch instructions")) ||
            (0 == strcmp(cat, "FP instructions")) || 
            (0 == strcmp(cat, "mcdram"))) {
            printf("%s%5.2f ", desc, l->value);
            fprintf(report_FP, "%s%5.2f ", desc, l->value);
            PRETTY_PRINT_BAR((int)rint((l->value * scale)), ">");
        } else if (0 == strcmp(cat, "memory bandwidth")) {
            printf("%s%5.1f ", desc, l->value * 100);
            fprintf(report_FP, "%s%5.1f ", desc, l->value * 100);
            PRETTY_PRINT_BAR((int)rint((l->value * scale)), ">");
        }

        PERFEXPERT_DEALLOC(temp);
    }

    /* Do we have something meaningful to show? Some warning? */
    if (LCPI_VARIANCE_LIMIT < h->variance) {
        printf("\n%s the instruction count variation for this bottleneck is "
            "%.2f%%, making\n         the results unreliable!",
            _BOLDRED("WARNING:"), h->variance * 100);
        fprintf(report_FP, "\nWARNING: the instruction count variation for this"
            " bottleneck is %.2f%%, making\n         the results unreliable!",
            h->variance * 100);
    }
    if ((0 == h->cycles) || (0 == h->instructions)) {
        printf("\n%s the runtime for this code section is too short, PerfExpert"
            " was unable\n         to collect the performance counters it needs"
            ", making the results\n         unreliable!",
            _BOLDRED("WARNING:"));
        fprintf(report_FP, "\nWARNING: the runtime for this code section is too"
            " short, PerfExpert was unable\n         to collect the performance"
            " counters it needs, making the results\n         unreliable!");
    }
    if (database_get_hound("CPU_freq") > h->cycles) {
        printf("\n%s the runtime for this code section is too short to gather "
            "meaningful\n         measurements!\n", _BOLDRED("WARNING:"));
        fprintf(report_FP, "\nWARNING: the runtime for this code section is too"
            " short to gather meaningful\n         measurements!\n");
        PRETTY_PRINT(81, "-");
        printf("\n");
        fprintf(report_FP, "\n");
        return PERFEXPERT_SUCCESS;
    }
    if (database_get_hound("CPI_threshold") >= h->cycles / h->instructions) {
        printf("\n%s  this code section performs just fine!",
            _BOLDGREEN("NOTICE:"));
        fprintf(report_FP, "\nNOTICE:  this code section performs just fine!");
    }
    if (PERFEXPERT_TRUE == warn_fp_ratio) {
        printf("\n%s this architecture overcounts floating-point operations, "
            "expect to see\n         more than 100%% of these instructions!",
            _BOLDRED("WARNING:"));
        fprintf(report_FP, "\nWARNING: this architecture overcounts floating-"
            "point operations, expect to see\n         more than 100%% of these"
            " instructions!");
    }
    printf("\n");
    fprintf(report_FP, "\n");
    PRETTY_PRINT(81, "-");
    printf("\n");
    fprintf(report_FP, "\n");

    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
