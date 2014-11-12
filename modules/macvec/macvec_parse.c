
#include <libgen.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "macvec.h"
#include "macvec_types.h"
#include "common/perfexpert_list.h"

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
    char filename[k_filename_len];

    uint32_t remark_count;
    uint32_t remarks_list[k_remarks_count];
} location_t;

typedef struct {
    long remark_number;
    const char** message;
} message_t;

const char* msg_ptr_check = "Check if pointers overlap.";
const char* msg_trip_count = "If loop trip count is known, add "
        "\"#pragma loop_count(n)\" directive before the loop body, where 'n' "
        "is equal to the loop count.";
const char* msg_mem_align = "Align memory references using _mm_malloc() or "
        "__attribute__((aligned)).";
const char* msg_branch_eval = "If the branch evaluates to always true or "
        "always false, add the \"__builtin_expect()\" primitive.";
const char* msg_float_conv = "Consider converting all value and variables to a "
        "single type (either all integers, or all floats or all doubles).";

const message_t messages[13] = {
    {   15046,  &msg_ptr_check      },
    {   15344,  &msg_ptr_check      },
    {   15037,  &msg_trip_count     },
    {   15017,  &msg_trip_count     },
    {   15315,  &msg_trip_count     },
    {   15523,  &msg_trip_count     },
    {   15167,  &msg_mem_align      },
    {   15524,  &msg_mem_align      },
    {   15421,  &msg_mem_align      },
    {   15167,  &msg_mem_align      },
    {   15038,  &msg_branch_eval    },
    {   15336,  &msg_branch_eval    },
    {   15327,  &msg_float_conv     },
};

const long message_count = sizeof(messages) / sizeof(message_t);

uint8_t in_list(perfexpert_list_t* hotspots, char* filename, int line_number) {
    macvec_hotspot_t* hotspot;
    if (hotspots == NULL || filename == NULL) {
        return 0;
    }

    perfexpert_list_for(hotspot, hotspots, macvec_hotspot_t) {
        char* base = basename(hotspot->file);
        if (strncmp(base, filename, k_filename_len) == 0 &&
                line_number == hotspot->line) {
            return 1;
        }
    }

    return 0;
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
            uint8_t present = in_list(hotspots, _file, line_number);
            uint8_t ignored = (1 - present);

            location_t* loc;
            PERFEXPERT_ALLOC(location_t, loc, sizeof(location_t));
            snprintf(loc->filename, k_filename_len, "%s", _file);
            loc->line_number = line_number;
            loc->ignored = ignored;
            loc->remark_count = 0;

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
        fprintf(stderr, "== %s:%d ==\n", location->filename,
                location->line_number);

        uint32_t i, j;
        for (i = 0; i < location->remark_count; i++) {
            uint32_t remark_number = location->remarks_list[i];
            for (j = 0; j < message_count; j++) {
                if (messages[j].remark_number == remark_number) {
                    char** message = messages[j].message;
                    fprintf(stderr, "  - %s\n", *message);
                }
            }
        }
    }
}

int process_hotspots(perfexpert_list_t* hotspots, const char* report_filename) {
    perfexpert_list_t locations;
    perfexpert_list_construct(&locations);

    int err_code = parse(hotspots, report_filename, &locations);
    if (err_code != SUCCESS) {
        fprintf(stderr, "Failed to parse vectorization report, err code: %d.\n",
                err_code);
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
