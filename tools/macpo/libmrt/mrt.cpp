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

#include <sched.h>

#include <csignal>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#include <algorithm>
#include <map>

#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>

#include "mrt.h"
#include "cpuid.h"
#include "macpo_record.h"

typedef std::pair<long, short> val_idx_pair;
typedef std::pair<int, short> line_threadid_pair;

static short proc = PROC_UNKNOWN;
static std::map<line_threadid_pair, long*> sstore_align_map, align_map,
        tripcount_map;
static std::map<line_threadid_pair, bool> overlap_bin;
static std::map<line_threadid_pair, short> branch_bin;
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

static int getCoreID()
{
	if (coreID != -1)
		return coreID;

	int info[4];
	if (!isCPUIDSupported())
	{
		coreID = 0;
		return coreID;	// default
	}

	if (proc == PROC_AMD)
	{
		__cpuid(info, 1, 0);
		coreID = (info[1] & 0xff000000) >> 24;
		return coreID;
	}
	else if (proc == PROC_INTEL)
	{
		int apic_id = 0;
		__cpuid(info, 0xB, 0);
		if (info[EBX] != 0)	// x2APIC
		{
			__cpuid(info, 0xB, 2);
			apic_id = info[EDX];

			#ifdef DEBUG_PRINT
				fprintf (stderr, "MACPO :: Request from core with x2APIC ID %d\n", apic_id);
			#endif
		}
		else			// Traditonal APIC
		{
			__cpuid(info, 1, 0);
			apic_id = (info[EBX] & 0xff000000) >> 24;

			#ifdef DEBUG_PRINT
				fprintf (stderr, "MACPO :: Request from core with legacy APIC ID %d\n", apic_id);
			#endif
		}

		int i;
		for (i=0; i<numCores; i++)
			if (apic_id == intel_apic_mapping[i])
				break;

		coreID = i == numCores ? 0 : i;
		return coreID;
	}

	coreID = 0;
	return coreID;
}

static void signalHandler(int sig)
{
	// Reset the signal handler
	signal(sig, signalHandler);

	struct itimerval itimer_old, itimer_new;
	itimer_new.it_interval.tv_sec = 0;
	itimer_new.it_interval.tv_usec = 0;

	if (sleeping == 1)
	{
		// Wake up for a brief period of time
		fdatasync(fd);
		write(fd, &terminal_node, sizeof(node_t));

		// Don't reorder so that `sleeping = 0' remains after fwrite()
		asm volatile("" ::: "memory");
		sleeping = 0;

		itimer_new.it_value.tv_sec = AWAKE_SEC;
		itimer_new.it_value.tv_usec = AWAKE_USEC;
		setitimer(ITIMER_PROF, &itimer_new, &itimer_old);
	}
	else
	{
		// Go to sleep now...
		sleeping = 1;

		int temp = sleep_sec + new_sleep_sec;
		sleep_sec = new_sleep_sec;
		new_sleep_sec = temp;

		itimer_new.it_value.tv_sec = 3+sleep_sec*3;
		itimer_new.it_value.tv_usec = 0;
		setitimer(ITIMER_PROF, &itimer_new, &itimer_old);
		access_count = 0;
	}
}

static void set_thread_affinity() {
    // Get which processor is this running on
    int info[4];
    if (!isCPUIDSupported())
        fprintf (stderr, "MACPO :: CPUID not supported, cannot determine processor core information, resorting to defaults...\n");
    else
    {
        char processorName[13];
        getProcessorName(processorName);

        if (strncmp(processorName, "AuthenticAMD", 12) == 0)		proc = PROC_AMD;
        else if (strncmp(processorName, "GenuineIntel", 12) == 0)	proc = PROC_INTEL;
        else								proc = PROC_UNKNOWN;

        if (proc == PROC_UNKNOWN)
            fprintf (stderr, "MACPO :: Cannot determine processor identification, resorting to defaults...\n");
        else if (proc == PROC_INTEL)	// We need to do some special set up for Intel machines
        {
            numCores = sysconf(_SC_NPROCESSORS_CONF);
            intel_apic_mapping = (int*) malloc (sizeof (int) * numCores);

            if (intel_apic_mapping)
            {
                // Get the original affinity mask
                cpu_set_t old_mask;
                CPU_ZERO(&old_mask);
                sched_getaffinity(0, sizeof(cpu_set_t), &old_mask);

                // Loop over all cores and find map their APIC IDs to core IDs
                for (int i=0; i<numCores; i++)
                {
                    cpu_set_t mask;
                    CPU_ZERO(&mask);
                    CPU_SET(i, &mask);

                    if (sched_setaffinity(0, sizeof(cpu_set_t), &mask) != -1)
                    {
                        // Get the APIC ID
                        __cpuid(info, 0xB, 0);
                        if (info[EBX] != 0)	// x2APIC
                        {
                            __cpuid(info, 0xB, 2);
                            intel_apic_mapping[i] = info[EDX];
                        }
                        else			// Traditonal APIC
                        {
                            __cpuid(info, 1, 0);
                            intel_apic_mapping[i] = (info[EBX] & 0xff000000) >> 24;
                        }

#ifdef DEBUG_PRINT
                        fprintf (stderr, "MACPO :: Registered mapping from core %d to APIC %d\n", i, intel_apic_mapping[i]);
#endif
                    }
                }

                // Reset the original affinity mask
                sched_setaffinity (0, sizeof(cpu_set_t), &old_mask);
            }
#ifdef DEBUG_PRINT
            else
            {
                fprintf (stderr, "MACPO :: malloc() failed\n");
            }
#endif
        }
    }
}

static void create_output_file() {
    char szFilename[32];
    sprintf (szFilename, "macpo.%d.out", (int) getpid());
    fd = open(szFilename, O_CREAT | O_APPEND | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP);
    if (fd < 0)
    {
        perror("MACPO :: Error opening log for writing");
        exit(1);
    }

    if (access("macpo.out", F_OK) == 0)
    {
        // file exists, remove it
        if (unlink("macpo.out") == -1)
            perror ("MACPO :: Failed to remove macpo.out");
    }

    // Now create the symlink
    if (symlink(szFilename, "macpo.out") == -1)
        perror ("MACPO :: Failed to create symlink \"macpo.out\"");

    // Now that we are done handling the critical stuff, write the metadata log to the macpo.out file
    node_t node;
    node.type_message = MSG_METADATA;
    size_t exe_path_len = readlink ("/proc/self/exe", node.metadata_info.binary_name, STRING_LENGTH-1);
    if (exe_path_len == -1)
        perror ("MACPO :: Failed to read name of the binary from /proc/self/exe");
    else
    {
        // Write the terminating character
        node.metadata_info.binary_name[exe_path_len]='\0';
        time(&node.metadata_info.execution_timestamp);

        write(fd, &node, sizeof(node_t));
    }

    terminal_node.type_message = MSG_TERMINAL;
}

static void set_timers() {
	// Set up the signal handler
	signal(SIGPROF, signalHandler);

	struct itimerval itimer_old, itimer_new;
	itimer_new.it_interval.tv_sec = 0;
	itimer_new.it_interval.tv_usec = 0;

	if (sleep_sec != 0)
	{
		sleeping = 1;

		itimer_new.it_value.tv_sec = 3+sleep_sec*4;
		itimer_new.it_value.tv_usec = 0;
		setitimer(ITIMER_PROF, &itimer_new, &itimer_old);
	}
	else
	{
		sleeping = 0;

		itimer_new.it_value.tv_sec = AWAKE_SEC;
		itimer_new.it_value.tv_usec = AWAKE_USEC;
		setitimer(ITIMER_PROF, &itimer_new, &itimer_old);
	}
}

void indigo__init_(short create_file)
{
    set_thread_affinity();

    if (create_file) {
        create_output_file();

        // We could set the timers even if the file did not have to be create.
        // But timers are used to write to the file only. Since there is no
        // other use of timers, we disable setting up timers as well.
        set_timers();
    }

	atexit(indigo__exit);
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
        fprintf (stderr, "MACPO :: Loop at line %d: ", line_number);

        int values[3] = { pair_histogram[MAX_HISTOGRAM_ENTRIES-1].second,
            pair_histogram[MAX_HISTOGRAM_ENTRIES-2].second,
            pair_histogram[MAX_HISTOGRAM_ENTRIES-3].second
        };

        long counts[3] = { pair_histogram[MAX_HISTOGRAM_ENTRIES-1].first,
            pair_histogram[MAX_HISTOGRAM_ENTRIES-2].first,
            pair_histogram[MAX_HISTOGRAM_ENTRIES-3].first
        };

        fprintf (stderr, "count %d%s found %ld times, ", values[0],
                values[0] == MAX_HISTOGRAM_ENTRIES-1 ? "+" : "", counts[0]);
        if (counts[1]) {
            fprintf (stderr, "count %d%s found %ld times, ", values[1],
                    values[1] == MAX_HISTOGRAM_ENTRIES-1 ? "+" : "", counts[1]);

            if (counts[2]) {
                fprintf (stderr, "count %d%s found %ld times, ", values[2],
                        values[2] == MAX_HISTOGRAM_ENTRIES-1 ? "+" : "",
                        counts[2]);
            }
        }

        fprintf (stderr, "\n");
    }
}

static void print_alignment_histogram(int line_number, long* histogram) {
    val_idx_pair pair_histogram[CACHE_LINE_SIZE];
    for (int i=0; i<CACHE_LINE_SIZE; i++) {
        pair_histogram[i] = val_idx_pair(histogram[i], i);
    }

    std::sort(pair_histogram, pair_histogram + CACHE_LINE_SIZE);
    if (pair_histogram[CACHE_LINE_SIZE-1].first > 0) {
        // At least one non-zero entry.
        fprintf (stderr, "MACPO :: Loop at line %d: ", line_number);

        int values[3] = { pair_histogram[CACHE_LINE_SIZE-1].second,
            pair_histogram[CACHE_LINE_SIZE-2].second,
            pair_histogram[CACHE_LINE_SIZE-3].second
        };

        long counts[3] = { pair_histogram[CACHE_LINE_SIZE-1].first,
            pair_histogram[CACHE_LINE_SIZE-2].first,
            pair_histogram[CACHE_LINE_SIZE-3].first
        };

        fprintf (stderr, "count %d%s found %ld times, ", values[0],
                values[0] == MAX_HISTOGRAM_ENTRIES-1 ? "+" : "", counts[0]);
        if (counts[1]) {
            fprintf (stderr, "count %d%s found %ld times, ", values[1],
                    values[1] == MAX_HISTOGRAM_ENTRIES-1 ? "+" : "", counts[1]);

            if (counts[2]) {
                fprintf (stderr, "count %d%s found %ld times, ", values[2],
                        values[2] == MAX_HISTOGRAM_ENTRIES-1 ? "+" : "",
                        counts[2]);
            }
        }

        fprintf (stderr, "\n");
    } else {
        fprintf (stderr, "MACPO :: All entries from alignment histogram in "
                "loop at line %d have the desired alignment.\n", line_number);
    }
}

static void indigo__exit()
{
	if (fd >= 0)	close(fd);
	if (intel_apic_mapping)	free(intel_apic_mapping);

    if (sstore_align_map.size()) {
        fprintf (stderr, "MACPO :: Alignments for streaming stores:\n");
        for (std::map<line_threadid_pair, long*>::iterator it =
                sstore_align_map.begin(); it != sstore_align_map.end(); it++) {
            line_threadid_pair pair = it->first;
            int line_number = pair.first;
            long* histogram = it->second;

            if (histogram) {
                print_alignment_histogram(line_number, histogram);
                free(histogram);
            }
        }
    }

    if (align_map.size()) {
        fprintf (stderr, "MACPO :: Alignments for loop references:\n");
        for (std::map<line_threadid_pair, long*>::iterator it =
                align_map.begin(); it != align_map.end(); it++) {
            line_threadid_pair pair = it->first;
            int line_number = pair.first;
            long* histogram = it->second;

            if (histogram) {
                print_alignment_histogram(line_number, histogram);
                free(histogram);
            }
        }
    }

    if (overlap_bin.size()) {
        fprintf (stderr, "MACPO :: Overlap among loop references:\n");
        for (std::map<line_threadid_pair, bool>::iterator it =
                overlap_bin.begin(); it != overlap_bin.end(); it++) {
            line_threadid_pair pair = it->first;
            int line_number = pair.first;
            bool overlap = it->second;

            if (overlap) {
                fprintf (stderr, "MACPO :: Array references in loop at line %d "
                        "do overlap.\n", line_number);
            } else {
                fprintf (stderr, "MACPO :: No overlap among array references "
                        "in loop at line %d.\n", line_number);
            }
        }
    }

    if (tripcount_map.size()) {
        fprintf (stderr, "MACPO :: Loop trip counts:\n");
        for (std::map<line_threadid_pair, long*>::iterator it =
                tripcount_map.begin(); it != tripcount_map.end(); it++) {
            line_threadid_pair pair = it->first;
            int line_number = pair.first;
            long* histogram = it->second;

            if (histogram) {
                print_tripcount_histogram(line_number, histogram);
                free(histogram);
            }
        }
    }

    if (branch_bin.size()) {
        fprintf (stderr, "MACPO :: Branch profiling results:\n");
        for (std::map<line_threadid_pair, short>::iterator it =
                branch_bin.begin(); it != branch_bin.end(); it++) {
            line_threadid_pair pair = it->first;
            int line_number = pair.first;
            short branch_status = it->second;

            switch(branch_status) {
                case BRANCH_UNKNOWN:
                    fprintf (stderr, "MACPO :: Branch at line %d is "
                            "unpredictable.\n", line_number);
                    break;

                case BRANCH_SIMD:
                    fprintf (stderr, "MACPO :: Branch at line %d can be "
                            "grouped as a vector.\n", line_number);
                    break;

                case BRANCH_SINGLE:
                    fprintf (stderr, "MACPO :: Branch at line %d is always "
                            "evaluated in one direction.\n", line_number);
                    break;
            }
        }
    }
}

static inline void fill_trace_struct(int read_write, int line_number, size_t base, size_t p, int var_idx)
{
	// If this process was never supposed to record stats
	// or if the file-open failed, then return
	if (fd < 0)	return;

	if (sleeping == 1 || access_count >= 131072)	// 131072 is 128*1024 (power of two)
		return;

	size_t address_base = (size_t) base >> 6;	// Shift six bits to right so that we can track cache line assuming cache line size is 64 bytes
	size_t address = (size_t) p >> 6;

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

static inline void fill_mem_struct(int read_write, int line_number, size_t p, int var_idx)
{
	// If this process was never supposed to record stats
	// or if the file-open failed, then return
	if (fd < 0)	return;

	if (sleeping == 1 || access_count >= 131072)	// 131072 is 128*1024 (power of two)
		return;

	size_t address = (size_t) p >> 6;	// Shift six bits to right so that we can track cache line assuming cache line size is 64 bytes

	node_t node;
	node.type_message = MSG_MEM_INFO;

	node.mem_info.coreID = getCoreID();
	node.mem_info.read_write = read_write;
	node.mem_info.address = address;
	node.mem_info.var_idx = var_idx;
	node.mem_info.line_number = line_number;

	write(fd, &node, sizeof(node_t));
}

void indigo__gen_trace_c(int read_write, int line_number, void* base, void* addr, int var_idx)
{
	if (fd >= 0)	fill_trace_struct(read_write, line_number, (size_t) base, (size_t) addr, var_idx);
}

void indigo__gen_trace_f(int *read_write, int *line_number, void* base, void* addr, int *var_idx)
{
	if (fd >= 0)	fill_trace_struct(*read_write, *line_number, (size_t) base, (size_t) addr, *var_idx);
}

short& get_branch_bin(int line_number) {
    line_threadid_pair pair(line_number, getCoreID());

    // Obtain lock.
    lock();

    std::map<line_threadid_pair, short>::iterator it = branch_bin.find(pair);
    if (it == branch_bin.end()) {
        branch_bin[pair] = BRANCH_SINGLE;
    }

    // Release lock.
    unlock();

    return branch_bin[pair];
}

void indigo__simd_branch_c(int line_number, int idxv, int branch_dir, int common_alignment, int* recorded_simd_branch_dir)
{
    int simd_index = idxv - common_alignment/4;
	if (common_alignment >= 0 && simd_index >= 0) {
        if (get_branch_bin(line_number) != BRANCH_UNKNOWN) {
            if (*recorded_simd_branch_dir == -1)
                *recorded_simd_branch_dir = branch_dir;

            if (simd_index > 0) {
                // Check if branch_dir is the same as previously recorded dirs.
                if (*recorded_simd_branch_dir != branch_dir) {
                    get_branch_bin(line_number) = BRANCH_UNKNOWN;
                }
            } else {
                // Set the branch_dir value for subsequent iterations.
                if (*recorded_simd_branch_dir != branch_dir)
                    get_branch_bin(line_number) = BRANCH_SIMD;
                *recorded_simd_branch_dir = branch_dir;
            }
        }
    }
}

void indigo__record_c(int read_write, int line_number, void* addr, int var_idx)
{
	if (fd >= 0)	fill_mem_struct(read_write, line_number, (size_t) addr, var_idx);
}

void indigo__record_f_(int *read_write, int *line_number, void* addr, int *var_idx)
{
	if (fd >= 0)	fill_mem_struct(*read_write, *line_number, (size_t) addr, *var_idx);
}

void indigo__write_idx_c(const char* var_name, const int length)
{
	node_t node;
	node.type_message = MSG_STREAM_INFO;
#define	indigo__MIN(a,b)	(a) < (b) ? (a) : (b)
	int dst_len = indigo__MIN(STREAM_LENGTH-1, length);
#undef indigo__MIN

	strncpy(node.stream_info.stream_name, var_name, dst_len);
	node.stream_info.stream_name[dst_len] = '\0';

	write(fd, &node, sizeof(node_t));
}

void indigo__write_idx_f_(const char* var_name, const int* length)
{
	indigo__write_idx_c(var_name, *length);
}

long* new_tripcount_histogram() {
    long* histogram = (long*) malloc(sizeof(long) * MAX_HISTOGRAM_ENTRIES);
    if (histogram) {
        memset(histogram, 0, sizeof(long) * MAX_HISTOGRAM_ENTRIES);
    }

    return histogram;
}

long* new_alignment_histogram() {
    long* histogram = (long*) malloc(sizeof(long) * CACHE_LINE_SIZE);
    if (histogram) {
        memset(histogram, 0, sizeof(long) * CACHE_LINE_SIZE);
    }

    return histogram;
}

bool& get_overlap_bin(int line_number) {
    line_threadid_pair pair(line_number, getCoreID());

    // Obtain lock.
    lock();

    std::map<line_threadid_pair, bool>::iterator it =
        overlap_bin.find(pair);
    if (it == overlap_bin.end()) {
        overlap_bin[pair] = 0;
    }

    // Release lock.
    unlock();

    return overlap_bin[pair];
}

/**
    Helper function to allocate / get histograms for alignment checking.
*/
long* get_alignment_histogram(int line_number) {
    line_threadid_pair pair(line_number, getCoreID());

    // Obtain lock.
    lock();

    long* return_value = NULL;
    std::map<line_threadid_pair, long*>::iterator it =
        align_map.find(pair);
    if (it == align_map.end()) {
        // Allocate a new histogram.
        align_map[pair] = new_alignment_histogram();
        return_value = align_map[pair];
    } else {
        return_value = it->second;
    }

    // Release lock.
    unlock();

    return return_value;
}

long* get_tripcount_histogram(int line_number) {
    line_threadid_pair pair(line_number, getCoreID());

    // Obtain lock.
    lock();

    long* return_value = NULL;
    std::map<line_threadid_pair, long*>::iterator it =
        tripcount_map.find(pair);
    if (it == tripcount_map.end()) {
        // Allocate a new histogram.
        tripcount_map[pair] = new_tripcount_histogram();
        return_value = tripcount_map[pair];
    } else {
        return_value = it->second;
    }

    // Release lock.
    unlock();

    return return_value;
}

#define MAX_ADDR    128

/**
    indigo__aligncheck_c()
    Checks for alignment to cache line boundary and memory overlap.
    Returns common alignment, if any. Otherwise, returns -1.
*/
int indigo__aligncheck_c(int line_number, int stream_count, ...) {
    va_list args;

    void* start_list[MAX_ADDR] = {0};
    void* end_list[MAX_ADDR] = {0};

    if (stream_count >= MAX_ADDR) {
#if 0
        fprintf (stderr, "MACPO :: Stream count too large, truncating to %d "
                " for memory disambiguation.\n", MAX_ADDR);
#endif

        stream_count = MAX_ADDR-1;
    }

    long* histogram = get_alignment_histogram(line_number);
    if (histogram == NULL)
        return -1;

    int common_alignment = -1, last_remainder = 0;

    int i, j;
    va_start(args, stream_count);

    for (i=0; i<stream_count; i++) {
        void* start = va_arg(args, void*);
        void* end = va_arg(args, void*);

        int remainder = ((long) start) % 64;
        histogram[remainder]++;

        // We already discovered an overlap, don't proceed further.
        if (get_overlap_bin(line_number) == true)
            break;

        if (i == 0) {
            common_alignment = remainder;
        } else {
            if (common_alignment != remainder)
                common_alignment = -1;
        }

#if 0
        if (((long) start) % 64) {
            fprintf (stderr, "MACPO :: Reference in loop at line %d is not "
                    "aligned at cache line boundary.\n", line_number);
        }
#endif

        // Really simple and inefficient (n^2) algorightm to check duplicates.
        bool overlap = false;
        for (j=0; j<i && overlap == false; j++) {
#if 0
            fprintf (stderr, "i: %p-%p, compared against: %p-%p = %d.\n",
                    start_list[j], end_list[j], start, end, start_list[j] > end || end_list[j] < start);
#endif
            if (!(start_list[j] > end || end_list[j] < start)) {
                overlap = true;
            }
        }

        get_overlap_bin(line_number) = overlap;

#if 0
        if (overlap) {
            fprintf (stderr, "MACPO :: Found overlap among references for loop "
                    "at line %d.\n", line_number);
        }
#endif

        start_list[i] = start;
        end_list[i] = end;
    }

    va_end(args);
    return common_alignment;
}

/**
    Helper function to allocate / get histogramss for alignment checking.
*/
long* get_sstore_alignment_histogram(int line_number) {
    line_threadid_pair pair(line_number, getCoreID());

    // Obtain lock.
    lock();

    long* return_value = NULL;
    std::map<line_threadid_pair, long*>::iterator it =
        sstore_align_map.find(pair);
    if (it == sstore_align_map.end()) {
        // Allocate a new histogram.
        sstore_align_map[pair] = new_alignment_histogram();
        return_value = sstore_align_map[pair];
    } else {
        return_value = it->second;
    }

    // Release lock.
    unlock();

    return return_value;
}

/**
    indigo__sstore_aligncheck_c()
    Checks for alignment of streaming stores to cache line boundary.
*/
void indigo__sstore_aligncheck_c(int line_number, int stream_count, ...) {
    va_list args;

    int i, j;
    long* histogram = get_sstore_alignment_histogram(line_number);
    if (histogram == NULL)
        return;

    va_start(args, stream_count);

    for (i=0; i<stream_count; i++) {
        void* addr = va_arg(args, void*);
        int remainder = ((long) addr) % 64;
        histogram[remainder]++;
    }

    va_end(args);
}

void indigo__tripcount_check_c(int line_number, long trip_count) {
    long* histogram = get_tripcount_histogram(line_number);
    if (histogram == NULL)
        return;

    if (trip_count < 0)
        trip_count = 0;

    if (trip_count >= MAX_HISTOGRAM_ENTRIES)
        trip_count = MAX_HISTOGRAM_ENTRIES-1;

    histogram[trip_count]++;
}
