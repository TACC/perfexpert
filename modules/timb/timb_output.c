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
#include <math.h>
#include <unistd.h>

/* Utility headers */
#include <sqlite3.h>

/* Modules headers */
#include "timb.h"
#include "timb_output.h"

/* PerfExpert common headers */
// #include "common/perfexpert_alloc.h"
#include "common/perfexpert_constants.h"
// #include "common/perfexpert_string.h"
// #include "common/perfexpert_util.h"

/* prety_print */
#define PRETTY_PRINT(size, symbol) \
    {                              \
        int i = size;              \
        while (i > 0) {            \
            printf("%s", symbol);  \
            i--;                   \
        }                          \
        printf("\n");              \
    }

/* prety_print_bar */
#define PRETTY_PRINT_BAR(size, symbol) \
    {                                  \
        int i = size;                  \
        if (50 < i) {                  \
            i = 49;                    \
            while (i > 0) {            \
                printf("%s", symbol);  \
                i--;                   \
            }                          \
            printf("+");               \
        } else {                       \
            while (i > 0) {            \
                printf("%s", symbol);  \
                i--;                   \
            }                          \
        }                              \
        printf("\n");                  \
    }




/* output_analysis */
int output_analysis(void) {
    char *error = NULL, sql[MAX_BUFFER_SIZE];

    bzero(sql, MAX_BUFFER_SIZE);
    sprintf(sql,
        "SELECT DISTINCT profile FROM hpctoolkit_hotspot WHERE perfexpert_id = %llu;",
        table, globals.unique_id);

    if (SQLITE_OK != sqlite3_exec(globals.db, sql, import_profiles,
        (void *)profiles, &error)) {
        OUTPUT(("%s %s", _ERROR("Error: SQL error"), error));
        sqlite3_free(error);
        return PERFEXPERT_ERROR;
    }




    return PERFEXPERT_SUCCESS;


    lcpi_hotspot_t *h = NULL;
    lcpi_profile_t *p = NULL;
    lcpi_module_t *m = NULL, *t = NULL;
    char *shortname = NULL;

    OUTPUT_VERBOSE((4, "%s", _YELLOW("Printing analysis report")));

    /* For each profile in the list of profiles... */
    perfexpert_list_for(p, profiles, lcpi_profile_t) {
        /* Print total runtime for this profile */
        PRETTY_PRINT(79, "-");
        printf(
            "Total running time for %s is %.2f seconds between all %d cores\n",
            _CYAN(p->name), p->cycles / database_get_hound("CPU_freq"),
            (int)sysconf(_SC_NPROCESSORS_ONLN));
        printf("The wall-clock time for %s is approximately %.2f seconds\n\n",
            _CYAN(p->name), (p->cycles / database_get_hound("CPU_freq") /
                sysconf(_SC_NPROCESSORS_ONLN)));

        /* For each module in the profile's list of modules... */
        perfexpert_hash_iter_str(p->modules_by_name, m, t) {
            perfexpert_util_filename_only(m->name, &shortname);
            m->importance = m->cycles / p->cycles;
            printf("Module %s takes %.2f%% of the total runtime\n",
                _MAGENTA(shortname), m->importance * 100);
        }
        printf("\n");

        /* For each hotspot in the profile's list of hotspots... */
        perfexpert_list_for(h, &(p->hotspots), lcpi_hotspot_t) {
            if (my_module_globals.threshold <= h->importance) {
                if (PERFEXPERT_SUCCESS != output_profile(h)) {
                    OUTPUT(("%s (%s)",
                        _ERROR("Error: printing hotspot analysis"), h->name));
                    return PERFEXPERT_ERROR;
                }
            }
        }
    }
    return PERFEXPERT_SUCCESS;
}

/* output_profile */
static int output_profile(lcpi_hotspot_t *h) {
    int print_ratio = PERFEXPERT_TRUE;
    lcpi_metric_t *l = NULL, *t = NULL;
    char *shortname = NULL;

    OUTPUT_VERBOSE((4, "   [%d] %s", h->id, _YELLOW(h->name)));

    /* Print the runtime of this hotspot */
    perfexpert_util_filename_only(h->file, &shortname);
    switch (h->type) {
        case PERFEXPERT_HOTSPOT_PROGRAM:
            printf("Aggregate (%.2f%% of the total runtime)\n",
                h->importance * 100);
            break;

        case PERFEXPERT_HOTSPOT_FUNCTION:
            printf(
                "Function %s in line %d of %s (%.2f%% of the total runtime)\n",
                _CYAN(h->name), h->line, shortname, h->importance * 100);
            break;

        case PERFEXPERT_HOTSPOT_LOOP:
            printf(
                "Loop in function %s in %s:%d (%.2f%% of the total runtime)\n",
                _CYAN(h->name), shortname, h->line, h->importance * 100);
            break;

        case PERFEXPERT_HOTSPOT_UNKNOWN:
        default:
            return PERFEXPERT_ERROR;
    }

    /* Print an horizontal double-line */
    PRETTY_PRINT(79, "=");

    /* Do we have something meaningful to show? Some warning? */
    if (LCPI_VARIANCE_LIMIT < h->variance) {
        printf("%s the instruction count variation for this bottleneck is "
            "%.2f%%, making\n         the results unreliable\n\n",
            _BOLDRED("WARNING:"), h->variance * 100);
    }
    // if (PERFEXPERT_FALSE == h->valid) {
    //     printf("%s the runtime for this code section is too short,
    // PerfExpert was unable\n         to collect the "
    //         "performance counters it needs, making the results\n"
    //         "         unreliable!\n\n", _BOLDRED("WARNING:"));
    // }
    if (database_get_hound("CPU_freq") > h->cycles) {
        printf("%s the runtime for this code section is too short to gather "
            "meaningful\n         measurements\n\n", _BOLDRED("WARNING:"));
        PRETTY_PRINT(79, "-");
        printf("\n");
        return PERFEXPERT_SUCCESS;
    }
    if (database_get_hound("CPI_threshold") >= h->cycles / h->instructions) {
        printf("%s\n\n",
            _GREEN("The performance of this code section is good!"));
    }
    // if (100 > (perfexpert_lcpi_hotspot_get(h, "ratio_floating_point") * 100)) {
    //     printf("%s this architecture overcounts floating-point operations,
    // expect to see\n         "
    //         "more than 100%% of these instructions!\n\n", _BOLDRED("WARNING:"));
    // }

    /* For each LCPI, print it's value */
    char PERCENTAGE[] = "0..........25..........50..........75..........100";
    char GOOD_BAD[] = "good.......okay........fair........poor........bad";
    print_ratio = PERFEXPERT_TRUE;
    perfexpert_hash_iter_str(h->metrics_by_name, l, t) {
        char *temp = NULL, *cat = NULL, *subcat = NULL, desc[24];

        PERFEXPERT_ALLOC(char, temp, 25);
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

        /* Print ratio and GFLOPS section */
        if ((0 == strcmp(cat, "ratio")) ||
            (0 == strncmp(cat, "GFLOPS", 6))) {
            if (PERFEXPERT_TRUE == print_ratio) {
                printf("ratio to total instrns    %%  %s\n", _CYAN(PERCENTAGE));
                print_ratio = PERFEXPERT_FALSE;
            }
            if (100 > (l->value * 100)) {
                printf("%s %4.1f ", desc, (l->value * 100));
                PRETTY_PRINT((int)rint((l->value * 50)), "*");
            } else {
                printf("%s%4.1f ", desc, (l->value * 100));
                PRETTY_PRINT(50, "*");
            }
        }

        /* Print the LCPI section */
        if (NULL == subcat) {
            PRETTY_PRINT(79, "-");
            printf("performance assessment  LCPI %s\n", _CYAN(GOOD_BAD));
        }
        if ((0 == strcmp(cat, "data accesses")) ||
            (0 == strcmp(cat, "instruction accesses")) ||
            (0 == strcmp(cat, "data TLB")) ||
            (0 == strcmp(cat, "instruction TLB")) ||
            (0 == strcmp(cat, "branch instructions")) ||
            (0 == strcmp(cat, "floating-point instr"))) {
            printf("%s%5.2f ", desc, l->value);
            PRETTY_PRINT_BAR((int)rint((l->value * 20)), ">");
        }
        /* Special colors for overall */
        if (0 == strcmp(cat, "overall")) {
            if (0.5 >= l->value) {
                printf("%s%5.2f ", _GREEN(desc), l->value);
                PRETTY_PRINT_BAR((int)rint((l->value * 20)), _GREEN(">"));
            } else if ((0.5 < l->value) && (1.0 >= l->value)) {
                printf("%s%5.2f ", _YELLOW(desc), l->value);
                PRETTY_PRINT_BAR((int)rint((l->value * 20)), _YELLOW(">"));
            } else if ((1.0 < l->value) && (2.0 >= l->value)) {
                printf("%s%5.2f ", _RED(desc), l->value);
                PRETTY_PRINT_BAR((int)rint((l->value * 20)), _RED(">"));
            } else {
                printf("%s%5.2f ", _BOLDRED(desc), l->value);
                PRETTY_PRINT_BAR((int)rint((l->value * 20)), _BOLDRED(">"));
            }
        }
        PERFEXPERT_DEALLOC(temp);
    }
    PRETTY_PRINT(79, "-");
    printf("\n");

    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
