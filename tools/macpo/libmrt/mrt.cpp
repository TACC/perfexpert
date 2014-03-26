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

#include <algorithm>
#include <cstdarg>
#include <limits.h>
#include <map>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sched.h>

#include <set>
#include <unistd.h>

#include "mrt.h"
#include "macpo_record.h"

typedef std::pair<long, short> val_idx_pair;
typedef std::pair<int, short> line_threadid_pair;

typedef short thread_id;
typedef int line_number;

typedef std::map<thread_id, bool> bool_map;
typedef std::map<thread_id, short> short_map;
typedef std::map<thread_id, long*> long_histogram;

typedef std::map<long, bool_map> bool_map_coll;
typedef std::map<long, short_map> short_map_coll;
typedef std::map<long, long_histogram> long_histogram_coll;

static std::set<int> analyzed_loops;

static bool_map_coll overlap_bin;
static short_map_coll branch_bin, align_bin, sstore_align_bin;
static long_histogram_coll tripcount_map;

static std::map<int, int> branch_loop_line_pair;
static std::map<int, std::set<int> > loop_branch_line_pair;

volatile static short lock_var;

static inline void lock() {
    while (__sync_bool_compare_and_swap(&lock_var, 0, 1) == false)
        ;

    asm volatile("lfence" ::: "memory");
}

static inline void unlock() {
    lock_var = 0;
    asm volatile("sfence" ::: "memory");
}

static bool index_comparator(const val_idx_pair& v1, const val_idx_pair& v2) {
    return v1.first < v2.first;
}

static void print_tripcount_histogram(int line_number, long* histogram) {
    val_idx_pair pair_histogram[MAX_HISTOGRAM_ENTRIES];
    for (int i=0; i<MAX_HISTOGRAM_ENTRIES; i++) {
        pair_histogram[i] = val_idx_pair(histogram[i], i);
    }

    std::sort(pair_histogram, pair_histogram + MAX_HISTOGRAM_ENTRIES);
    if (pair_histogram[MAX_HISTOGRAM_ENTRIES-1].first > 0) {
        // At least one non-zero entry.
        int values[3] = { pair_histogram[MAX_HISTOGRAM_ENTRIES-1].second,
            pair_histogram[MAX_HISTOGRAM_ENTRIES-2].second,
            pair_histogram[MAX_HISTOGRAM_ENTRIES-3].second
        };

        long counts[3] = { pair_histogram[MAX_HISTOGRAM_ENTRIES-1].first,
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
            fprintf (stderr, "Found low trip count (%d) for this loop, "
                "insert the following pragma just before the loop body: "
                "#pragma loop_count(%d)\n", low_tripcount, low_tripcount);
        }
    }
}

static bool nonzero_alignment(int line_number, long* histogram) {
    for (int i=1; i<CACHE_LINE_SIZE; i++) {
        if (histogram[i] > 0)
            return true;
    }

    return false;
}

static void print_alignment_histogram(int line_number, long* histogram) {
    val_idx_pair pair_histogram[CACHE_LINE_SIZE];
    for (int i=0; i<CACHE_LINE_SIZE; i++) {
        pair_histogram[i] = val_idx_pair(histogram[i], i);
    }

    std::sort(pair_histogram, pair_histogram + CACHE_LINE_SIZE);
    if (pair_histogram[CACHE_LINE_SIZE-1].first > 0) {
        // At least one non-zero entry.
        int values[3] = { pair_histogram[CACHE_LINE_SIZE-1].second,
            pair_histogram[CACHE_LINE_SIZE-2].second,
            pair_histogram[CACHE_LINE_SIZE-3].second
        };

        long counts[3] = { pair_histogram[CACHE_LINE_SIZE-1].first,
            pair_histogram[CACHE_LINE_SIZE-2].first,
            pair_histogram[CACHE_LINE_SIZE-3].first
        };

        fprintf (stderr, "  count %d%s found %ld time(s), ", values[0],
                values[0] == MAX_HISTOGRAM_ENTRIES-1 ? "+" : "", counts[0]);
        if (counts[1]) {
            fprintf (stderr, "count %d%s found %ld time(s), ", values[1],
                    values[1] == MAX_HISTOGRAM_ENTRIES-1 ? "+" : "", counts[1]);

            if (counts[2]) {
                fprintf (stderr, "count %d%s found %ld time(s), ", values[2],
                        values[2] == MAX_HISTOGRAM_ENTRIES-1 ? "+" : "",
                        counts[2]);
            }
        }

        fprintf (stderr, "\n");
    }
}

void indigo__exit() {
    if (fd >= 0)   
        close(fd);

    if (intel_apic_mapping)
        free(intel_apic_mapping);

    for (std::set<int>::iterator it = analyzed_loops.begin();
            it != analyzed_loops.end(); it++) {
        const int& line_number = *it;

        fprintf (stderr, "\n==== Loop at line %d ====\n", line_number);

        if (sstore_align_bin.find(line_number) != sstore_align_bin.end()) {
            short_map& map = sstore_align_bin[line_number];
            short align_status = ALIGN_NOINIT;
            for (short_map::iterator it = map.begin(); it != map.end();
                    it++) {
                short _align_status = it->second;
                if (_align_status < align_status)
                    align_status = _align_status;
            }

            switch(align_status) {
                case FULL_ALIGNED:
                    fprintf (stderr, "all non-temporal arrays align, use "
                            "__assume_aligned() or #pragma vector aligned to "
                            "tell compiler about alignment.\n");
                    break;

                case MUTUAL_ALIGNED:
                    fprintf (stderr, "all non-temporal arrays are mutually "
                            "aligned but not aligned to cache-line boundary, "
                            "use _mm_malloc/_mm_free to allocate/free aligned "
                            "storage.\n");
                    break;

                case NOT_ALIGNED:
                    fprintf (stderr, "non-temporal arrays are not aligned, "
                            "try using _mm_malloc/_mm_free to allocate/free "
                            "arrays that are aligned with cache-line "
                            "bounary.\n");
                    break;
            }
        }

        if (align_bin.find(line_number) != align_bin.end()) {
            short_map& map = align_bin[line_number];
            short align_status = ALIGN_NOINIT;
            for (short_map::iterator it = map.begin(); it != map.end();
                    it++) {
                short _align_status = it->second;
                if (_align_status < align_status)
                    align_status = _align_status;
            }

            switch(align_status) {
                case FULL_ALIGNED:
                    fprintf (stderr, "all arrays align, use "
                            "__assume_aligned() or #pragma vector aligned to "
                            "tell compiler about alignment.\n");
                    break;

                case MUTUAL_ALIGNED:
                    fprintf (stderr, "all arrays are mutually aligned but "
                            "not aligned to cache-line boundary, use "
                            "_mm_malloc/_mm_free to allocate/free aligned "
                            "storage.\n");
                    break;

                case NOT_ALIGNED:
                    fprintf (stderr, "arrays are not aligned, try using "
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
                fprintf (stderr, "Use `restrict' keyword to inform "
                        "compiler that arrays do not overlap.\n");
            }
        }

        if (tripcount_map.find(line_number) != tripcount_map.end()) {
            long_histogram& histogram = tripcount_map[line_number];
            for (long_histogram::iterator it = histogram.begin();
                    it != histogram.end(); it++) {
                long* histogram = it->second;

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
                    short branch_status = BRANCH_NOINIT;
                    for (short_map::iterator it = map.begin(); it != map.end();
                            it++) {
                        short _branch_status = it->second;
                        if (_branch_status < branch_status)
                            branch_status = _branch_status;
                    }

                    switch(branch_status) {
                        case BRANCH_UNKNOWN:
                            fprintf (stderr, "branch at line %d is "
                                    "unpredictable.\n", branch_line_number);
                            break;

                        case BRANCH_MOSTLY_FALSE:
                            fprintf (stderr, "branch at line %d mostly "
                                    "evaluates to false.\n",
                                    branch_line_number);
                            break;

                        case BRANCH_MOSTLY_TRUE:
                            fprintf (stderr, "branch at line %d mostly "
                                    "evaluates to true.\n",
                                    branch_line_number);
                            break;

                        case BRANCH_FALSE:
                            fprintf (stderr, "branch at line %d always "
                                    "evaluates to false, use "
                                    "__builtin_expect() to inform compiler "
                                    "about expected branch outcome.\n",
                                    branch_line_number);
                            break;

                        case BRANCH_TRUE:
                            fprintf (stderr, "branch at line %d always "
                                    "evaluates to true, use "
                                    "__builtin_expect() to inform compiler "
                                    "about expected branch outcome.\n",
                                    branch_line_number);
                            break;
                    }
                }
            }
        }
    }
}

short& get_branch_bin(int line_number, int loop_line_number) {
    analyzed_loops.insert(loop_line_number);
    short core_id = getCoreID();

    // Obtain lock.
    lock();

    long* return_value = NULL;
    // Check if we need to allocate a new histogram.
    short_map_coll::iterator it = branch_bin.find(line_number);
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

long* new_histogram(size_t histogram_entries) {
    long* histogram = (long*) malloc(sizeof(long) * histogram_entries);
    if (histogram) {
        memset(histogram, 0, sizeof(long) * histogram_entries);
    }

    return histogram;
}

bool& get_overlap_bin(int line_number) {
    analyzed_loops.insert(line_number);
    short core_id = getCoreID();

    // Obtain lock.
    lock();

    long* return_value = NULL;
    // Check if we need to allocate a new histogram.
    bool_map_coll::iterator it = overlap_bin.find(line_number);
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

long* get_histogram(long_histogram_coll& collection, int line_number,
        size_t histogram_entries) {
    short core_id = getCoreID();

    // Obtain lock.
    lock();

    long* return_value = NULL;
    // Check if we need to allocate a new histogram.
    long_histogram_coll::iterator it = collection.find(line_number);
    if (it == collection.end()) {
        long_histogram& histogram = collection[line_number];
        long_histogram::iterator it = histogram.find(core_id);
        if (it == histogram.end()) {
            histogram[core_id] = new_histogram(histogram_entries);
            return_value = histogram[core_id];
        } else {
            return_value = it->second;
        }
    } else {
        long_histogram& histogram = it->second;
        long_histogram::iterator it = histogram.find(core_id);
        if (it == histogram.end()) {
            histogram[core_id] = new_histogram(histogram_entries);
            return_value = histogram[core_id];
        } else {
            return_value = it->second;
        }
    }

    // Release lock.
    unlock();

    return return_value;
}

short& get_sstore_alignment_bin(int line_number) {
    analyzed_loops.insert(line_number);
    short core_id = getCoreID();

    // Obtain lock.
    lock();

    long* return_value = NULL;
    // Check if we need to allocate a new histogram.
    short_map_coll::iterator it = sstore_align_bin.find(line_number);
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

short& get_alignment_bin(int line_number) {
    analyzed_loops.insert(line_number);
    short core_id = getCoreID();

    // Obtain lock.
    lock();

    long* return_value = NULL;
    // Check if we need to allocate a new histogram.
    short_map_coll::iterator it = align_bin.find(line_number);
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

long* get_tripcount_histogram(int line_number) {
    analyzed_loops.insert(line_number);
    return get_histogram(tripcount_map, line_number, MAX_HISTOGRAM_ENTRIES);
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

    long remainder = -1;
    for (i=0; i<stream_count; i++) {
        void* address = va_arg(args, void*);
        long _remainder = ((long) address) % 64;
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

    long remainder = -1;
    for (i=0; i<stream_count; i++) {
        void* address = va_arg(args, void*);
        long _remainder = ((long) address) % 64;
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

void indigo__tripcount_check_c(int line_number, long trip_count) {
    long* histogram = get_tripcount_histogram(line_number);
    if (histogram == NULL)
        return;

    if (trip_count < 0)
        trip_count = 0;

    if (trip_count >= MAX_HISTOGRAM_ENTRIES)
        trip_count = MAX_HISTOGRAM_ENTRIES-1;

    if (histogram[trip_count] < LONG_MAX)
        histogram[trip_count]++;
}

void set_thread_affinity() {
    int info[4];
    int proc = get_proc_kind();
    if (proc == PROC_UNKNOWN) {
        fprintf (stderr, "MACPO :: Cannot determine processor identification, "
                "resorting to defaults...\n");
    } else if (proc == PROC_INTEL) {
        numCores = sysconf(_SC_NPROCESSORS_CONF);
        intel_apic_mapping = (int*) malloc (sizeof (int) * numCores);

        if (intel_apic_mapping) {
            // Get the original affinity mask
            cpu_set_t old_mask;
            CPU_ZERO(&old_mask);
            sched_getaffinity(0, sizeof(cpu_set_t), &old_mask);

            // Loop over all cores and find map their APIC IDs to core IDs
            int i;
            for (i=0; i<numCores; i++) {
                cpu_set_t mask;
                CPU_ZERO(&mask);
                CPU_SET(i, &mask);

                if (sched_setaffinity(0, sizeof(cpu_set_t), &mask) != -1) {
                    // Get the APIC ID
                    __cpuid(info, 0xB, 0);
                    if (info[EBX] != 0) { // x2APIC
                        __cpuid(info, 0xB, 2);
                        intel_apic_mapping[i] = info[EDX];
                    } else {  // Traditonal APIC
                        __cpuid(info, 1, 0);
                        intel_apic_mapping[i] = (info[EBX] & 0xff000000) >> 24;
                    }

#ifdef DEBUG_PRINT
                    fprintf (stderr, "MACPO :: Registered mapping from core %d "
                            "to APIC %d\n", i, intel_apic_mapping[i]);
#endif
                }
            }

            // Reset the original affinity mask
            sched_setaffinity (0, sizeof(cpu_set_t), &old_mask);
        }
#ifdef DEBUG_PRINT
        else {
            fprintf (stderr, "MACPO :: malloc() failed\n");
        }
#endif
    }
}

