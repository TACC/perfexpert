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

#include <libgen.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "macvec.h"
#include "macvec_database.h"
#include "macvec_types.h"
#include "common/perfexpert_list.h"
#include "common/perfexpert_output.h"
#include "common/perfexpert_database.h"

enum { SUCCESS = 0,
    ERR_FILE,
    ERR_REGEX,
    ERR_FILE_CONTENTS,
};

#define k_var_len       64
#define k_line_len      1024
#define k_filename_len  1024
#define k_remarks_count 1024

typedef struct {
    /* For list iteration. */
    volatile perfexpert_list_item_t *next;
    volatile perfexpert_list_item_t *prev;

    char name[k_var_len];
    int line_number;
} var_t;

typedef struct {
    /* For list iteration. */
    volatile perfexpert_list_item_t *next;
    volatile perfexpert_list_item_t *prev;

    uint8_t ignored;
    long line_number;
    double importance;
    char function[k_filename_len];
    char filename[k_filename_len];

    uint32_t remark_count;
    uint32_t remarks_list[k_remarks_count];

    perfexpert_list_t var_list;
} location_t;

static regex_t re_dep_var, re_align_var;

int align_var(const char* line, location_t* location) {
    regmatch_t pmatch[3] = {0};
    OUTPUT_VERBOSE((4, "Trying to extract variable and line number from: %s.",
            line));
    if (regexec(&re_align_var, line, 3, pmatch, 0) == 0) {
        var_t *var;

        int start = pmatch[1].rm_so;
        int length = pmatch[1].rm_eo - pmatch[1].rm_so;

        if (k_filename_len < length) {
            length = k_filename_len;
        }

        char name[k_filename_len] = "";
        strncat(name, line + start, length);
        long line_number = strtol(line + pmatch[2].rm_so, NULL, 10);

        OUTPUT_VERBOSE((4, "Found variable %s, line %d.", name, line_number));

        // Add name, line location_t::var_list.

        PERFEXPERT_ALLOC(var_t, var, sizeof(var_t));

        perfexpert_list_item_construct((perfexpert_list_item_t *) var);
        snprintf(var->name, k_var_len, "%s", name);
        var->line_number = line_number;

        var_t* it_var = NULL;
        perfexpert_list_for(it_var, &(location->var_list), var_t) {
            if (strncmp(it_var->name, name, k_var_len) == 0 &&
                    it_var->line_number == line_number) {
                return;
            }
        }

        perfexpert_list_append(&(location->var_list),
                (perfexpert_list_item_t *)var);

        OUTPUT_VERBOSE((2, "List size after adding variables: %d.",
                perfexpert_list_get_size(&(location->var_list))));
        return PERFEXPERT_SUCCESS;
    }
    else {
        return PERFEXPERT_ERROR;
    }
}

int extract_var(const char* line, location_t* location) {
    regmatch_t pmatch[5] = {0};
    OUTPUT_VERBOSE((4, "Trying to extract variables and line numbers from: %s.",
            line));
    if (regexec(&re_dep_var, line, 5, pmatch, 0) == 0) {
        var_t *var_01, *var_02;

        int start = pmatch[1].rm_so;
        int length = pmatch[1].rm_eo - pmatch[1].rm_so;

        if (k_filename_len < length) {
            length = k_filename_len;
        }

        char name_01[k_filename_len] = "";
        strncat(name_01, line + start, length);
        long line_01 = strtol(line + pmatch[2].rm_so, NULL, 10);

        OUTPUT_VERBOSE((4, "Found variable %s, line %d.", name_01, line_01));

        start = pmatch[3].rm_so;
        length = pmatch[3].rm_eo - pmatch[3].rm_so;

        if (k_filename_len < length) {
            length = k_filename_len;
        }

        char name_02[k_filename_len] = "";
        strncat(name_02, line + start, length);
        long line_02 = strtol(line + pmatch[2].rm_so, NULL, 10);

        OUTPUT_VERBOSE((4, "Found variable %s, line %d.", name_02, line_02));

        // Add name_01, line_01 and name_02, line_02 to location_t::var_list.

        PERFEXPERT_ALLOC(var_t, var_01, sizeof(var_t));
        PERFEXPERT_ALLOC(var_t, var_02, sizeof(var_t));

        perfexpert_list_item_construct((perfexpert_list_item_t *) var_01);
        perfexpert_list_item_construct((perfexpert_list_item_t *) var_02);

        snprintf(var_01->name, k_var_len, "%s", name_01);
        snprintf(var_02->name, k_var_len, "%s", name_02);

        var_01->line_number = line_01;
        var_02->line_number = line_02;

        var_t* var = NULL;
        uint8_t found_1 = 0;
        uint8_t found_2 = 0;
        perfexpert_list_for(var, &(location->var_list), var_t) {
            if (strncmp(var->name, name_01, k_var_len) == 0) {
                found_1 = 1;
            }

            if (strncmp(var->name, name_02, k_var_len) == 0) {
                found_2 = 1;
            }

            if (found_1 == 1 && found_2 == 1) {
                break;
            }
        }

        if (found_1 == 0) {
            perfexpert_list_append(&(location->var_list),
                    (perfexpert_list_item_t *)var_01);
        }

        if (found_2 == 0) {
            perfexpert_list_append(&(location->var_list),
                    (perfexpert_list_item_t *)var_02);
        }

        OUTPUT_VERBOSE((2, "List size after adding variables: %d.",
                perfexpert_list_get_size(&(location->var_list))));
        return PERFEXPERT_SUCCESS;
    }
    else {
        return PERFEXPERT_ERROR;
    }
}

void extract_vars(const char* line, location_t* location) {
    if (PERFEXPERT_SUCCESS != extract_var (line, location))
        align_var(line, location);
}

var_t* find_var(perfexpert_list_t* vars, char* name, int line) {
    var_t* var;
    if (vars == NULL || name == NULL) {
        return NULL;
    }

    perfexpert_list_for(var, vars, var_t) {
        if (strncmp(var->name, name, k_var_len) == 0 &&
                line == var->line_number) {
            return (var_t*) var;
        }
    }

    return NULL;
}

location_t* find_location(perfexpert_list_t* locations, char* filename,
        int line) {
    location_t* location;
    if (locations == NULL || filename == NULL) {
        return NULL;
    }

    perfexpert_list_for(location, locations, location_t) {
        if (strncmp(location->filename, filename, k_filename_len) == 0 &&
                line == location->line_number) {
            return (location_t*) location;
        }
    }

    return NULL;
}

macvec_hotspot_t* find(perfexpert_list_t* hotspots, char* filename, int line) {
    macvec_hotspot_t* hotspot;
    if (hotspots == NULL || filename == NULL) {
        return NULL;
    }

    perfexpert_list_for(hotspot, hotspots, macvec_hotspot_t) {
        char* base_01 = basename(hotspot->file);
        char* base_02 = basename(filename);
        if (strncmp(base_01, base_02, k_filename_len) == 0 &&
                line == hotspot->line) {
            return (macvec_hotspot_t*) hotspot;
        }
    }

    return NULL;
}

int process_file(perfexpert_list_t* hotspots, FILE* stream,
    regex_t* re_loop, regex_t* re_remark, perfexpert_list_t* locations) {
    char file_line[k_line_len];
    char sql[MAX_BUFFER_SIZE];
    char *error = NULL, *description = NULL;

    perfexpert_list_t stack;
    perfexpert_list_construct(&stack);

    const char* dep_var_pattern = "remark #15346: vector dependence: assumed "
        "[A-Z]* dependence between ([^ ]*) line ([0-9]*) and ([^ ]*) line "
        "([0-9]*)";
    if (regcomp(&re_dep_var, dep_var_pattern, REG_EXTENDED) != 0) {
        return -ERR_REGEX;
    }

    const char* align_var_pattern = "remark #15389: vectorization support: "
            "reference ([^ ]*) has unaligned access[ ]* "
            "\\[ [^\\(]*\\(([0-9]*),[0-9]*\\) \\]";
    if (regcomp(&re_align_var, align_var_pattern, REG_EXTENDED) != 0) {
        regfree(&re_dep_var);
        return -ERR_REGEX;
    }

    while (fgets(file_line, k_line_len, stream)) {
        regmatch_t pmatch[3] = {0};
        if (strstr(file_line, "LOOP END") != NULL) {
            if (0 != perfexpert_list_get_size(&stack)) {
                location_t *location = (location_t*)perfexpert_list_get_last(&stack);
                if (location->ignored == 0) {
                    location_t* loc = NULL;
                    /* First search if this already exists in 'locations'. */
                    loc = find_location(locations, location->filename,
                            location->line_number);

                    if (loc == NULL) {
                        /* Copy this location into `locations'. */
                        PERFEXPERT_ALLOC(location_t, loc, sizeof(location_t));
                        memcpy(loc, location, sizeof(location_t));
                        perfexpert_list_item_construct((perfexpert_list_item_t *)loc);
                        perfexpert_list_append(locations, (perfexpert_list_item_t *)loc);

                        OUTPUT_VERBOSE((2, "Adding new hotspot: %s:%d",
                                    location->filename, location->line_number));

                        perfexpert_list_construct(&(loc->var_list));
                    } else {
                        OUTPUT_VERBOSE((2, "Found existing hotspot: %s:%d",
                                    location->filename, location->line_number));

                        // Copy remark numbers?
                        int i, j;
                        for (i=0; i<location->remark_count && i<k_remarks_count;
                                i++) {
                            for (j=0; j<loc->remark_count && j<k_remarks_count;
                                    j++) {
                                if (loc->remarks_list[j] ==
                                        location->remarks_list[i]) {
                                    break;
                                }
                            }

                            if (j<loc->remark_count && j<k_remarks_count) {
                                // Already found this remark, ignore.
                            } else if (j<k_remarks_count) {
                                // Add this remark at the end of the list.
                                loc->remarks_list[j] = location->remarks_list[i];
                                loc->remark_count += 1;
                            } else {
                                // Ignore.
                            }
                        }
                    }

                    var_t* var;
                    perfexpert_list_for(var, &(location->var_list), var_t) {
                        var_t* var_duplicate = find_var(&(loc->var_list),
                                var->name, var->line_number);
                        if (var_duplicate != NULL) {
                            continue;
                        }

                        PERFEXPERT_ALLOC(var_t, var_duplicate, sizeof(var_t));
                        snprintf(var_duplicate->name, k_var_len, "%s",
                                var->name);
                        var_duplicate->line_number = var->line_number;
                        perfexpert_list_item_construct((perfexpert_list_item_t *) var_duplicate);
                        perfexpert_list_append(&(loc->var_list), (perfexpert_list_item_t *) var_duplicate);
                    }
                }

                perfexpert_list_remove_item(&stack, (perfexpert_list_item_t *)location);
            }
        } else if (regexec(re_loop, file_line, 3, pmatch, 0) == 0) {
            int start = pmatch[1].rm_so;
            int length = pmatch[1].rm_eo - pmatch[1].rm_so;

            if (k_filename_len < length) {
                length = k_filename_len;
            }

            char _file[k_filename_len] = "";
            strncat(_file, file_line + start, length);

            long line_number = strtol(file_line + pmatch[2].rm_so, NULL, 10);
            macvec_hotspot_t* hotspot = find(hotspots, _file, line_number);

            location_t* loc;
            PERFEXPERT_ALLOC(location_t, loc, sizeof(location_t));
            snprintf(loc->filename, k_filename_len, "%s", _file);
            loc->remark_count = 0;
            loc->line_number = line_number;
            perfexpert_list_construct(&(loc->var_list));
            perfexpert_list_item_construct((perfexpert_list_item_t *)loc);
            perfexpert_list_append(&stack, (perfexpert_list_item_t *)loc);

            if (hotspot != NULL) {
                loc->ignored = 0;
                loc->importance = hotspot->importance;
                snprintf(loc->function, k_filename_len, "%s", hotspot->name);
            } else {
                loc->ignored = 1;
                loc->importance = 0;
                loc->function[0] = '\0';
            }

            OUTPUT_VERBOSE((4, "Found loop in vec report at: %s:%d.",
                    loc->filename, loc->line_number));
        } else if (0 != perfexpert_list_get_size(&stack)) {
            location_t *location = NULL;
            location = (location_t *)perfexpert_list_get_last(&stack);

            if (location->ignored == 0 &&
                regexec(re_remark, file_line, 2, pmatch, 0) == 0) {
                long remark_number = strtol(file_line + pmatch[1].rm_so, NULL,
                    10);

                /* Check if this is an "interesting" remark. */
                uint32_t i = 0;
                bzero(sql, MAX_BUFFER_SIZE);
                sprintf(sql, "SELECT description FROM macvec_rules WHERE rule=%d;", remark_number);
                if (SQLITE_OK!=sqlite3_exec(globals.db, sql, perfexpert_database_get_string, (void *) &description, &error)) {
                    OUTPUT(("%s %s", _ERROR("SQL error"), error));
                    sqlite3_free(error);
                }
                if (description==NULL) {
                    OUTPUT_VERBOSE((4, "Ignoring remark %ld because we are not "
                            "interested in tracking it.", remark_number));
                    /* Not found in interested remarks. */
                    continue;
                }

                OUTPUT_VERBOSE((4, "Found line with remark %ld.",
                        remark_number));
                extract_vars(file_line, location);

                OUTPUT_VERBOSE((4, "Looping over %d remarks.",
                        location->remark_count));
                for (i = 0; i < location->remark_count && i < k_remarks_count-1;
                        i++) {
                    if (location->remarks_list[i] == remark_number) {
                        break;
                    }
                }

                uint32_t count = location->remark_count;
                if (i < k_remarks_count && i >= count) {
                    /* Add this remark to the end. */
                    location->remarks_list[count] = remark_number;
                    location->remark_count += 1;
                    OUTPUT_VERBOSE((2, "Appended remark to list."));
                } else if (i >= k_remarks_count) {
                    /* Exhausted space for remarks, print warning. */
                    OUTPUT_VERBOSE((1, "Exhausted space for remarks, "
                            "ignoring..."));
                } else {
                    /* Found remark in list of remarks, ignore this case. */
                    OUTPUT_VERBOSE((1, "Remark %ld was already inserted in "
                            "list at location %d.", remark_number, i));
                }
            }
        }
    }

    regfree(&re_dep_var);
    regfree(&re_align_var);

    if (perfexpert_list_get_size(locations) > 0) {
        OUTPUT(("%s [%d]", _YELLOW("Location list size after matching "
            "vectorization report with hotspots:"),
            perfexpert_list_get_size(locations)));

        if (perfexpert_list_get_size(locations) > 0 ) {
            while (perfexpert_list_get_size(&stack) != 0) {
                perfexpert_list_item_t* last = NULL;
                last = perfexpert_list_get_last(&stack);
                perfexpert_list_remove_item(&stack, last);

                location_t* location = (location_t*) last;
                while (perfexpert_list_get_size(&(location->var_list)) != 0) {
                    perfexpert_list_item_t* _last = NULL;
                    _last = perfexpert_list_get_last(&(location->var_list));
                    perfexpert_list_remove_item(&(location->var_list), _last);

                    PERFEXPERT_DEALLOC(_last);
                }

                PERFEXPERT_DEALLOC(last);
            }
        }
    }
    perfexpert_list_destruct(&stack);
    return PERFEXPERT_SUCCESS;
}

int parse(perfexpert_list_t* hotspots, perfexpert_list_t* locations, char *filename) {
//    char_t *h = NULL;

    regex_t re_loop;
    const char* loop_pattern = "LOOP BEGIN at ([^\\(]*)\\(([0-9]*),([0-9]*)\\)";
    if (regcomp(&re_loop, loop_pattern, REG_EXTENDED) != 0) {
        return -ERR_REGEX;
    }

    regex_t re_remark;
    const char* remark_pattern = "   remark #([0-9]*):.*";
    if (regcomp(&re_remark, remark_pattern, REG_EXTENDED) != 0) {
        regfree(&re_loop);
        return -ERR_REGEX;
    }

    FILE* stream = NULL;
    char * report_filename;
    PERFEXPERT_ALLOC (char, report_filename, strlen(filename)+7);
    char * pch;
    pch = strrchr(filename, '.');
    memcpy(report_filename, filename, pch - filename);
    strcat(report_filename, ".optrpt");
    stream = fopen(report_filename, "r");
    if (stream == NULL) {
        OUTPUT_VERBOSE((6, "%s %s", _ERROR(" couldn't open file"), report_filename));
        PERFEXPERT_DEALLOC(report_filename);
        PERFEXPERT_ALLOC (char, report_filename, strlen(globals.moduledir)+17);
        sprintf(report_filename, "%s/make.output", globals.moduledir);
        stream = fopen(report_filename, "r");
        if (stream == NULL) {
            OUTPUT_VERBOSE((6, "%s %s", _ERROR(" couldn't open file"), report_filename));
            regfree(&re_remark);
            regfree(&re_loop);
            PERFEXPERT_DEALLOC(report_filename);
            return -ERR_FILE;
        }
    }

    PERFEXPERT_DEALLOC(report_filename);
    int err = process_file(hotspots, stream, &re_loop, &re_remark, locations);
    if (err != 0) {
        regfree(&re_remark);
        regfree(&re_loop);
        fclose(stream);

        return err;
    }

    regfree(&re_remark);
    regfree(&re_loop);
    fclose(stream);

    return PERFEXPERT_SUCCESS;
}

static void print_recommendations(perfexpert_list_t* locations) {
    location_t* location;
    char *report_FP_file;
    FILE *report_FP;
    char sql[MAX_BUFFER_SIZE];
    char *error = NULL, *description = NULL;
    
    PERFEXPERT_ALLOC(char, report_FP_file, (strlen(globals.moduledir) + 15));
    sprintf(report_FP_file, "%s/report.txt", globals.moduledir);
    if (NULL == (report_FP = fopen(report_FP_file, "a"))) {
        OUTPUT(("%s (%s)", _ERROR("unable to open file"), report_FP_file));
        return;
    }
    PERFEXPERT_DEALLOC(report_FP_file);

    /*  Make sure that we have something to print */
    if (perfexpert_list_get_size(locations)==0) {
        fclose(report_FP);
        return;
    }

    OUTPUT(("%s [%d]", _YELLOW("Printing recommendations"),
        perfexpert_list_get_size(locations)));

    perfexpert_list_for(location, locations, location_t) {
        fprintf(stdout, "\nLoop in function %s in %s:%d (%.2f%% of the total "
            "runtime)\n", location->function, location->filename,
            location->line_number, location->importance * 100);
        fprintf(report_FP, "\nLoop in function %s in %s:%d (%.2f%% of the total "
            "runtime)\n", location->function, location->filename,
            location->line_number, location->importance * 100);
            
        OUTPUT_VERBOSE((2, "Size of list containing variables: %d.",
                perfexpert_list_get_size(&(location->var_list))));

        uint32_t size = perfexpert_list_get_size(&(location->var_list));
        if (0 != size) {
            var_t* var;
            fprintf(stdout, "Affected variables: ");
            uint32_t i = 0;
            perfexpert_list_for(var, &(location->var_list), var_t) {
                fprintf(stdout, "%s at line %ld", var->name, var->line_number);
                fprintf(report_FP, "%s at line %ld", var->name, var->line_number);

                if (i != size-1) {
                    fprintf(stdout, ", ");
                    fprintf(report_FP, ", ");
                } else {
                    fprintf(stdout, ".");
                    fprintf(report_FP, ".");
                }

                i += 1;
            }

            fprintf(stdout, "\n");
            fprintf(report_FP, "\n");
        }

        OUTPUT_VERBOSE((4, "Remark count: %d.", location->remark_count));

        uint32_t i, j;
        for (i = 0; i < location->remark_count; i++) {
            uint32_t remark_number = location->remarks_list[i];
            bzero(sql, MAX_BUFFER_SIZE);
            sprintf(sql, "SELECT description FROM macvec_rules WHERE rule=%d;", remark_number);
            OUTPUT_VERBOSE((10, "  sql: %s\n", sql));
            if (SQLITE_OK!=sqlite3_exec(globals.db, sql, perfexpert_database_get_string, (void *) &description, &error)) {
                OUTPUT(("%s %s", _ERROR("SQL error"), error));
                sqlite3_free(error);
            }
            if (description == NULL)
                continue;
            fprintf(stdout, "  - %s\n", description);
            fprintf(report_FP, "  - %s\n", description);
            
            store_macvec_result(location->line_number, location->filename, description);

            PERFEXPERT_DEALLOC(description);
            description = NULL;
        }
    }

    fprintf(stdout, "\n");
    fprintf(report_FP, "\n");
    fclose(report_FP);
}

int process_hotspots(perfexpert_list_t* hotspots, char *filename) {
    perfexpert_list_t locations;
    perfexpert_list_construct(&locations);
    int err = PERFEXPERT_SUCCESS;
  
    if (PERFEXPERT_SUCCESS != (err = parse(hotspots, &locations, filename))) {
//        OUTPUT(("%s %s, error code: %d",
//            _ERROR("Failed to parse vectorization report"), filename, err));
        return PERFEXPERT_ERROR;
    }

    print_recommendations(&locations);

    while (perfexpert_list_get_size(&locations) != 0) {
        perfexpert_list_item_t* last = NULL;
        last = perfexpert_list_get_last(&locations);
        perfexpert_list_remove_item(&locations, last);

        location_t* location = (location_t*) last;
        perfexpert_list_t* var_list = &(location->var_list);
        while (perfexpert_list_get_size(var_list) != 0) {
            perfexpert_list_item_t* _last = NULL;
            _last = perfexpert_list_get_last(var_list);
            perfexpert_list_remove_item(var_list, _last);
            PERFEXPERT_DEALLOC(_last);
        }

        perfexpert_list_destruct(var_list);
        PERFEXPERT_DEALLOC(last);
    }

    perfexpert_list_destruct(&locations);
    return PERFEXPERT_SUCCESS;
}
