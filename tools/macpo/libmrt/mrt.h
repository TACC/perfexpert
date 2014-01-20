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

#ifndef	LIBMRT_H_
#define	LIBMRT_H_

#include <stdlib.h>
#include <signal.h>

#include "macpo_record.h"

#define	AWAKE_SEC		0

/***

The following analysis takes the numbers from Stampede as reference. Using a
different referene machine will influence the duration of the sampling window
(AWAKE_USEC).

Let's first set the 'infinite' reuse distance to something based on intuition.
Consider that any reuse distance more than the twice the size of the L3 cache is
'too high' to be of use while tracking. The L3 cache on Stampede (Sandy Bridge
process) is 20MB. We can, thus, set the infinite reuse distance value to 40MB.
Assuming each cache line is 64 bytes long, the reuse distance is equal to 640K
cache lines.

Experience from analysing programs using PerfExpert tells that most
poorly-performing programs that are bottlenecked on memory typically take about
3 cycles on each memory access. Assuming out-of-order instruction execution
does not leave any bubbles among access to the memory subsystem, MACPO needs to
keep the instrumentation window for 640K * 3 cycles = 1.92M cycles.

Note that for most multi-threaded programs, a shorter time interval is okay as
well. Multiple threads will access the memory simultaneously, thus filling up
the 640K cache line 'buffer' faster than a single-threaded program.

The Sandy Bridges on Stampede are clocked at 2.7GHz, so 1.92M cycles correspond
to 711 micro-seconds. And thus, we set AWAKE_USEC to 711.

*/

#ifndef	AWAKE_USEC
#define	AWAKE_USEC		711
#endif

#define ALIGN_ENTRIES           3
#define CACHE_LINE_SIZE         64
#define MAX_HISTOGRAM_ENTRIES   1024

static size_t numCores = 0;
static __thread int coreID=-1;
static char szFilename[256]={0};
static volatile sig_atomic_t sleeping=0, access_count=0;
static int fd=-1, sleep_sec=0, new_sleep_sec=1, *intel_apic_mapping=NULL;
static node_t terminal_node;

enum { BRANCH_SINGLE=0, BRANCH_SIMD, BRANCH_UNKNOWN };

#if	defined(__cplusplus)
extern "C" {
#endif
void indigo__init_(short create_file);
#if	defined(__cplusplus)
}
#endif

#if defined(__cplusplus)
extern "C" {
#endif
static void indigo__exit();
#if defined (__cplusplus)
}
#endif

#if defined(__cplusplus)
extern "C" {
#endif
void indigo__write_idx_c(const char* var_name, const int length);
#if defined (__cplusplus)
}
#endif

#if defined(__cplusplus)
extern "C" {
#endif
void indigo__write_idx_f_(const char* var_name, const int* length);
#if defined (__cplusplus)
}
#endif

static inline int getCoreID();

static inline void signalHandler(int sig);
static inline void fill_struct(int read_write, int line_number, size_t p, int var_idx);

#if defined(__cplusplus)
extern "C" {
#endif
void indigo__gen_trace_c(int read_write, int line_number, void* base, void* addr, int var_idx);
#if defined (__cplusplus)
}
#endif

#if defined(__cplusplus)
extern "C" {
#endif
void indigo__gen_trace_f(int* read_write, int* line_number, void* base, void* addr, int* var_idx);
#if defined (__cplusplus)
}
#endif

#if defined(__cplusplus)
extern "C" {
#endif
void indigo__record_c(int read_write, int line_number, void* addr, int var_idx);
#if defined (__cplusplus)
}
#endif

#if defined(__cplusplus)
extern "C" {
#endif
void indigo__record_f_(int* read_write, int* line_number, void* addr, int* var_idx);
#if defined (__cplusplus)
}
#endif

#if defined(__cplusplus)
extern "C" {
#endif
void indigo__simd_branch_c(int line_number, int idxv, int branch_dir, int common_alignment, int* recorded_simd_branch_dir);
#if defined (__cplusplus)
}
#endif

#if defined(__cplusplus)
extern "C" {
#endif
int indigo__aligncheck_c(int line_number, int stream_count, ...);
#if defined (__cplusplus)
}
#endif

#if defined(__cplusplus)
extern "C" {
#endif
void indigo__sstore_aligncheck_c(int line_number, int stream_count, ...);
#if defined (__cplusplus)
}
#endif

#if defined(__cplusplus)
extern "C" {
#endif
void indigo__tripcount_check_c(int line_number, long trip_count);
#if defined (__cplusplus)
}
#endif

#endif	/* LIBMRT_H_ */
