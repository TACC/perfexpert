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

#ifndef TOOLS_MACPO_LIBMRT_MRT_H_
#define TOOLS_MACPO_LIBMRT_MRT_H_

#include <fcntl.h>

#if defined(__cplusplus)
#include <cstdio>
#include <cstdlib>
#include <csignal>
#include <cstring>
#include <string>
#else
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#endif

#include <stdint.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "macpo_record.h"

#define AWAKE_SEC   0

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

#ifndef AWAKE_USEC
#define AWAKE_USEC  711
#endif

#define ALIGN_ENTRIES           3
#define MAX_VARIABLES           32
#define CACHE_LINE_SIZE         64
#define MAX_HISTOGRAM_ENTRIES   1024

enum { EAX = 0, EBX, ECX, EDX };
enum { PROC_UNKNOWN = -1, PROC_INTEL = 0, PROC_AMD };

#ifdef __GNUC__
static void mycpuid(int *p, unsigned int param, unsigned int ecx) {
#ifndef __x86_64
    __asm__ __volatile__(
         "pushl    %%ebx\n\t"
         "cpuid\n\t"
         "movl %%ebx, %1\n\t"
         "popl %%ebx"
         : "=a" (p[0]), "=r" (p[1]), "=c" (p[2]), "=d" (p[3])
         : "a" (param), "c" (ecx)
         : "cc");
#else
    __asm__ __volatile__(
         "cpuid\n\t"
         : "=a" (p[0]), "=b" (p[1]), "=c" (p[2]), "=d" (p[3])
         : "a" (param), "c" (ecx)
         : "cc");
#endif
}
#endif /* __GNUC__ */

#define __cpuid mycpuid

static int isCPUIDSupported() {
#ifdef  _NO_CPUID
    return 0;
#else
#ifndef __x86_64
    int supported = 2;
    asm (
            "pushfl\n\t"
            "popl   %%eax\n\t"
            "movl   %%eax, %%ecx\n\t"
            "xorl   $0x200000, %%eax\n\t"
            "pushl  %%eax\n\t"
            "popfl\n\t"

            "pushfl\n\t"
            "popl   %%eax\n\t"
            "xorl   %%ecx, %%eax\n\t"
            "movl   %%eax, %0\n\t"
            : "=r" (supported)
            :: "eax", "ecx"
        );
#else
    int supported = 2;
    asm (
            "pushfq\n\t"
            "popq   %%rax\n\t"
            "movq   %%rax, %%rcx\n\t"
            "xorq   $0x200000, %%rax\n\t"
            "pushq  %%rax\n\t"
            "popfq\n\t"

            "pushfq\n\t"
            "popq   %%rax\n\t"
            "xorl   %%ecx, %%eax\n\t"
            "movl   %%eax, %0\n\t"
            : "=r" (supported)
            :: "eax", "ecx"
        );
#endif

    return supported != 0;
#endif
}

static void getProcessorName(char* string) {
    int info[4];
    __cpuid(info, 0, 0);
    char processorName[13] = {0};

    int charCounter = 0;

    // Remember that register sequence is EBX, EDX and ECX
    processorName[charCounter++] = info[EBX] & 0xff;
    processorName[charCounter++] = (info[EBX] & 0xff00) >> 8;
    processorName[charCounter++] = (info[EBX] & 0xff0000) >> 16;
    processorName[charCounter++] = (info[EBX] & 0xff000000) >> 24;

    processorName[charCounter++] = info[EDX] & 0xff;
    processorName[charCounter++] = (info[EDX] & 0xff00) >> 8;
    processorName[charCounter++] = (info[EDX] & 0xff0000) >> 16;
    processorName[charCounter++] = (info[EDX] & 0xff000000) >> 24;

    processorName[charCounter++] = info[ECX] & 0xff;
    processorName[charCounter++] = (info[ECX] & 0xff00) >> 8;
    processorName[charCounter++] = (info[ECX] & 0xff0000) >> 16;
    processorName[charCounter++] = (info[ECX] & 0xff000000) >> 24;

    // 13th character in processorName is already a NULL character,
    // so do not inserting it explicitly.
    snprintf(string, sizeof(processorName), "%s", processorName);
}

#if defined(__cplusplus)
extern "C" {
#endif
void indigo__exit();

void indigo__record_branch_c(int line_number, void* func_addr,
    int loop_line_number, int true_branch_count, int false_branch_count);

void indigo__vector_stride_c(int loop_line_number, int var_idx, void* addr,
    int type_size);

int indigo__aligncheck_c(int line_number, void* func_addr, int stream_count,
    int16_t dep_status, ...);

int indigo__sstore_aligncheck_c(int line_number, void* func_addr,
    int stream_count, int16_t dep_status, ...);

void indigo__overlap_check_c(int line_number, void* func_addr,
    int stream_count, ...);

void indigo__tripcount_check_c(int line_number, void* func_addr,
    int64_t trip_count);

void indigo__unknown_stride_check_c(int line_number, void* func_addr);

void indigo__stride_check_c(int line_number, void* func_addr, int stride);

void indigo__reuse_dist_c(int index, void* address);

void indigo__init_(int16_t create_file, int16_t enable_sampling);

void indigo__write_idx_c(const char* var_name, const int length);

void indigo__gen_trace_c(int read_write, int line_number, void* base,
        void* addr, int var_idx);

void indigo__gen_trace_f(int *read_write, int *line_number, void* base,
        void* addr, int *var_idx);

void indigo__record_c(int read_write, int line_number, void* addr,
        int var_idx, int type_size);

void indigo__record_f_(int *read_write, int *line_number, void* addr,
        int *var_idx, int* type_size);

    void indigo__write_idx_c(const char* var_name, const int length);
#if defined (__cplusplus)
}
#endif
void indigo__write_idx_f_(const char* var_name, const int length);

// XXX: Don't change the order of the elements of the enum!
// XXX: The order is used in arithmetic comparison.
enum { NOT_ALIGNED = 0, MUTUAL_ALIGNED, FULL_ALIGNED, ALIGN_NOINIT };
enum { BRANCH_MOSTLY_TRUE = 0, BRANCH_TRUE, BRANCH_MOSTLY_FALSE, BRANCH_FALSE,
    BRANCH_UNKNOWN, BRANCH_NOINIT };
enum { STRIDE_UNKNOWN = 0, STRIDE_FIXED, STRIDE_UNIT, STRIDE_NOINIT };

#endif  // TOOLS_MACPO_LIBMRT_MRT_H_
