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

static size_t numCores = 0;
static __thread int coreID=-1;
static char szFilename[256]={0};
static volatile sig_atomic_t sleeping = 0, access_count = 0;
static int fd = -1, sleep_sec = 0, new_sleep_sec = 1;
static int *intel_apic_mapping = NULL;
static node_t terminal_node;

#if defined(__cplusplus)
extern "C" {
#endif
void set_thread_affinity();
#if defined (__cplusplus)
}
#endif

#if defined(__cplusplus)
extern "C" {
#endif
void indigo__exit();
#if defined (__cplusplus)
}
#endif

#if defined(__cplusplus)
extern "C" {
#endif
void indigo__record_branch_c(int line_number, int loop_line_number,
    int true_branch_count, int false_branch_count);
#if defined (__cplusplus)
}
#endif

#if defined(__cplusplus)
extern "C" {
#endif
void indigo__vector_stride_c(int loop_line_number, int var_idx, void* addr,
    int type_size);
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
int indigo__sstore_aligncheck_c(int line_number, int stream_count, ...);
#if defined (__cplusplus)
}
#endif

#if defined(__cplusplus)
extern "C" {
#endif
void indigo__overlap_check_c(int line_number, int stream_count, ...);
#if defined (__cplusplus)
}
#endif

#if defined(__cplusplus)
extern "C" {
#endif
void indigo__tripcount_check_c(int line_number, int64_t trip_count);
#if defined (__cplusplus)
}
#endif

#if defined(__cplusplus)
extern "C" {
#endif
void indigo__unknown_stride_check_c(int line_number);
#if defined (__cplusplus)
}
#endif

#if defined(__cplusplus)
extern "C" {
#endif
void indigo__stride_check_c(int line_number, int stride);
#if defined (__cplusplus)
}
#endif

// XXX: Don't change the order of the elements of the enum!
// XXX: The order is used in arithmetic comparison.
enum { NOT_ALIGNED = 0, MUTUAL_ALIGNED, FULL_ALIGNED, ALIGN_NOINIT };
enum { BRANCH_MOSTLY_TRUE = 0, BRANCH_TRUE, BRANCH_MOSTLY_FALSE, BRANCH_FALSE,
    BRANCH_UNKNOWN, BRANCH_NOINIT };
enum { STRIDE_UNKNOWN = 0, STRIDE_FIXED, STRIDE_UNIT, STRIDE_NOINIT };

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

static void signalHandler(int sig) {
    // Reset the signal handler
    signal(sig, signalHandler);

    struct itimerval itimer_old, itimer_new;
    itimer_new.it_interval.tv_sec = 0;
    itimer_new.it_interval.tv_usec = 0;

    if (sleeping == 1) {
        // Wake up for a brief period of time
        fdatasync(fd);
        write(fd, &terminal_node, sizeof(node_t));

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

        itimer_new.it_value.tv_sec = 3+sleep_sec*3;
        itimer_new.it_value.tv_usec = 0;
        setitimer(ITIMER_PROF, &itimer_new, &itimer_old);
        access_count = 0;
    }
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

static inline void indigo__init_(int16_t create_file, int16_t enable_sampling) {
    set_thread_affinity();

    if (create_file) {
        create_output_file();

        if (enable_sampling) {
            // We could set the timers even if the file did not have to be
            // create.  But timers are used to write to the file only. Since
            // there is no other use of timers, we disable setting up timers as
            // well.
            set_timers();
        } else {
            // Explicitly set awake mode to ON.
            sleeping = 0;
        }
    }

    atexit(indigo__exit);
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

static void indigo__write_idx_c(const char* var_name, const int length) {
    node_t node;
    node.type_message = MSG_STREAM_INFO;
#define indigo__MIN(a, b)   (a) < (b) ? (a) : (b)
    int dst_len = indigo__MIN(STREAM_LENGTH-1, length);
#undef indigo__MIN

    strncpy(node.stream_info.stream_name, var_name, dst_len);
    node.stream_info.stream_name[dst_len] = '\0';

    write(fd, &node, sizeof(node_t));
}

static void indigo__write_idx_f_(const char* var_name, const int* length) {
    indigo__write_idx_c(var_name, *length);
}

#endif  // TOOLS_MACPO_LIBMRT_MRT_H_
