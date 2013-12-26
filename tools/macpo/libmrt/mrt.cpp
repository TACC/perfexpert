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

static short proc = PROC_UNKNOWN;
static std::map<int, long*> sstore_align_map, align_map;
static std::map<int, bool> overlap_bin;
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

void indigo__init_()
{
	int i;

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
				for (i=0; i<numCores; i++)
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

	terminal_node.type_message = MSG_TERMINAL;

	// Output file
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

	atexit(indigo__exit);
}

static bool index_comparator(const val_idx_pair& v1, const val_idx_pair& v2) {
    return v1.first < v2.first;
}

static void print_histogram(int line_number, long* histogram) {
    if (histogram) {
        val_idx_pair pair_histogram[CACHE_LINE_SIZE];
        for (int i=0; i<CACHE_LINE_SIZE; i++) {
            pair_histogram[i] = val_idx_pair(histogram[i], i);
        }

        std::sort(pair_histogram, pair_histogram + CACHE_LINE_SIZE);
        if (pair_histogram[CACHE_LINE_SIZE-1].first > 0) {
            // At least one non-zero entry.
            fprintf (stderr, "MACPO :: Top %d entries for loop at line %d: ",
                    ALIGN_ENTRIES, line_number);

            fprintf (stderr, "offset %d found %ld times, ",
                    pair_histogram[CACHE_LINE_SIZE-1].second,
                    pair_histogram[CACHE_LINE_SIZE-1].first);
            fprintf (stderr, "offset %d found %ld times, ",
                    pair_histogram[CACHE_LINE_SIZE-2].second,
                    pair_histogram[CACHE_LINE_SIZE-2].first);
            fprintf (stderr, "offset %d found %ld times.\n",
                    pair_histogram[CACHE_LINE_SIZE-3].second,
                    pair_histogram[CACHE_LINE_SIZE-3].first);
        } else {
            fprintf (stderr, "MACPO :: All entries from alignment histogram "
                    "in loop at line %d have the desired alignment.\n",
                    line_number);
        }
    }
}

static void indigo__exit()
{
	if (fd >= 0)	close(fd);
	if (intel_apic_mapping)	free(intel_apic_mapping);

    fprintf (stderr, "MACPO :: Alignments for streaming stores:\n");
    for (std::map<int, long*>::iterator it = sstore_align_map.begin();
            it != sstore_align_map.end(); it++) {
        int line_number = it->first;
        long* histogram = it->second;
        print_histogram(line_number, histogram);

        if (histogram)
            free(histogram);
    }

    fprintf (stderr, "MACPO :: Alignments for loop references:\n");
    for (std::map<int, long*>::iterator it = align_map.begin();
            it != align_map.end(); it++) {
        int line_number = it->first;
        long* histogram = it->second;
        print_histogram(line_number, histogram);

        if (histogram)
            free(histogram);
    }

    fprintf (stderr, "MACPO :: Overlap among loop references:\n");
    for (std::map<int, bool>::iterator it = overlap_bin.begin();
            it != overlap_bin.end(); it++) {
        int line_number = it->first;
        bool overlap = it->second;

        if (overlap) {
            fprintf (stderr, "MACPO :: Array references in loop at line %d do "
                    "overlap.\n", line_number);
        } else {
            fprintf (stderr, "MACPO :: No overlap among array references in "
                    "loop at line %d.\n", line_number);
        }
    }
}

static inline void fill_struct(int read_write, int line_number, size_t p, int var_idx)
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

void indigo__record_c(int read_write, int line_number, void* addr, int var_idx)
{
	if (fd >= 0)	fill_struct(read_write, line_number, (size_t) addr, var_idx);
}

void indigo__record_f_(int *read_write, int *line_number, void* addr, int *var_idx)
{
	if (fd >= 0)	fill_struct(*read_write, *line_number, (size_t) addr, *var_idx);
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

long* new_histogram() {
    long* histogram = (long*) malloc(sizeof(long) * CACHE_LINE_SIZE);
    if (histogram) {
        memset(histogram, 0, sizeof(long) * CACHE_LINE_SIZE);
    }

    return histogram;
}

bool& get_overlap_bin(int line_number) {
    // Obtain lock.
    lock();

    std::map<int, bool>::iterator it = overlap_bin.find(line_number);
    if (it == overlap_bin.end()) {
        overlap_bin[line_number] = 0;
    }

    // Release lock.
    unlock();

    return overlap_bin[line_number];
}

/**
    Helper function to allocate / get histograms for alignment checking.
*/
long* get_alignment_histogram(int line_number) {
    // Obtain lock.
    lock();

    long* return_value = NULL;
    std::map<int, long*>::iterator it = align_map.find(line_number);
    if (it == align_map.end()) {
        // Allocate a new histogram.
        align_map[line_number] = new_histogram();
        return_value = align_map[line_number];
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
*/
void indigo__aligncheck_c(int line_number, int stream_count, ...) {
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
        return;

    int i, j;
    va_start(args, stream_count);

    for (i=0; i<stream_count; i++) {
        void* start = va_arg(args, void*);
        void* end = va_arg(args, void*);

        int remainder = ((long) start) % 64;
        histogram[remainder]++;

#if 0
        if (((long) start) % 64) {
            fprintf (stderr, "MACPO :: Reference in loop at line %d is not "
                    "aligned at cache line boundary.\n", line_number);
        }
#endif

        // Really simple and inefficient (n^2) algorightm to check duplicates.
        bool overlap = false;
        for (j=0; j<i && overlap == 0; j++) {
            if (!(start_list[j] > end_list[i] || end_list[j] < start_list[i])) {
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
}

/**
    Helper function to allocate / get histogramss for alignment checking.
*/
long* get_sstore_alignment_histogram(int line_number) {
    // Obtain lock.
    lock();

    long* return_value = NULL;
    std::map<int, long*>::iterator it = sstore_align_map.find(line_number);
    if (it == sstore_align_map.end()) {
        // Allocate a new histogram.
        sstore_align_map[line_number] = new_histogram();
        return_value = sstore_align_map[line_number];
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
