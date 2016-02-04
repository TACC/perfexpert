
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

typedef struct {
    long remark_number;
    const char** message;
    const char** description;

    void (*process_fn)(const char*, location_t*);
} message_t;

const char* msg_ptr_check = "Check if pointers overlap";
const char* msg_trip_count = "Inform compiler about loop trip count";
const char* msg_mem_align = "Align memory references";
const char* msg_branch_eval = "Indicate branch evaluations";
const char* msg_float_conv = "Use limited types";
const char* msg_unit_stride = "Try and use unit strides";
const char* msg_prefetch = "Try software prefetching";

const char* dsc_ptr_check = "If the compiler is unaware that pointers do not "
    "overlap in memory, the compiler's dependence analysis may infer "
    "existence of vector dependence. If the pointers used in this loop "
    "body indeed do not overlap, declare them with the 'restrict' keyword. "
    "For instance: double* restrict ptr_a;. See "
    "https://software.intel.com/en-us/articles/vectorization-with-the-intel"
    "-compilers-part-i for additional details.";

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
    "https://software.intel.com/en-us/articles/data-alignment-to-assist-"
    "vectorization for additional details.";

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

const char* dsc_unit_stride = "Strided accesses to array elements leads to "
    "cache and TLB misses. A unit stride (stride increment of 1) is not only "
    "cache and TLB friendly but also assists in vectorization. To enforce unit "
    "strides, check that the loop induction variable is incremented by one and "
    "that the loop induction variable is used to index into the last "
    "(innermost) dimension of the array.";

const char* dsc_prefetch = "Gather and scatter accesses in vector instructions "
    "cause high penalty at execution time. If the code is being compiled for "
    "the Xeon Phi coprocessor, using software prefetching may help. See "
    "https://software.intel.com/sites/products/documentation/doclib/iss/2013/compiler/cpp-lin/GUID-3A086451-4C82-4BB1-B742-FF93EBF60DA3.htm "
    "for details.";

static regex_t re_dep_var, re_align_var;

void align_var(const char* line, location_t* location) {
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
    }
}

void extract_var(const char* line, location_t* location) {
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
    }
}

const message_t messages[] = {
    { 15308, &msg_ptr_check,    &dsc_ptr_check,     NULL,         },
    { 15344, &msg_ptr_check,    &dsc_ptr_check,     NULL,         },
    { 15346, &msg_ptr_check,    NULL,               &extract_var, },

    { 15315, &msg_trip_count,   &dsc_trip_count,    NULL,         },
    { 15321, &msg_trip_count,   &dsc_trip_count,    NULL,         },
    { 15335, &msg_trip_count,   &dsc_trip_count,    NULL,         },
    { 15523, &msg_trip_count,   &dsc_trip_count,    NULL,         },

    { 15381, &msg_mem_align,    &dsc_mem_align,     NULL,         },
    { 15389, &msg_mem_align,    &dsc_mem_align,     &align_var,   },
    { 15409, &msg_mem_align,    &dsc_mem_align,     NULL,         },
    { 15421, &msg_mem_align,    &dsc_mem_align,     NULL,         },
    { 15450, &msg_mem_align,    &dsc_mem_align,     NULL,         },
    { 15451, &msg_mem_align,    &dsc_mem_align,     NULL,         },
    { 15456, &msg_mem_align,    &dsc_mem_align,     NULL,         },
    { 15457, &msg_mem_align,    &dsc_mem_align,     NULL,         },
    { 15468, &msg_mem_align,    &dsc_mem_align,     NULL,         },
    { 15469, &msg_mem_align,    &dsc_mem_align,     NULL,         },
    { 15472, &msg_mem_align,    &dsc_mem_align,     NULL,         },
    { 15473, &msg_mem_align,    &dsc_mem_align,     NULL,         },
    { 15524, &msg_mem_align,    &dsc_mem_align,     NULL,         },

    { 15336, &msg_branch_eval,  &dsc_branch_eval,   NULL,         },

    { 15311, &msg_float_conv,   &dsc_float_conv,    NULL,         },
    { 15327, &msg_float_conv,   &dsc_float_conv,    NULL,         },
    { 15386, &msg_float_conv,   &dsc_float_conv,    NULL,         },
    { 15410, &msg_float_conv,   &dsc_float_conv,    NULL,         },
    { 15411, &msg_float_conv,   &dsc_float_conv,    NULL,         },
    { 15417, &msg_float_conv,   &dsc_float_conv,    NULL,         },
    { 15418, &msg_float_conv,   &dsc_float_conv,    NULL,         },

    { 15320, &msg_unit_stride,  &dsc_unit_stride,   NULL,         },
    { 15452, &msg_unit_stride,  &dsc_unit_stride,   NULL,         },
    { 15453, &msg_unit_stride,  &dsc_unit_stride,   NULL,         },
    { 15460, &msg_unit_stride,  &dsc_unit_stride,   NULL,         },
    { 15461, &msg_unit_stride,  &dsc_unit_stride,   NULL,         },

    { 15415, &msg_prefetch,     &dsc_prefetch,      NULL,         },
    { 15416, &msg_prefetch,     &dsc_prefetch,      NULL,         },
    { 15458, &msg_prefetch,     &dsc_prefetch,      NULL,         },
    { 15459, &msg_prefetch,     &dsc_prefetch,      NULL,         },
    { 15462, &msg_prefetch,     &dsc_prefetch,      NULL,         },
    { 15463, &msg_prefetch,     &dsc_prefetch,      NULL,         },
};

const long message_count = sizeof(messages) / sizeof(message_t);

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
        OUTPUT(("Processing line: %s", file_line));
        if (strlen(file_line)<5) {
            continue;
        }
        regmatch_t pmatch[3] = {0};
        OUTPUT(("BLAH"));
        if (strstr(file_line, "LOOP END") != NULL) {
                OUTPUT(("1"));
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
            OUTPUT(("2"));
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
            OUTPUT(("3"));
            location_t *location = NULL;
            location = (location_t *)perfexpert_list_get_last(&stack);

            if (location->ignored == 0 &&
                regexec(re_remark, file_line, 2, pmatch, 0) == 0) {
                long remark_number = strtol(file_line + pmatch[1].rm_so, NULL,
                    10);

                /* Check if this is an "interesting" remark. */
                uint32_t i = 0;
                for (i = 0; i < message_count; i++) {
                    if (messages[i].remark_number == remark_number) {
                        break;
                    }
                }

                if (i == message_count) {
                    OUTPUT_VERBOSE((4, "Ignoring remark %ld because we are not "
                            "interested in tracking it.", remark_number));
                    /* Not found in interested remarks. */
                    continue;
                }

                OUTPUT_VERBOSE((4, "Found line with remark %ld.",
                        remark_number));
                if (messages[i].process_fn != NULL) {
                    OUTPUT_VERBOSE((4, "Running remark-specific handler..."));
                    messages[i].process_fn(file_line, location);
                    OUTPUT_VERBOSE((4, "Exited remark-specific handler..."));
                }

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

    OUTPUT(("4"));
    regfree(&re_dep_var);
    regfree(&re_align_var);

    OUTPUT(("%s [%d]", _YELLOW("Location list size after matching "
            "vectorization report with hotspots:"),
            perfexpert_list_get_size(locations)));

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

    perfexpert_list_destruct(&stack);
    return SUCCESS;
}

int parse(perfexpert_list_t* hotspots, perfexpert_list_t* locations) {
    macvec_hotspot_t *h = NULL;

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

    perfexpert_list_for (h, hotspots, macvec_hotspot_t) {
        FILE* stream = NULL;
        char * report_filename;
        PERFEXPERT_ALLOC (char, report_filename, strlen(h->file)+7);
        char * pch;
        pch = strrchr(h->file, '.');
        memcpy(report_filename, h->file, pch - h->file);
        strcat(report_filename, ".optrpt");
        OUTPUT(("\n\n\n\n\n\n\n Reading report file: %s \n\n\n\n\n",report_filename));
//        char * report_filename = h->file;
        stream = fopen(report_filename, "r");
        PERFEXPERT_DEALLOC(report_filename);
        if (stream == NULL) {
            regfree(&re_remark);
            regfree(&re_loop);

            return -ERR_FILE;
        }

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
    }

    return SUCCESS;
}

static void print_recommendations(perfexpert_list_t* locations) {
    location_t* location;

    OUTPUT(("%s [%d]", _YELLOW("Printing recommendations"),
        perfexpert_list_get_size(locations)));

    perfexpert_list_for(location, locations, location_t) {
        fprintf(stdout, "\nLoop in function %s in %s:%d (%.2f%% of the total "
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

                if (i != size-1) {
                    fprintf(stdout, ", ");
                } else {
                    fprintf(stdout, ".");
                }

                i += 1;
            }

            fprintf(stdout, "\n");
        }

        OUTPUT_VERBOSE((4, "Remark count: %d.", location->remark_count));

        uint32_t i, j;
        for (i = 0; i < location->remark_count; i++) {
            uint32_t remark_number = location->remarks_list[i];
            for (j = 0; j < message_count; j++) {
                if (messages[j].remark_number == remark_number) {
                    const char** message = messages[j].message;
                    const char** description = messages[j].description;
                    if (message != NULL && description != NULL) {
                        fprintf(stdout, "  - %s: %s\n", *message, *description);
                    }
                }
            }
        }
    }

    fprintf(stdout, "\n");
}

int process_hotspots(perfexpert_list_t* hotspots) {
    perfexpert_list_t locations;
    perfexpert_list_construct(&locations);
    int err = SUCCESS;
   
    if (SUCCESS != (err = parse(hotspots, &locations))) {
        OUTPUT(("%s: %d",
            _ERROR("Failed to parse vectorization reports, err code"), err));
        return PERFEXPERT_ERROR;
    }

    OUTPUT(("GOING TO PRINT THE RECOMMENDATIONS"));
    print_recommendations(&locations);

    OUTPUT(("DONE WITH THE RECOMMENDATIONS"));
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
    OUTPUT(("DONE PROCESSING HOTSPOTS"));
    return PERFEXPERT_SUCCESS;
}
