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

#include <limits.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sched.h>
#include <unistd.h>

#include <algorithm>
#include <cstdarg>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <utility>

#include "avl_tree.h"
#include "histogram.h"
#include "mrt.h"
#include "macpo_record.h"

typedef std::pair<int64_t, int16_t> val_idx_pair;
typedef std::pair<int, int16_t> line_threadid_pair;

typedef int16_t thread_id;
typedef int line_number;

typedef std::map<thread_id, bool> bool_map;
typedef std::map<thread_id, int16_t> short_map;
typedef std::map<thread_id, int64_t*> long_histogram;

typedef std::map<int64_t, bool_map> bool_map_coll;
typedef std::map<int64_t, short_map> short_map_coll;
typedef std::map<int64_t, long_histogram> long_histogram_coll;

static const int DIST_INFINITY = 40 * 1024 * 1024 / 64;

static std::set<int> analyzed_loops;

static bool_map_coll overlap_bin;
static short_map_coll branch_bin, align_bin, sstore_align_bin, stride_bin;
static long_histogram_coll tripcount_map;

static std::map<int, int> branch_loop_line_pair;
static std::map<int, std::set<int> > loop_branch_line_pair;

static volatile int16_t lock_var;
static volatile sig_atomic_t sleeping = 0;
static volatile sig_atomic_t access_count = 0;

static int fd = -1;
static int sleep_sec = 0;
static int new_sleep_sec = 1;
static int *intel_apic_mapping = NULL;
static node_t terminal_node;
static size_t numCores = 0;

static std::vector<std::string> stream_list;

static __thread int coreID = -1;
static __thread avl_tree* tree = NULL;
static __thread gsl_histogram* histogram_list[MAX_VARIABLES] = {0};

static inline void lock() {
    while (__sync_bool_compare_and_swap(&lock_var, 0, 1) == false) {
    }

    asm volatile("lfence" ::: "memory");
}

static inline void unlock() {
    lock_var = 0;
    asm volatile("sfence" ::: "memory");
}

static bool index_comparator(const val_idx_pair& v1, const val_idx_pair& v2) {
    return v1.first < v2.first;
}

static int get_proc_kind() {
    // Get which processor is this running on
    int proc, info[4];
    if (!isCPUIDSupported()) {
        fprintf(stderr, "MACPO :: CPUID not supported, cannot determine "
                "processor core information, resorting to defaults...\n");
        proc = PROC_UNKNOWN;
    } else {
        char processorName[13];
        getProcessorName(processorName);

        if (strncmp(processorName, "AuthenticAMD", 12) == 0)
            proc = PROC_AMD;
        else if (strncmp(processorName, "GenuineIntel", 12) == 0)
            proc = PROC_INTEL;
        else
            proc = PROC_UNKNOWN;
    }

    return proc;
}

static int getCoreID() {
    if (coreID != -1)
        return coreID;

    int info[4];
    if (!isCPUIDSupported()) {
        coreID = 0;
        return coreID;  // default
    }

    int proc = get_proc_kind();
    if (proc == PROC_AMD) {
        __cpuid(info, 1, 0);
        coreID = (info[1] & 0xff000000) >> 24;
        return coreID;
    } else if (proc == PROC_INTEL) {
        int apic_id = 0;
        __cpuid(info, 0xB, 0);
        if (info[EBX] != 0) {   // x2APIC
            __cpuid(info, 0xB, 2);
            apic_id = info[EDX];

#ifdef DEBUG_PRINT
            fprintf(stderr, "MACPO :: Request from core with x2APIC ID "
                "%d\n", apic_id);
#endif
        } else {    // Traditonal APIC
            __cpuid(info, 1, 0);
            apic_id = (info[EBX] & 0xff000000) >> 24;

#ifdef DEBUG_PRINT
            fprintf(stderr, "MACPO :: Request from core with legacy APIC ID "
                    "%d\n", apic_id);
#endif
        }

        int i;
        for (i = 0; i < numCores; i++) {
            if (apic_id == intel_apic_mapping[i])
                break;
        }

        coreID = i == numCores ? 0 : i;
        return coreID;
    }

    coreID = 0;
    return coreID;
}

static void print_tripcount_histogram(int line_number, int64_t* histogram) {
    val_idx_pair pair_histogram[MAX_HISTOGRAM_ENTRIES];
    for (int i = 0; i < MAX_HISTOGRAM_ENTRIES; i++) {
        pair_histogram[i] = val_idx_pair(histogram[i], i);
    }

    std::sort(pair_histogram, pair_histogram + MAX_HISTOGRAM_ENTRIES);
    if (pair_histogram[MAX_HISTOGRAM_ENTRIES-1].first > 0) {
        // At least one non-zero entry.
        int values[3] = { pair_histogram[MAX_HISTOGRAM_ENTRIES-1].second,
            pair_histogram[MAX_HISTOGRAM_ENTRIES-2].second,
            pair_histogram[MAX_HISTOGRAM_ENTRIES-3].second
        };

        int64_t counts[3] = { pair_histogram[MAX_HISTOGRAM_ENTRIES-1].first,
            pair_histogram[MAX_HISTOGRAM_ENTRIES-2].first,
            pair_histogram[MAX_HISTOGRAM_ENTRIES-3].first
        };

        int low_tripcount = 0;
        const int VALUE_THRESHOLD = 16;
        const int COUNT_THRESHOLD = 16;

        if (counts[0] > COUNT_THRESHOLD && values[0] < VALUE_THRESHOLD) {
            low_tripcount = values[0];
        } else if (counts[1] > COUNT_THRESHOLD && values[1] < VALUE_THRESHOLD) {
            low_tripcount = values[1];
        } else if (counts[2] > COUNT_THRESHOLD && values[2] < VALUE_THRESHOLD) {
            low_tripcount = values[2];
        }

        if (low_tripcount > 0) {
            fprintf(stderr, "Found low trip count (%d) for this loop, "
                "insert the following pragma just before the loop body: "
                "#pragma loop_count(%d)\n", low_tripcount, low_tripcount);
        }
    }
}

static bool nonzero_alignment(int line_number, int64_t* histogram) {
    for (int i = 1; i < CACHE_LINE_SIZE; i++) {
        if (histogram[i] > 0)
            return true;
    }

    return false;
}

static void print_alignment_histogram(int line_number, int64_t* histogram) {
    val_idx_pair pair_histogram[CACHE_LINE_SIZE];
    for (int i = 0; i < CACHE_LINE_SIZE; i++) {
        pair_histogram[i] = val_idx_pair(histogram[i], i);
    }

    std::sort(pair_histogram, pair_histogram + CACHE_LINE_SIZE);
    if (pair_histogram[CACHE_LINE_SIZE-1].first > 0) {
        // At least one non-zero entry.
        int values[3] = { pair_histogram[CACHE_LINE_SIZE-1].second,
            pair_histogram[CACHE_LINE_SIZE-2].second,
            pair_histogram[CACHE_LINE_SIZE-3].second
        };

        int64_t counts[3] = { pair_histogram[CACHE_LINE_SIZE-1].first,
            pair_histogram[CACHE_LINE_SIZE-2].first,
            pair_histogram[CACHE_LINE_SIZE-3].first
        };

        fprintf(stderr, "  count %d%s found %ld time(s), ", values[0],
                values[0] == MAX_HISTOGRAM_ENTRIES-1 ? "+" : "", counts[0]);
        if (counts[1]) {
            fprintf(stderr, "count %d%s found %ld time(s), ", values[1],
                    values[1] == MAX_HISTOGRAM_ENTRIES-1 ? "+" : "", counts[1]);

            if (counts[2]) {
                fprintf(stderr, "count %d%s found %ld time(s), ", values[2],
                        values[2] == MAX_HISTOGRAM_ENTRIES-1 ? "+" : "",
                        counts[2]);
            }
        }

        fprintf(stderr, "\n");
    }
}

void indigo__exit() {
    int core_id = getCoreID();

    if (fd >= 0)
        close(fd);

    if (intel_apic_mapping)
        free(intel_apic_mapping);

    for (std::set<int>::iterator it = analyzed_loops.begin();
            it != analyzed_loops.end(); it++) {
        const int& line_number = *it;

        fprintf(stderr, "\n==== Loop at line %d ====\n", line_number);

        if (sstore_align_bin.find(line_number) != sstore_align_bin.end()) {
            short_map& map = sstore_align_bin[line_number];
            int16_t align_status = ALIGN_NOINIT;
            for (short_map::iterator it = map.begin(); it != map.end();
                    it++) {
                int16_t _align_status = it->second;
                if (_align_status < align_status)
                    align_status = _align_status;
            }

            switch (align_status) {
                case FULL_ALIGNED:
                    fprintf(stderr, "all non-temporal arrays align, use "
                            "__assume_aligned() or #pragma vector aligned to "
                            "tell compiler about alignment.\n");
                    break;

                case MUTUAL_ALIGNED:
                    fprintf(stderr, "all non-temporal arrays are mutually "
                            "aligned but not aligned to cache-line boundary, "
                            "use _mm_malloc/_mm_free to allocate/free aligned "
                            "storage.\n");
                    break;

                case NOT_ALIGNED:
                    fprintf(stderr, "non-temporal arrays are not aligned, "
                            "try using _mm_malloc/_mm_free to allocate/free "
                            "arrays that are aligned with cache-line "
                            "bounary.\n");
                    break;
            }
        }

        if (align_bin.find(line_number) != align_bin.end()) {
            short_map& map = align_bin[line_number];
            int16_t align_status = ALIGN_NOINIT;
            for (short_map::iterator it = map.begin(); it != map.end();
                    it++) {
                int16_t _align_status = it->second;
                if (_align_status < align_status)
                    align_status = _align_status;
            }

            switch (align_status) {
                case FULL_ALIGNED:
                    fprintf(stderr, "all arrays align, use "
                            "__assume_aligned() or #pragma vector aligned to "
                            "tell compiler about alignment.\n");
                    break;

                case MUTUAL_ALIGNED:
                    fprintf(stderr, "all arrays are mutually aligned but "
                            "not aligned to cache-line boundary, use "
                            "_mm_malloc/_mm_free to allocate/free aligned "
                            "storage.\n");
                    break;

                case NOT_ALIGNED:
                    fprintf(stderr, "arrays are not aligned, try using "
                            "_mm_malloc/_mm_free to allocate/free arrays that "
                            "are aligned with cache-line bounary.\n");
                    break;
            }
        }

        if (overlap_bin.find(line_number) != overlap_bin.end()) {
            bool_map& map = overlap_bin[line_number];
            bool overlap = false;
            for (bool_map::iterator it = map.begin(); it != map.end(); it++) {
                bool _overlap = it->second;
                if (_overlap == true) {
                    overlap = true;
                    break;
                }
            }

            if (overlap == false) {
                fprintf(stderr, "Use `restrict' keyword to inform "
                        "compiler that arrays do not overlap.\n");
            }
        }

        if (tripcount_map.find(line_number) != tripcount_map.end()) {
            long_histogram& histogram = tripcount_map[line_number];
            for (long_histogram::iterator it = histogram.begin();
                    it != histogram.end(); it++) {
                int64_t* histogram = it->second;

                if (histogram) {
                    print_tripcount_histogram(line_number, histogram);
                    free(histogram);
                }
            }
        }

        if (loop_branch_line_pair.find(line_number) !=
                loop_branch_line_pair.end()) {
            std::set<int>& branch_lines = loop_branch_line_pair[line_number];
            for (std::set<int>::iterator it = branch_lines.begin();
                    it != branch_lines.end(); it++) {
                const int& branch_line_number = *it;

                if (branch_bin.find(branch_line_number) != branch_bin.end()) {
                    short_map& map = branch_bin[branch_line_number];
                    int16_t branch_status = BRANCH_NOINIT;
                    for (short_map::iterator it = map.begin(); it != map.end();
                            it++) {
                        int16_t _branch_status = it->second;
                        if (_branch_status < branch_status)
                            branch_status = _branch_status;
                    }

                    switch (branch_status) {
                        case BRANCH_UNKNOWN:
                            fprintf(stderr, "branch at line %d is "
                                    "unpredictable.\n", branch_line_number);
                            break;

                        case BRANCH_MOSTLY_FALSE:
                            fprintf(stderr, "branch at line %d mostly "
                                    "evaluates to false.\n",
                                    branch_line_number);
                            break;

                        case BRANCH_MOSTLY_TRUE:
                            fprintf(stderr, "branch at line %d mostly "
                                    "evaluates to true.\n",
                                    branch_line_number);
                            break;

                        case BRANCH_FALSE:
                            fprintf(stderr, "branch at line %d always "
                                    "evaluates to false, use "
                                    "__builtin_expect() to inform compiler "
                                    "about expected branch outcome.\n",
                                    branch_line_number);
                            break;

                        case BRANCH_TRUE:
                            fprintf(stderr, "branch at line %d always "
                                    "evaluates to true, use "
                                    "__builtin_expect() to inform compiler "
                                    "about expected branch outcome.\n",
                                    branch_line_number);
                            break;
                    }
                }
            }
        }

        if (stride_bin.find(line_number) != stride_bin.end()) {
            short_map& map = stride_bin[line_number];
            int16_t stride_status = STRIDE_NOINIT;
            for (short_map::iterator it = map.begin(); it != map.end();
                    it++) {
                int16_t _stride_status = it->second;
                if (_stride_status < stride_status)
                    stride_status = _stride_status;
            }

            switch (stride_status) {
                case STRIDE_UNKNOWN:
                    fprintf(stderr, "Could not determine stride value.\n");
                    break;

                case STRIDE_FIXED:
                    fprintf(stderr, "Each array reference has a fixed "
                            "(but non-unit) stride.\n");
                    break;

                case STRIDE_UNIT:
                    fprintf(stderr, "Each array reference has a unit "
                            "stride.\n ");
                    break;
            }
        }
    }

    for (int i = 0; i < MAX_VARIABLES; i++) {
        if (histogram_list[i] == NULL) {
            continue;
        }

        pair_list_t pair_list;
        flatten_and_sort_histogram(histogram_list[i], pair_list);

        if (i >= stream_list.size()) {
            // We can't process any more histograms because
            // stream_list will not contain names for any of them.
            return;
        }

        size_t limit = 3 < pair_list.size() ? 3 : pair_list.size();
        if (limit == 0) {
            continue;
        }

        size_t max_bin = pair_list[0].first;
        if (max_bin == DIST_INFINITY) {
            fprintf(stderr, "\nReuse distance for %s is greater than the size "
                    "of the last-level cache. Consider adding the following "
                    "pragma to let the compiler generate non-temporal store "
                    "instructions:\n#pragma vector nontemporal(%s)\n",
                    stream_list[i].c_str(), stream_list[i].c_str());
        }
    }

    // FIXME: De-allocate all histograms.
}

int16_t& get_branch_bin(int line_number, int loop_line_number) {
    int16_t core_id = getCoreID();
    short_map_coll::iterator it = branch_bin.find(line_number);
    if (it != branch_bin.end()) {
        short_map& map = it->second;
        short_map::iterator it = map.find(core_id);
        if (it != map.end()) {
            // It's a hit!
            analyzed_loops.insert(line_number);
            return map[core_id];
        }
    }

    // We couldn't locate the histogram value, go down the slow path.
    // Obtain lock.
    lock();

    // Check if we need to allocate a new histogram.
    it = branch_bin.find(line_number);
    if (it == branch_bin.end()) {
        short_map& map = branch_bin[line_number];
        short_map::iterator it = map.find(core_id);
        if (it == map.end()) {
            map[core_id] = BRANCH_NOINIT;
        }
    } else {
        short_map& map = it->second;
        short_map::iterator it = map.find(core_id);
        if (it == map.end()) {
            map[core_id] = BRANCH_NOINIT;
        }
    }

    // Release lock.
    unlock();

    return branch_bin[line_number][core_id];
}

int64_t* new_histogram(size_t histogram_entries) {
    int64_t* histogram = reinterpret_cast<int64_t*>(malloc(sizeof(int64_t)
                * histogram_entries));
    if (histogram) {
        memset(histogram, 0, sizeof(int64_t) * histogram_entries);
    }

    return histogram;
}

bool& get_overlap_bin(int line_number) {
    int16_t core_id = getCoreID();
    bool_map_coll::iterator it = overlap_bin.find(line_number);
    if (it != overlap_bin.end()) {
        bool_map& map = it->second;
        bool_map::iterator it = map.find(core_id);
        if (it != map.end()) {
            // It's a hit!
            analyzed_loops.insert(line_number);
            return map[core_id];
        }
    }

    // We couldn't locate the histogram value, go down the slow path.
    // Obtain lock.
    lock();

    // Check if we need to allocate a new histogram.
    it = overlap_bin.find(line_number);
    if (it == overlap_bin.end()) {
        bool_map& map = overlap_bin[line_number];
        bool_map::iterator it = map.find(core_id);
        if (it == map.end()) {
            map[core_id] = 0;
        }
    } else {
        bool_map& map = it->second;
        bool_map::iterator it = map.find(core_id);
        if (it == map.end()) {
            map[core_id] = 0;
        }
    }

    // Release lock.
    unlock();

    return overlap_bin[line_number][core_id];
}

#define MAX_ADDR    128

/**
    Helper function to allocate / get histogramss for alignment checking.
*/

int64_t* get_tripcount_histogram(int line_number) {
    int16_t core_id = getCoreID();
    long_histogram_coll::iterator it = tripcount_map.find(line_number);
    if (it != tripcount_map.end()) {
        long_histogram& histogram = it->second;
        long_histogram::iterator it = histogram.find(core_id);
        if (it != histogram.end()) {
            // It's a hit!
            analyzed_loops.insert(line_number);
            return histogram[core_id];
        }
    }

    // We couldn't locate the histogram value, go down the slow path.
    int64_t* return_value = NULL;

    // Obtain lock.
    lock();

    // Check if we need to allocate a new histogram.
    it = tripcount_map.find(line_number);
    if (it == tripcount_map.end()) {
        long_histogram& histogram = tripcount_map[line_number];
        long_histogram::iterator it = histogram.find(core_id);
        if (it == histogram.end()) {
            histogram[core_id] = new_histogram(MAX_HISTOGRAM_ENTRIES);
            return_value = histogram[core_id];
        } else {
            return_value = it->second;
        }
    } else {
        long_histogram& histogram = it->second;
        long_histogram::iterator it = histogram.find(core_id);
        if (it == histogram.end()) {
            histogram[core_id] = new_histogram(MAX_HISTOGRAM_ENTRIES);
            return_value = histogram[core_id];
        } else {
            return_value = it->second;
        }
    }

    // Release lock.
    unlock();

    return return_value;
}

int16_t& get_sstore_alignment_bin(int line_number) {
    int16_t core_id = getCoreID();
    short_map_coll::iterator it = sstore_align_bin.find(line_number);
    if (it != sstore_align_bin.end()) {
        short_map& map = it->second;
        short_map::iterator it = map.find(core_id);
        if (it != map.end()) {
            // It's a hit!
            analyzed_loops.insert(line_number);
            return map[core_id];
        }
    }

    // We couldn't locate the histogram value, go down the slow path.
    // Obtain lock.
    lock();

    // Check if we need to allocate a new histogram.
    it = sstore_align_bin.find(line_number);
    if (it == sstore_align_bin.end()) {
        short_map& map = sstore_align_bin[line_number];
        short_map::iterator it = map.find(core_id);
        if (it == map.end()) {
            map[core_id] = ALIGN_NOINIT;
        }
    } else {
        short_map& map = it->second;
        short_map::iterator it = map.find(core_id);
        if (it == map.end()) {
            map[core_id] = ALIGN_NOINIT;
        }
    }

    // Release lock.
    unlock();

    return sstore_align_bin[line_number][core_id];
}

int16_t& get_alignment_bin(int line_number) {
    int16_t core_id = getCoreID();
    short_map_coll::iterator it = align_bin.find(line_number);
    if (it != align_bin.end()) {
        short_map& map = it->second;
        short_map::iterator it = map.find(core_id);
        if (it != map.end()) {
            // It's a hit!
            analyzed_loops.insert(line_number);
            return map[core_id];
        }
    }

    // We couldn't locate the histogram value, go down the slow path.
    // Obtain lock.
    lock();

    // Check if we need to allocate a new histogram.
    it = align_bin.find(line_number);
    if (it == align_bin.end()) {
        short_map& map = align_bin[line_number];
        short_map::iterator it = map.find(core_id);
        if (it == map.end()) {
            map[core_id] = ALIGN_NOINIT;
        }
    } else {
        short_map& map = it->second;
        short_map::iterator it = map.find(core_id);
        if (it == map.end()) {
            map[core_id] = ALIGN_NOINIT;
        }
    }

    // Release lock.
    unlock();

    return align_bin[line_number][core_id];
}

int16_t& get_stride_bin(int line_number) {
    int16_t core_id = getCoreID();
    short_map_coll::iterator it = stride_bin.find(line_number);
    if (it != stride_bin.end()) {
        short_map& map = it->second;
        short_map::iterator it = map.find(core_id);
        if (it != map.end()) {
            // It's a hit!
            analyzed_loops.insert(line_number);
            return map[core_id];
        }
    }

    // We couldn't locate the histogram value, go down the slow path.
    // Obtain lock.
    lock();

    // Check if we need to allocate a new histogram.
    it = stride_bin.find(line_number);
    if (it == stride_bin.end()) {
        short_map& map = stride_bin[line_number];
        short_map::iterator it = map.find(core_id);
        if (it == map.end()) {
            map[core_id] = STRIDE_NOINIT;
        }
    } else {
        short_map& map = it->second;
        short_map::iterator it = map.find(core_id);
        if (it == map.end()) {
            map[core_id] = STRIDE_NOINIT;
        }
    }

    // Release lock.
    unlock();

    return stride_bin[line_number][core_id];
}

void indigo__record_branch_c(int line_number, int loop_line_number,
        int true_branch_count, int false_branch_count) {
    // Short circuit to prevent runtime overhead.
    if (get_branch_bin(line_number, loop_line_number) == BRANCH_UNKNOWN)
        return;

    branch_loop_line_pair[line_number] = loop_line_number;
    loop_branch_line_pair[loop_line_number].insert(line_number);

    int status = BRANCH_UNKNOWN;
    int branch_count = true_branch_count + false_branch_count;
    if (true_branch_count != 0 && false_branch_count == 0) {
        status = BRANCH_TRUE;
    } else if (true_branch_count * 100.0f > branch_count * 85.0f) {
        status = BRANCH_MOSTLY_TRUE;
    } else if (false_branch_count != 0 > true_branch_count == 0) {
        status = BRANCH_FALSE;
    } else if (false_branch_count * 100.0f > branch_count * 85.0f) {
        status = BRANCH_MOSTLY_FALSE;
    } else {
        status = BRANCH_UNKNOWN;
    }

    if (get_branch_bin(line_number, loop_line_number) == BRANCH_NOINIT) {
        get_branch_bin(line_number, loop_line_number) = status;
    } else {
        if (get_branch_bin(line_number, loop_line_number) != status) {
            get_branch_bin(line_number, loop_line_number) = BRANCH_UNKNOWN;
        }
    }
}

void indigo__vector_stride_c(int loop_line_number, int var_idx, void* addr,
        int type_size) {
    // If this process was never supposed to record stats
    // or if the file-open failed, then return
    if (fd < 0)
        return;

    if (sleeping == 1 || access_count >= 131072)    // 131072 is 128*1024.
        return;

    node_t node;
    node.type_message = MSG_VECTOR_STRIDE_INFO;

    node.vector_stride_info.coreID = getCoreID();
    node.vector_stride_info.address = (size_t) addr;
    node.vector_stride_info.var_idx = var_idx;
    node.vector_stride_info.loop_line_number = loop_line_number;
    node.vector_stride_info.type_size = type_size;

    write(fd, &node, sizeof(node_t));
}

/**
    indigo__aligncheck_c()
    Checks for alignment to cache line boundary and updates histogram.
    Returns common alignment, if any. Otherwise, returns -1.
*/
int indigo__aligncheck_c(int line_number, int stream_count, ...) {
    va_list args;
    int i, j;

    if (get_alignment_bin(line_number) == NOT_ALIGNED) {
        return -1;
    }

    va_start(args, stream_count);

    int64_t remainder = -1;
    for (i = 0; i < stream_count; i++) {
        void* address = va_arg(args, void*);
        int64_t _remainder = ((int64_t) address) % 64;
        if (remainder != -1 && remainder != _remainder) {
            va_end(args);
            get_alignment_bin(line_number) = NOT_ALIGNED;
            return -1;
        }

        if (remainder == -1)
            remainder = _remainder;
    }

    va_end(args);

    if (remainder == 0) {
        get_alignment_bin(line_number) = FULL_ALIGNED;
    } else {
        get_alignment_bin(line_number) = MUTUAL_ALIGNED;
    }

    return remainder;
}

/**
    indigo__sstore_aligncheck_c()
    Checks for alignment of streaming stores to cache line boundary and
    updates histogram.
    Returns common alignment, if any. Otherwise, returns -1.
*/

int indigo__sstore_aligncheck_c(int line_number, int stream_count, ...) {
    va_list args;
    int i, j;

    if (get_sstore_alignment_bin(line_number) == NOT_ALIGNED) {
        return -1;
    }

    va_start(args, stream_count);

    int64_t remainder = -1;
    for (i = 0; i < stream_count; i++) {
        void* address = va_arg(args, void*);
        int64_t _remainder = ((int64_t) address) % 64;
        if (remainder != -1 && remainder != _remainder) {
            va_end(args);
            get_sstore_alignment_bin(line_number) = NOT_ALIGNED;
            return -1;
        }

        if (remainder == -1)
            remainder = _remainder;
    }

    va_end(args);

    if (remainder == 0) {
        get_sstore_alignment_bin(line_number) = FULL_ALIGNED;
    } else {
        get_sstore_alignment_bin(line_number) = MUTUAL_ALIGNED;
    }

    return remainder;
}

/**
    indigo__overlap_check_c()
    If alternating references refer to valid start and end addresses,
    then checks for overlap among the references and updates the histogram.
    Returns void.
*/

void indigo__overlap_check_c(int line_number, int stream_count, ...) {
    va_list args;
    int i, j, ctr = 0;

#define MAX_STREAMS 64
    void* start_addresses[MAX_STREAMS];
    void* end_addresses[MAX_STREAMS];

    // If we've already found an overlap, terminate the search.
    if (get_overlap_bin(line_number) == true)
        return;

    va_start(args, stream_count);

    for (i = 0; i < stream_count && i < MAX_STREAMS; i++) {
        void* start = va_arg(args, void*);
        void* end = va_arg(args, void*);

        if (end > start) {
            start_addresses[ctr] = start;
            end_addresses[ctr] = end;

            ctr += 1;
        }
    }

    va_end(args);

    for (i = 1; i < ctr; i++) {
        void* start = start_addresses[i];
        void* end = end_addresses[i];

        // Loop over all previous addresses to see if there's an overlap.
        for (j = 0; j < i; j++) {
            void* ref_start = start_addresses[j];
            void* ref_end = end_addresses[j];

            if ((ref_start <= start && ref_end >= start) ||
                    (ref_start <= end && ref_end >= end)) {
                // We have an overlap!
                get_overlap_bin(line_number) = true;
                return;
            }
        }
    }
}

void indigo__tripcount_check_c(int line_number, int64_t trip_count) {
    int64_t* histogram = get_tripcount_histogram(line_number);
    if (histogram == NULL)
        return;

    if (trip_count < 0)
        trip_count = 0;

    if (trip_count >= MAX_HISTOGRAM_ENTRIES)
        trip_count = MAX_HISTOGRAM_ENTRIES-1;

    if (histogram[trip_count] < LONG_MAX)
        histogram[trip_count]++;
}

void indigo__unknown_stride_check_c(int line_number) {
    get_stride_bin(line_number) = STRIDE_UNKNOWN;
}

void indigo__stride_check_c(int line_number, int stride) {
    // Short circuit to prevent runtime overhead.
    if (get_stride_bin(line_number) == STRIDE_UNKNOWN)
        return;

    int16_t status = STRIDE_NOINIT;
    switch (abs(stride)) {
        // Don't modify stride status on stride value of zero.
        // This is meant to account for references to two dimensional arrays
        // as the last index value may not always correspond to the loop index
        // variable. For an example, look at the matrix multiplication code.
        case 0:
            break;

        default:
            status = STRIDE_FIXED;
            break;

        case 1:
            status = STRIDE_UNIT;
            break;
    }

    if (get_stride_bin(line_number) > status)
        get_stride_bin(line_number) = status;
}

void indigo__reuse_dist_c(int var_id, void* address) {
    if (sleeping == 1) {
        return;
    }

    // FIXME: This measures reuse distance only for those accesses
    // that are generated from the current thread only.

    if (var_id >= MAX_VARIABLES)
        return;

    // FIXME: Set DIST_INFINITY to twice the size of the largest cache.
    if (create_histogram_if_null(histogram_list[var_id], DIST_INFINITY) < 0)
        return;

    if (tree == NULL) {
        tree = new avl_tree();
    }

    // Construct a dummy mem_info_t packet.
    mem_info_t mem_info;
    mem_info.coreID = getCoreID();
    mem_info.var_idx = var_id;
    mem_info.address = (size_t) address;
    mem_info.read_write = TYPE_WRITE;

    size_t distance = DIST_INFINITY - 1;
    size_t cache_line = ADDR_TO_CACHE_LINE(mem_info.address);

    if (tree->contains(cache_line)) {
        distance = tree->get_distance(cache_line);
        if (distance >= DIST_INFINITY) {
            distance = DIST_INFINITY - 1;
        }
    }

    gsl_histogram_increment(histogram_list[var_id], distance);
    tree->insert(&mem_info);
}

static inline void fill_trace_struct(int read_write, int line_number,
        size_t base, size_t p, int var_idx) {
    // If this process was never supposed to record stats
    // or if the file-open failed, then return
    if (fd < 0)
        return;

    if (sleeping == 1 || access_count >= 131072)    // 131072 is 128*1024.
        return;

    size_t address_base = (size_t) base;
    size_t address = (size_t) p;

    node_t node;
    node.type_message = MSG_TRACE_INFO;

    node.trace_info.coreID = getCoreID();
    node.trace_info.read_write = read_write;
    node.trace_info.base = address_base;
    node.trace_info.address = address;
    node.trace_info.var_idx = var_idx;
    node.trace_info.line_number = line_number;

    write(fd, &node, sizeof(node_t));
}

static inline void fill_mem_struct(int read_write, int line_number, size_t p,
        int var_idx, int type_size) {
    // If this process was never supposed to record stats
    // or if the file-open failed, then return
    if (fd < 0)
        return;

    if (sleeping == 1 || access_count >= 131072)    // 131072 is 128*1024.
        return;

    node_t node;
    node.type_message = MSG_MEM_INFO;

    node.mem_info.coreID = getCoreID();
    node.mem_info.read_write = read_write;
    node.mem_info.address = p;
    node.mem_info.var_idx = var_idx;
    node.mem_info.line_number = line_number;
    node.mem_info.type_size = type_size;

    write(fd, &node, sizeof(node_t));
}

static void indigo__gen_trace_c(int read_write, int line_number, void* base,
        void* addr, int var_idx) {
    if (fd >= 0)
        fill_trace_struct(read_write, line_number, (size_t) base,
                (size_t) addr, var_idx);
}

static void indigo__gen_trace_f(int *read_write, int *line_number, void* base,
        void* addr, int *var_idx) {
    if (fd >= 0)
        fill_trace_struct(*read_write, *line_number, (size_t) base,
                (size_t) addr, *var_idx);
}

static void indigo__record_c(int read_write, int line_number, void* addr,
        int var_idx, int type_size) {
    if (fd >= 0)
        fill_mem_struct(read_write, line_number, (size_t) addr, var_idx,
                type_size);
}

static void indigo__record_f_(int *read_write, int *line_number, void* addr,
        int *var_idx, int* type_size) {
    if (fd >= 0)
        fill_mem_struct(*read_write, *line_number, (size_t) addr, *var_idx,
                *type_size);
}

void indigo__write_idx_c(const char* var_name, const int length) {
    node_t node;
    node.type_message = MSG_STREAM_INFO;
#define indigo__MIN(a, b)   (a) < (b) ? (a) : (b)
    int dst_len = indigo__MIN(STREAM_LENGTH-1, length);
#undef indigo__MIN

    strncpy(node.stream_info.stream_name, var_name, dst_len);
    node.stream_info.stream_name[dst_len] = '\0';

    std::string stream_name(node.stream_info.stream_name);
    stream_list.push_back(stream_name);

    if (fd >= 0) {
        write(fd, &node, sizeof(node_t));
    }
}

static void indigo__write_idx_f_(const char* var_name, const int* length) {
    indigo__write_idx_c(var_name, *length);
}

static void create_output_file() {
    char szFilename[32];
    snprintf(szFilename, sizeof(szFilename), "macpo.%d.out", getpid());

    fd = open(szFilename, O_CREAT | O_APPEND | O_WRONLY | O_TRUNC, S_IRUSR |
            S_IWUSR | S_IRGRP);
    if (fd < 0) {
        perror("MACPO :: Error opening log for writing");
        exit(1);
    }

    if (access("macpo.out", F_OK) == 0) {
        // file exists, remove it
        if (unlink("macpo.out") == -1)
            perror("MACPO :: Failed to remove macpo.out");
    }

    // Now create the symlink
    if (symlink(szFilename, "macpo.out") == -1)
        perror("MACPO :: Failed to create symlink \"macpo.out\"");

    // Now that we are done handling the critical stuff,
    // write the metadata log to the macpo.out file.
    node_t node;
    node.type_message = MSG_METADATA;
    size_t exe_path_len = readlink("/proc/self/exe",
            node.metadata_info.binary_name, STRING_LENGTH-1);
    if (exe_path_len == -1) {
        perror("MACPO :: Failed to read binary name from /proc/self/exe");
    } else {
        // Write the terminating character
        node.metadata_info.binary_name[exe_path_len] = '\0';
        time(&node.metadata_info.execution_timestamp);
        write(fd, &node, sizeof(node_t));
    }

    terminal_node.type_message = MSG_TERMINAL;
}

static void signalHandler(int sig) {
    // Reset the signal handler
    signal(sig, signalHandler);

    struct itimerval itimer_old, itimer_new;
    itimer_new.it_interval.tv_sec = 0;
    itimer_new.it_interval.tv_usec = 0;

    if (sleeping == 1) {
        // Wake up for a brief period of time
        if (fd >= 0) {
            fdatasync(fd);
            write(fd, &terminal_node, sizeof(node_t));
        }

        // Don't reorder so that `sleeping = 0' remains after fwrite()
        asm volatile("" ::: "memory");
        sleeping = 0;

        itimer_new.it_value.tv_sec = AWAKE_SEC;
        itimer_new.it_value.tv_usec = AWAKE_USEC;
        setitimer(ITIMER_PROF, &itimer_new, &itimer_old);
    } else {
        // Go to sleep now...
        sleeping = 1;

        int temp = sleep_sec + new_sleep_sec;
        sleep_sec = new_sleep_sec;
        new_sleep_sec = temp;

        itimer_new.it_value.tv_sec = sleep_sec;
        itimer_new.it_value.tv_usec = 0;
        setitimer(ITIMER_PROF, &itimer_new, &itimer_old);
        access_count = 0;
    }
}

static void set_timers() {
    // Set up the signal handler
    signal(SIGPROF, signalHandler);

    struct itimerval itimer_old, itimer_new;
    itimer_new.it_interval.tv_sec = 0;
    itimer_new.it_interval.tv_usec = 0;

    if (sleep_sec != 0) {
        sleeping = 1;

        itimer_new.it_value.tv_sec = 3+sleep_sec*4;
        itimer_new.it_value.tv_usec = 0;
        setitimer(ITIMER_PROF, &itimer_new, &itimer_old);
    } else {
        sleeping = 0;

        itimer_new.it_value.tv_sec = AWAKE_SEC;
        itimer_new.it_value.tv_usec = AWAKE_USEC;
        setitimer(ITIMER_PROF, &itimer_new, &itimer_old);
    }
}

static void set_thread_affinity() {
    int info[4];
    int proc = get_proc_kind();
    if (proc == PROC_UNKNOWN) {
        fprintf(stderr, "MACPO :: Cannot determine processor identification, "
                "resorting to defaults...\n");
    } else if (proc == PROC_INTEL) {
        numCores = sysconf(_SC_NPROCESSORS_CONF);
        intel_apic_mapping = reinterpret_cast<int*>(malloc (sizeof (int) *
                numCores));

        if (intel_apic_mapping) {
            // Get the original affinity mask
            cpu_set_t old_mask;
            CPU_ZERO(&old_mask);
            sched_getaffinity(0, sizeof(cpu_set_t), &old_mask);

            // Loop over all cores and find map their APIC IDs to core IDs
            int i;
            for (i = 0; i < numCores; i++) {
                cpu_set_t mask;
                CPU_ZERO(&mask);
                CPU_SET(i, &mask);

                if (sched_setaffinity(0, sizeof(cpu_set_t), &mask) != -1) {
                    // Get the APIC ID
                    __cpuid(info, 0xB, 0);
                    if (info[EBX] != 0) {   // x2APIC
                        __cpuid(info, 0xB, 2);
                        intel_apic_mapping[i] = info[EDX];
                    } else {  // Traditonal APIC
                        __cpuid(info, 1, 0);
                        intel_apic_mapping[i] = (info[EBX] & 0xff000000) >> 24;
                    }

#ifdef DEBUG_PRINT
                    fprintf(stderr, "MACPO :: Registered mapping from core %d "
                            "to APIC %d\n", i, intel_apic_mapping[i]);
#endif
                }
            }

            // Reset the original affinity mask
            sched_setaffinity(0, sizeof(cpu_set_t), &old_mask);
        } else {
#ifdef DEBUG_PRINT
            fprintf(stderr, "MACPO :: malloc() failed\n");
#endif
        }
    }
}

void indigo__init_(int16_t create_file, int16_t enable_sampling) {
    set_thread_affinity();

    if (create_file) {
        create_output_file();
    }

    if (enable_sampling) {
        set_timers();
    } else {
        // Explicitly set awake mode to ON.
        sleeping = 0;
    }

    atexit(indigo__exit);
}
