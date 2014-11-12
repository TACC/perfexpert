
#include <libgen.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "macvec.h"
#include "macvec_types.h"
#include "common/perfexpert_list.h"
#include "common/perfexpert_output.h"

enum { SUCCESS = 0,
    ERR_FILE,
    ERR_REGEX,
    ERR_FILE_CONTENTS,
};

#define k_line_len      1024
#define k_filename_len  1024
#define k_remarks_count 1024

typedef struct _tag_location {
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
} location_t;

typedef struct {
    long remark_number;
    const char** message;
    const char** description;
} message_t;

const char* msg_ptr_check = "Check if pointers overlap";
const char* msg_trip_count = "Inform compiler about loop trip count";
const char* msg_mem_align = "Align memory references";
const char* msg_branch_eval = "Indicate branch evaluations";
const char* msg_float_conv = "Use limited types";

const char* dsc_ptr_check = "If the compiler is unaware that pointers do not "
        "overlap in memory, the compiler's dependence analysis may infer "
        "existance of vector dependence. If the pointers used in this loop "
        "body indeed do not overlap, declare them with the 'restrict' keyword. "
        "For instance: double* restrict ptr_a;. See "
        "https://software.intel.com/en-us/articles/vectorization-with-the-intel-compilers-part-i for additional details.";

const char* dsc_trip_count = "The compiler's vectorization cost model can be "
        "takes various factors into account, with the loop trip count being "
        "one among them. Informing the compiler about loop trip counts that "
        "cannot be statically inferred helps the compiler make better "
        "decisions about the vectorizability of the code. If loop trip count "
        "is known, add \"#pragma loop_count(n)\" directive before the loop, "
        "where 'n' is equal to the loop count. See "
        "https://software.intel.com/en-us/node/524502 for additional details.";

const char* dsc_mem_align = "Aligned memory accesses usually incur much lower "
        "penalty of access (when compared with unaligned acccesses), "
        "especially on the Intel Xeon Phi coprocessor. To align arrays and "
        "structures, use __attribute__((aligned(64))) for statically-allocated "
        "memory and _mm_malloc() for dynamically allocated memory. See "
        "https://software.intel.com/en-us/articles/data-alignment-to-assist-vectorization "
        "for additional details.";

const char* dsc_branch_eval = "Branches that evaluate to always true or always "
        "false can be indicated to the compiler to change code layout for "
        "optimization. Use the \"__builtin_expect()\" intrinsic function to "
        "indicate branch outcomes that are highly biased.";

const char* dsc_float_conv = "Using mixed precision (single- as well as double-"
        "precision) values in a loop body introduces many inefficiences in "
        "vectorized code generation. The inefficiencies arise in choosing "
        "the appropriate alignment, converting intermediate results from one "
        "format to another and in estimating the cost of vectorization. If "
        "possible, convert all floating point values to use the same "
        "precision.";

const message_t messages[13] = {
    {   15046,  &msg_ptr_check,     &dsc_ptr_check      },
    {   15344,  &msg_ptr_check,     &dsc_ptr_check      },
    {   15037,  &msg_trip_count,    &dsc_trip_count     },
    {   15017,  &msg_trip_count,    &dsc_trip_count     },
    {   15315,  &msg_trip_count,    &dsc_trip_count     },
    {   15523,  &msg_trip_count,    &dsc_trip_count     },
    {   15167,  &msg_mem_align,     &dsc_mem_align      },
    {   15524,  &msg_mem_align,     &dsc_mem_align      },
    {   15421,  &msg_mem_align,     &dsc_mem_align      },
    {   15167,  &msg_mem_align,     &dsc_mem_align      },
    {   15038,  &msg_branch_eval,   &dsc_branch_eval    },
    {   15336,  &msg_branch_eval,   &dsc_branch_eval    },
    {   15327,  &msg_float_conv,    &dsc_float_conv     },
};

const long message_count = sizeof(messages) / sizeof(message_t);

macvec_hotspot_t* find(perfexpert_list_t* hotspots, char* filename, int line) {
    macvec_hotspot_t* hotspot;
    if (hotspots == NULL || filename == NULL) {
        return NULL;
    }

    perfexpert_list_for(hotspot, hotspots, macvec_hotspot_t) {
        char* base = basename(hotspot->file);
        if (strncmp(base, filename, k_filename_len) == 0 &&
                line == hotspot->line) {
            return (macvec_hotspot_t*) hotspot;
        }
    }

    return NULL;
}

int process_file(perfexpert_list_t* hotspots, FILE* stream,
        regex_t* re_loop, regex_t* re_remark, perfexpert_list_t* locations) {
    char file_line[k_line_len];

    perfexpert_list_t stack;
    perfexpert_list_construct(&stack);

    while (fgets(file_line, k_line_len, stream)) {
        regmatch_t pmatch[3] = {0};
        if (strstr(file_line, "LOOP END") != NULL) {
            if (perfexpert_list_get_size(&stack) != 0) {
                perfexpert_list_item_t* last = NULL;
                last = perfexpert_list_get_last(&stack);
                location_t* location = (location_t*) last;
                if (location->ignored == 0) {
                    /* Copy this location into `locations'. */
                    location_t* loc;
                    PERFEXPERT_ALLOC(location_t, loc, sizeof(location_t));
                    memcpy(loc, location, sizeof(location_t));
                    perfexpert_list_item_t* list_item = NULL;
                    list_item = (perfexpert_list_item_t*) loc;
                    perfexpert_list_item_construct(list_item);
                    perfexpert_list_append(locations, list_item);
                }

                perfexpert_list_remove_item(&stack, last);
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
            if (hotspot != NULL) {
                loc->ignored = 0;
                loc->importance = hotspot->importance;
                snprintf(loc->function, k_filename_len, "%s", hotspot->name);
            } else {
                loc->ignored = 1;
                loc->importance = 0;
                loc->function[0] = '\0';
            }

            perfexpert_list_item_t* list_item = (perfexpert_list_item_t*) loc;
            perfexpert_list_item_construct(list_item);
            perfexpert_list_append(&stack, list_item);
        } else if (perfexpert_list_get_size(&stack) != 0) {
            perfexpert_list_item_t* last = NULL;
            last = perfexpert_list_get_last(&stack);
            location_t* location = (location_t*) last;
            if (location->ignored == 0 && regexec(re_remark, file_line, 2,
                        pmatch, 0) == 0) {
                long remark_number = strtol(file_line + pmatch[1].rm_so, NULL,
                        10);

                uint32_t i = 0;
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
                } else if (i >= k_remarks_count) {
                    /* Exhausted space for remarks, print warning. */
                } else {
                    /* Found remark in list of remarks, ignore this case. */
                }
            }
        }
    }

    while (perfexpert_list_get_size(&stack) != 0) {
        perfexpert_list_item_t* last = NULL;
        last = perfexpert_list_get_last(&stack);
        perfexpert_list_remove_item(&stack, last);
        PERFEXPERT_DEALLOC(last);
    }

    perfexpert_list_destruct(&stack);
    return 0;
}

int parse(perfexpert_list_t* hotspots, const char* report_filename,
        perfexpert_list_t* locations) {
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
    stream = fopen(report_filename, "r");
    if (stream == NULL) {
        regfree(&re_remark);
        regfree(&re_loop);

        return -ERR_FILE;
    }

    if (process_file(hotspots, stream, &re_loop, &re_remark, locations) != 0) {
        regfree(&re_remark);
        regfree(&re_loop);
        fclose(stream);

        return -ERR_FILE_CONTENTS;
    }

    regfree(&re_remark);
    regfree(&re_loop);
    fclose(stream);

    return SUCCESS;
}

void print_recommendations(perfexpert_list_t* locations) {
    location_t* location;
    perfexpert_list_for (location, locations, location_t) {
        fprintf(stdout, "\nLoop in function %s in %s:%d (%.2f%% of the total "
                "runtime)\n", location->function, location->filename,
                location->line_number, location->importance * 100);

        uint32_t i, j;
        for (i = 0; i < location->remark_count; i++) {
            uint32_t remark_number = location->remarks_list[i];
            for (j = 0; j < message_count; j++) {
                if (messages[j].remark_number == remark_number) {
                    const char** message = messages[j].message;
                    const char** description = messages[j].description;
                    fprintf(stdout, "  - %s: %s\n", *message, *description);
                }
            }
        }
    }

    fprintf(stdout, "\n");
}

int process_hotspots(perfexpert_list_t* hotspots, const char* report_filename) {
    perfexpert_list_t locations;
    perfexpert_list_construct(&locations);

    int err = parse(hotspots, report_filename, &locations);
    if (err != SUCCESS) {
        OUTPUT(("%s: %d",
                _ERROR("Failed to parse vectorization report, err code"), err));
        return PERFEXPERT_ERROR;
    }

    print_recommendations(&locations);

    while (perfexpert_list_get_size(&locations) != 0) {
        perfexpert_list_item_t* last = NULL;
        last = perfexpert_list_get_last(&locations);
        perfexpert_list_remove_item(&locations, last);
        PERFEXPERT_DEALLOC(last);
    }

    perfexpert_list_destruct(&locations);

    return PERFEXPERT_SUCCESS;
}
