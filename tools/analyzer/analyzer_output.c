/*
 * Copyright (c) 2013  University of Texas at Austin. All rights reserved.
 *
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * This file is part of PerfExpert.
 *
 * PerfExpert is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * PerfExpert is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with PerfExpert. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Leonardo Fialho
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

/* PerfExpert headers */
#include "analyzer.h"
#include "analyzer_profile.h"
#include "perfexpert_alloc.h"
#include "perfexpert_constants.h"
#include "perfexpert_hash.h"
#include "perfexpert_list.h"
#include "perfexpert_md5.h"
#include "perfexpert_output.h"
#include "perfexpert_string.h"

/* prety_print */
#define PRETTY_PRINT(size, symbol)                        \
    {                                                     \
        int i = size;                                     \
        while (i > 0) {                                   \
            fprintf(globals.outputfile_FP, "%s", symbol); \
            i--;                                          \
        }                                                 \
        fprintf(globals.outputfile_FP, "\n");             \
    }

/* prety_print_bar */
#define PRETTY_PRINT_BAR(size, symbol)                        \
    {                                                         \
        int i = size;                                         \
        if (50 < i) {                                         \
            i = 49;                                           \
            while (i > 0) {                                   \
                fprintf(globals.outputfile_FP, "%s", symbol); \
                i--;                                          \
            }                                                 \
            fprintf(globals.outputfile_FP, "+");              \
        } else {                                              \
            while (i > 0) {                                   \
                fprintf(globals.outputfile_FP, "%s", symbol); \
                i--;                                          \
            }                                                 \
        }                                                     \
        fprintf(globals.outputfile_FP, "\n");                 \
    }

/* output_analysis_all */
int output_analysis_all(perfexpert_list_t *profiles) {
    procedure_t *hotspot = NULL;
    profile_t *profile = NULL;

    OUTPUT_VERBOSE((4, "%s", _BLUE("Printing analysis report")));

    /* For each profile in the list of profiles... */
    profile = (profile_t *)perfexpert_list_get_first(profiles);
    while ((perfexpert_list_item_t *)profile != &(profiles->sentinel)) {
        /* Print total runtime for this profile */
        fprintf(globals.outputfile_FP,
            "Total running time for %s is %.2f seconds between all %d cores\n",
            _CYAN(profile->name),
            profile->cycles / perfexpert_machine_get("CPU_freq"),
            (int)sysconf(_SC_NPROCESSORS_ONLN));
        fprintf(globals.outputfile_FP,
            "The wall-clock time for %s is approximately %.2f seconds\n\n",
            _CYAN(profile->name), (profile->cycles /
                perfexpert_machine_get("CPU_freq") /
                sysconf(_SC_NPROCESSORS_ONLN)));

        /* For each hotspot in the profile's list of hotspots... */
        hotspot = (procedure_t *)perfexpert_list_get_first(
            &(profile->hotspots));
        while ((perfexpert_list_item_t *)hotspot !=
            &(profile->hotspots.sentinel)) {
            if (globals.threshold <= hotspot->importance) {
                if (PERFEXPERT_SUCCESS != output_analysis(profile, hotspot)) {
                    OUTPUT(("%s (%s)",
                        _ERROR("Error: printing hotspot analysis"),
                        hotspot->name));
                    return PERFEXPERT_ERROR;
                }
            }
            /* Move on */
            hotspot = (procedure_t *)perfexpert_list_get_next(hotspot);
        }
        /* Move on */
        profile = (profile_t *)perfexpert_list_get_next(profile);
    }
    return PERFEXPERT_SUCCESS;
}

/* output_analysis */
int output_analysis(profile_t *profile, procedure_t *hotspot) {
    int print_ratio = PERFEXPERT_TRUE;
    loop_t *loop = (loop_t *)hotspot;
    lcpi_t *lcpi = NULL;

    OUTPUT_VERBOSE((4, "   [%d] %s", hotspot->id, _YELLOW(hotspot->name)));

    /* Print the runtime of this hotspot */
    switch (hotspot->type) {
        case PERFEXPERT_HOTSPOT_PROGRAM:
            fprintf(globals.outputfile_FP,
                "Aggregate (%.2f%% of the total runtime)\n",
                hotspot->importance * 100);
            break;

        case PERFEXPERT_HOTSPOT_FUNCTION:
            fprintf(globals.outputfile_FP,
                "Function %s in line %d of %s (%.2f%% of the total runtime)\n",
                _CYAN(hotspot->name), hotspot->line, hotspot->file->shortname,
                hotspot->importance * 100);
            break;

        case PERFEXPERT_HOTSPOT_LOOP:
            fprintf(globals.outputfile_FP,
                "Loop in function %s in %s:%d (%.2f%% of the total runtime)\n",
                _CYAN(loop->procedure->name), loop->procedure->file->shortname,
                hotspot->line, hotspot->importance * 100);
            break;

        case PERFEXPERT_HOTSPOT_UNKNOWN:
        default:
            return PERFEXPERT_ERROR;
    }

    /* Print an horizontal double-line */
    PRETTY_PRINT(79, "=");

    /* Do we have something meaningful to show? */
    char UNRELIABLE[] = "making the results unreliable";
    char SHORTRUNTIME[] = "the runtime for this code section is too short";
    if (ANALYZER_VARIANCE_LIMIT < hotspot->variance) {
        fprintf(globals.outputfile_FP,
            "%s the instruction count variation is %.2f%%, %s!\n",
            _BOLDRED("WARNING:"), hotspot->variance * 100, UNRELIABLE);
        PRETTY_PRINT(79, "-");
    }
    if (PERFEXPERT_FALSE == hotspot->valid) {
        fprintf(globals.outputfile_FP, "%s %s, PerfExpert was\n",
            _BOLDRED("WARNING:"), SHORTRUNTIME);
        fprintf(globals.outputfile_FP,
            "unable to collect the performance counters it needs, %s!\n",
            UNRELIABLE);
        PRETTY_PRINT(79, "-");
    }
    if (hotspot->cycles < perfexpert_machine_get("CPU_freq")) {
        fprintf(globals.outputfile_FP,
            "%s %s to gather meaningful measurements\n", _BOLDRED("WARNING:"),
            SHORTRUNTIME);
        PRETTY_PRINT(79, "-");
        return PERFEXPERT_SUCCESS;
    }
    if (perfexpert_machine_get("CPI_threshold") >=
        hotspot->cycles / hotspot->instructions) {
        fprintf(globals.outputfile_FP, "%s\n",
            _GREEN("The performance of this code section is good!"));
        PRETTY_PRINT(79, "-");
    }

    /* For each LCPI, print it's value */
    char PERCENTAGE[] = "0..........25..........50..........75..........100";
    char GOOD_BAD[] = "good.......okay........fair........poor........bad";
    print_ratio = PERFEXPERT_TRUE;
    for (lcpi = hotspot->lcpi_by_name; lcpi != NULL;
        lcpi = lcpi->hh_str.next) {
        char *temp_str = NULL;
        char *category = NULL;
        char *subcategory = NULL;
        char description[24];

        PERFEXPERT_ALLOC(char, temp_str, 25);
        strcpy(temp_str, lcpi->name);
        perfexpert_string_replace_char(temp_str, '_', ' ');

        category = strtok(temp_str, ".");
        subcategory = strtok(NULL, ".");

        /* Format LCPI description */
        bzero(description, 24);
        if (NULL != strstr(lcpi->name, "overall")) {
            sprintf(description, "* %s", category);
        } else {
            sprintf(description, " - %s", subcategory);
        }
        while (23 > strlen(description)) {
            strcat(description, " ");
        }

        /* Print ratio and GFLOPS section */
        if ((0 == strcmp(category, "ratio")) ||
            (0 == strncmp(category, "GFLOPS", 6))) {
            if (PERFEXPERT_TRUE == print_ratio) {
                fprintf(globals.outputfile_FP,
                    "ratio to total instrns    %%  %s\n", _CYAN(PERCENTAGE));
                print_ratio = PERFEXPERT_FALSE;
            }
            if (100 > (lcpi->value * 100)) {
                fprintf(globals.outputfile_FP, "%s %4.1f ", description,
                    (lcpi->value * 100));
                PRETTY_PRINT((int)rint((lcpi->value * 50)), "*");
            } else {
                fprintf(globals.outputfile_FP, "%s100.0 ", description);
                PRETTY_PRINT(50, "*");
            }
        }

        /* Print the LCPI section */
        if (NULL == subcategory) {
            PRETTY_PRINT(79, "-");
            fprintf(globals.outputfile_FP, "performance assessment  LCPI %s\n",
                _CYAN(GOOD_BAD));
        }
        if ((0 == strcmp(category, "data accesses")) ||
            (0 == strcmp(category, "instruction accesses")) ||
            (0 == strcmp(category, "data TLB")) ||
            (0 == strcmp(category, "instruction TLB")) ||
            (0 == strcmp(category, "branch instructions")) ||
            (0 == strcmp(category, "floating-point instr"))) {
            fprintf(globals.outputfile_FP, "%s%5.2f ", description, lcpi->value);
            PRETTY_PRINT_BAR((int)rint((lcpi->value * 20)), ">");
        }
        /* Special colors for overall */
        if (0 == strcmp(category, "overall")) {
            if (0.5 >= lcpi->value) {
                fprintf(globals.outputfile_FP, "%s%5.2f ", _GREEN(description),
                    lcpi->value);
                PRETTY_PRINT_BAR((int)rint((lcpi->value * 20)), _GREEN(">"));
            } else if ((0.5 < lcpi->value) && (1.0 >= lcpi->value)) {
                fprintf(globals.outputfile_FP, "%s%5.2f ", _YELLOW(description),
                    lcpi->value);
                PRETTY_PRINT_BAR((int)rint((lcpi->value * 20)), _YELLOW(">"));
            } else if ((1.0 < lcpi->value) && (2.0 >= lcpi->value)) {
                fprintf(globals.outputfile_FP, "%s%5.2f ", _RED(description),
                    lcpi->value);
                PRETTY_PRINT_BAR((int)rint((lcpi->value * 20)), _RED(">"));
            } else {
                fprintf(globals.outputfile_FP, "%s%5.2f ", _ERROR(description),
                    lcpi->value);
                PRETTY_PRINT_BAR((int)rint((lcpi->value * 20)), _ERROR(">"));                
            }
        }
        PERFEXPERT_DEALLOC(temp_str);
    }
    PRETTY_PRINT(79, "=");
    fprintf(globals.outputfile_FP, "\n");

    return PERFEXPERT_SUCCESS;
}

/* output_metrics_all */
int output_metrics_all(perfexpert_list_t *profiles) {
    procedure_t *hotspot = NULL;
    profile_t *profile = NULL;
    FILE *file_FP;

    OUTPUT_VERBOSE((4, "%s", _BLUE("Printing metrics")));

    OUTPUT_VERBOSE((7, "   %s (%s)", _YELLOW("printing metrics to file"),
        globals.outputmetrics));
    file_FP = fopen(globals.outputmetrics, "w+");
    if (NULL == file_FP) {
        OUTPUT(("%s (%s)", _ERROR("Error: unable to open metrics output file"),
            globals.outputmetrics));
        return PERFEXPERT_ERROR;
    }

    /* For each profile in the list of profiles... */
    profile = (profile_t *)perfexpert_list_get_first(profiles);
    while ((perfexpert_list_item_t *)profile != &(profiles->sentinel)) {
        /* For each hotspot in the profile's list of hotspots... */
        hotspot = (procedure_t *)perfexpert_list_get_first(
            &(profile->hotspots));
        while ((perfexpert_list_item_t *)hotspot !=
            &(profile->hotspots.sentinel)) {
            if (globals.threshold <= hotspot->importance) {
                /* Print a 'hotspot separator' */
                fprintf(file_FP, "%%\n");

                if (PERFEXPERT_SUCCESS != output_metrics(profile, hotspot,
                    file_FP)) {
                    OUTPUT(("%s (%s)",
                        _ERROR("Error: printing hotspot analysis"),
                        hotspot->name));
                    fclose(file_FP);
                    return PERFEXPERT_ERROR;
                }
            }
            /* Move on */
            hotspot = (procedure_t *)perfexpert_list_get_next(hotspot);
        }
        /* Move on */
        profile = (profile_t *)perfexpert_list_get_next(profile);
    }
    fclose(file_FP);

    return PERFEXPERT_SUCCESS;
}

/* output_metrics */
int output_metrics(profile_t *profile, procedure_t *hotspot, FILE *file_FP) {
    loop_t *loop = (loop_t *)hotspot;
    metric_t *metric = NULL;
    lcpi_t *lcpi = NULL;

    OUTPUT_VERBOSE((4, "   [%d] %s", hotspot->id, _YELLOW(hotspot->name)));

    switch (hotspot->type) {
        case PERFEXPERT_HOTSPOT_PROGRAM:
        case PERFEXPERT_HOTSPOT_FUNCTION:
            fprintf(file_FP, "code.function_name=%s\n", hotspot->name);
            fprintf(file_FP, "code.filename=%s\n", hotspot->file->name);
            break;

        case PERFEXPERT_HOTSPOT_LOOP:
            fprintf(file_FP, "code.function_name=%s\n", loop->procedure->name);
            fprintf(file_FP, "code.filename=%s\n", loop->procedure->file->name);
            fprintf(file_FP, "code.loopdepth=%d\n", loop->depth);
            break;

        case PERFEXPERT_HOTSPOT_UNKNOWN:
        default:
            return PERFEXPERT_ERROR;
    }

    fprintf(file_FP, "code.line_number=%d\n", hotspot->line);
    fprintf(file_FP, "code.type=%d\n", hotspot->type);
    fprintf(file_FP, "code.importance=%f\n", hotspot->importance);
    fprintf(file_FP, "code.totalruntime=%f\n", profile->cycles /
        perfexpert_machine_get("CPU_freq"));
    fprintf(file_FP, "code.totalwalltime=%f\n", profile->cycles /
        perfexpert_machine_get("CPU_freq") / sysconf(_SC_NPROCESSORS_ONLN));

    for (lcpi = hotspot->lcpi_by_name; lcpi != NULL;
        lcpi = lcpi->hh_str.next) {
        fprintf(file_FP, "perfexpert.%s=%f\n", lcpi->name, lcpi->value);
    }

    for (metric = hotspot->metrics_by_name; metric != NULL;
        metric = metric->hh_str.next) {
        fprintf(file_FP, "%s.%s=%.0f\n",
            perfexpert_tool_get_prefix(globals.tool), metric->name,
            metric->value);
    }

    for (metric = globals.machine_by_name; metric != NULL;
        metric = metric->hh_str.next) {
        fprintf(file_FP, "hound.%s=%.2f\n", metric->name, metric->value);
    }

    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
