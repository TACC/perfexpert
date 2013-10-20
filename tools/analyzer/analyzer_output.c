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
#include <math.h>
#include <unistd.h>

/* PerfExpert headers */
#include "analyzer.h"
#include "analyzer_output.h"
#include "perfexpert_alloc.h"
#include "perfexpert_constants.h"
#include "perfexpert_fork.h"
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
    module_t *module = NULL, *module_temp = NULL;
    procedure_t *hotspot = NULL;
    profile_t *profile = NULL;

    OUTPUT_VERBOSE((4, "%s", _BLUE("Printing analysis report")));

    /* For each profile in the list of profiles... */
    profile = (profile_t *)perfexpert_list_get_first(profiles);
    while ((perfexpert_list_item_t *)profile != &(profiles->sentinel)) {
        /* Print total runtime for this profile */
        PRETTY_PRINT(79, "-");
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

        /* For each module in the profile's list of modules... */
        perfexpert_hash_iter_int(profile->modules_by_id, module, module_temp) {
            module->importance = module->cycles / profile->cycles;

            fprintf(globals.outputfile_FP,
                "Module %s takes %.2f%% of the total runtime\n",
                _MAGENTA(module->shortname), module->importance * 100);
        }
        PRETTY_PRINT(79, "-");
        fprintf(globals.outputfile_FP, "\n");

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
static int output_analysis(profile_t *profile, procedure_t *hotspot) {
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
    if (ANALYZER_VARIANCE_LIMIT < hotspot->variance) {
        fprintf(globals.outputfile_FP, "%s the instruction count variation is "
            "%.2f%%, making the results unreliable!\n", _BOLDRED("WARNING:"),
            hotspot->variance * 100);
        PRETTY_PRINT(79, "-");
    }
    if (PERFEXPERT_FALSE == hotspot->valid) {
        fprintf(globals.outputfile_FP, "%s the runtime for this code section "
            "is too short, PerfExpert was\nunable to collect the performance "
            "counters it needs, making the results unreliable!\n",
            _BOLDRED("WARNING:"));
        PRETTY_PRINT(79, "-");
    }
    if (perfexpert_machine_get("CPU_freq") > hotspot->cycles) {
        fprintf(globals.outputfile_FP, "%s the runtime for this code section "
            "is too short to gather meaningful measurements\n",
            _BOLDRED("WARNING:"));
        PRETTY_PRINT(79, "-");
        fprintf(globals.outputfile_FP, "\n");
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
                fprintf(globals.outputfile_FP, "%s%5.2f ",
                    _BOLDRED(description), lcpi->value);
                PRETTY_PRINT_BAR((int)rint((lcpi->value * 20)), _BOLDRED(">"));                
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
static int output_metrics(profile_t *profile, procedure_t *hotspot,
    FILE *file_FP) {
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

    /* Print compiler/language info */
    if (PERFEXPERT_SUCCESS != print_compiler_info(hotspot, file_FP)) {
        OUTPUT(("%s (%s)", _ERROR("Error: unable to print compiler/language"),
            hotspot->name));
        return PERFEXPERT_ERROR;
    }

    /* Print LCPI metrics */
    for (lcpi = hotspot->lcpi_by_name; lcpi != NULL;
        lcpi = lcpi->hh_str.next) {
        fprintf(file_FP, "perfexpert.%s=%f\n", lcpi->name, lcpi->value);
    }

    /* Print raw performance counters */
    for (metric = hotspot->metrics_by_name; metric != NULL;
        metric = metric->hh_str.next) {
        fprintf(file_FP, "%s.%s=%.0f\n", globals.toolmodule.counter_prefix,
            metric->name, metric->value);
    }

    /* Print machine properties */
    for (metric = globals.machine_by_name; metric != NULL;
        metric = metric->hh_str.next) {
        fprintf(file_FP, "hound.%s=%.2f\n", metric->name, metric->value);
    }

    return PERFEXPERT_SUCCESS;
}

/* print_compiler_info */
static int print_compiler_info(procedure_t *hotspot, FILE *output_FP) {
    loop_t *loop = (loop_t *)hotspot;
    FILE *info_FP = NULL;
    char buffer[1024];
    char *argv[3];
    test_t test;

    argv[0] = GET_COMPILER_INFO_PROGRAM;
    if ((PERFEXPERT_HOTSPOT_PROGRAM == hotspot->type) ||
        (PERFEXPERT_HOTSPOT_FUNCTION == hotspot->type)) {
        argv[1] = hotspot->module->name;

        PERFEXPERT_ALLOC(char, test.output, (strlen(globals.workdir) +
            strlen(hotspot->module->shortname) + 24));
        sprintf(test.output, "%s/get_compiler_info_%s.txt", globals.workdir,
            hotspot->module->shortname);

    } else if (PERFEXPERT_HOTSPOT_LOOP == hotspot->type) {
        argv[1] = loop->procedure->module->name;

        PERFEXPERT_ALLOC(char, test.output, (strlen(globals.workdir) +
            strlen(loop->procedure->module->shortname) + 24));
        sprintf(test.output, "%s/get_compiler_info_%s.txt", globals.workdir,
            loop->procedure->module->shortname);
    }
    test.input = NULL;
    test.info = NULL;
    argv[2] = NULL;

    /* fork_and_wait_and_pray */
    if (0 != fork_and_wait(&test, argv)) {
        OUTPUT(("   %s [%s]", _RED("error running"), argv[0]));
    } else {
        /* Open file and print it */
        info_FP = fopen(test.output, "r");
        if (NULL != info_FP) {
            while (NULL != fgets(buffer, sizeof(buffer), info_FP)) {
                fprintf(output_FP, "compiler.%s", buffer);
            }
            if (ferror(info_FP)) {
                return PERFEXPERT_ERROR;
            }
            fclose(info_FP);
        } else {
            return PERFEXPERT_ERROR;
        }
    }

    PERFEXPERT_DEALLOC(test.output);

    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
