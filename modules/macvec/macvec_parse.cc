
#include <regex.h>
#include "common/perfexpert_list.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <map>
#include <set>
#include <stack>

enum { SUCCESS = 0,
    ERR_FILE,
    ERR_REGEX,
    ERR_FILE_CONTENTS,
};

typedef struct _tag_location {
    std::string filename;
    long line_number;

    _tag_location(std::string _filename, long _line_number) {
        filename = _filename;
        line_number = _line_number;
    }

    bool operator<(const struct _tag_location& rhs) const {
        if (filename < rhs.filename) {
            return true;
        }
        
        if (filename == rhs.filename && line_number < rhs.line_number) {
            return true;
        }

        return false;
    }
} location_t;

typedef struct {
    long remark_number;
    const char* message;
} msg_list_t;

typedef std::set<long> remarks_set_t;
typedef std::stack<location_t> location_stack_t;
typedef std::map<location_t, remarks_set_t> remarks_t;

const int k_filename_len = 1024;

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

const msg_list_t messages[] = {
    {   15046,  msg_ptr_check       },
    {   15344,  msg_ptr_check       },
    {   15037,  msg_trip_count      },
    {   15017,  msg_trip_count      },
    {   15315,  msg_trip_count      },
    {   15523,  msg_trip_count      },
    {   15167,  msg_mem_align       },
    {   15524,  msg_mem_align       },
    {   15421,  msg_mem_align       },
    {   15167,  msg_mem_align       },
    {   15038,  msg_branch_eval     },
    {   15336,  msg_branch_eval     },
    {   15327,  msg_float_conv      },
};

const long message_count = sizeof(messages) / sizeof(msg_list_t);

int process_file(perfexpert_list_t* hotspot_list, std::ifstream& report_file,
        remarks_t& remarks, regex_t& re_loop, regex_t& re_remark) {
    std::string line;
    location_stack_t loc_stack;

    while (getline(report_file, line)) {
        regmatch_t pmatch[3] = {0};
        const char* c_line = line.c_str();
        if (strstr(c_line, "LOOP END") != NULL) {
            if (loc_stack.empty() == false) {
                loc_stack.pop();
            }
        } else if (regexec(&re_loop, c_line, 3, pmatch, 0) == 0) {
            int start = pmatch[1].rm_so;
            int length = pmatch[1].rm_eo - pmatch[1].rm_so;

            char _file[k_filename_len] = "";
            strncat(_file, c_line + start, std::min(k_filename_len, length));
            std::string filename = _file;

            long line_number = strtol(c_line + pmatch[2].rm_so, NULL, 10);

            location_t location(filename, line_number);
            loc_stack.push(location);
        } else if (loc_stack.empty() == false) {
            if (regexec(&re_remark, c_line, 2, pmatch, 0) == 0) {
                long remark_number = strtol(c_line + pmatch[1].rm_so, NULL, 10);

                location_t& loc = loc_stack.top();
                remarks_t::iterator it = remarks.find(loc);
                if (it == remarks.end()) {
                    remarks_set_t remarks_set;
                    remarks_set.insert(remark_number);
                    remarks[loc] = remarks_set;
                } else {
                    remarks_set_t& remarks_set = it->second;
                    remarks_set.insert(remark_number);
                }
            }
        }
    }

    return 0;
}

int parse(perfexpert_list_t* hotspot_list, const std::string& report_filename,
        remarks_t& remarks) {
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

    std::ifstream report_file(report_filename.c_str());
    if (report_file.is_open() == false) {
        return -ERR_FILE;
    }

    if (process_file(hotspot_list, report_file, remarks, re_loop,
            re_remark) != 0) {
        return -ERR_FILE_CONTENTS;
    }

    report_file.close();

    regfree(&re_remark);
    regfree(&re_loop);

    return SUCCESS;
}

int print_recommendations(const remarks_t& remarks) {
    for (remarks_t::const_iterator it = remarks.begin(); it != remarks.end();
            it++) {
        location_t loc = it->first;
        remarks_set_t remarks_set = it->second;

        fprintf(stderr, "== %s:%d ==\n", loc.filename.c_str(), loc.line_number);
        for (remarks_set_t::iterator it = remarks_set.begin();
                it != remarks_set.end(); it++) {
            long int remark_number = *it;

            // We only have a handful of messages,
            // so just loop over them linearly.
            for (int i = 0; i < message_count; i++) {
                if (messages[i].remark_number == remark_number) {
                    fprintf(stderr, "  - %s\n", messages[i].message);
                }
            }
        }

        fprintf(stderr, "\n");
    }

    return SUCCESS;
}

extern "C" {
int process_hotspots(perfexpert_list_t* hotspot_list) {
    remarks_t remarks;
    std::string report_filename = "vec-report.txt";

    int err_code = parse(hotspot_list, report_filename, remarks);
    if (err_code != SUCCESS) {
        fprintf(stderr, "Failed to parse vectorization report, err code: %d.\n",
                err_code);
        return err_code;
    }

    print_recommendations(remarks);

    return 0;
}
}
