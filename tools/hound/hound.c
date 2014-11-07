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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#include "err.h"
#include "cpuid.h"
#include "typeDefinitions.h"
#include "discovery_amd.h"
#include "discovery_intel.h"
#include "x86_generic.h"

#define COLLECTION_SIZE 50

char processor = PROC_UNKNOWN;

double get90thPercentile(volatile double* lpCycles, unsigned int count) {
    double temp = 0.0, value = 0.0;
    int i = 0, j = 0;

    // Sort elements
    for (i = 0; i < count - 1; i++) {
        for (j = i + 1; j < count; j++) {
            if (lpCycles[i] > lpCycles[j]) {
                temp = lpCycles[i];
                lpCycles[i] = lpCycles[j];
                lpCycles[j] = temp;
            }
        }
    }

    // Return the 90th percentile from the sorted list
    value = lpCycles[(int) (0.9 * count)];
    if (value < 1.0) {
        return 1.0;
    }
    return value;
}

int insertIntoCacheList(cacheList** lplpCacheList, cacheInfo cache) {
    while (*lplpCacheList != NULL) {
        lplpCacheList = &((*lplpCacheList)->lpNext);
    }

    if (NULL == ((*lplpCacheList) = (cacheList*)malloc(sizeof(cacheList)))) {
        return -ERR_MEMFAILED;
    }

    (*lplpCacheList)->info = cache;
    (*lplpCacheList)->lpNext = NULL;
    return 0;
}

void destroyCacheLists(cacheCollection* lpCaches) {
    cacheList* lpTemp;
    while ((lpTemp = lpCaches->lpL1Caches) != NULL) {
        lpCaches->lpL1Caches = lpTemp->lpNext;
        free(lpTemp);
    }

    while ((lpTemp = lpCaches->lpL2Caches) != NULL) {
        lpCaches->lpL2Caches = lpTemp->lpNext;
        free(lpTemp);
    }

    while ((lpTemp = lpCaches->lpL2Caches) != NULL) {
        lpCaches->lpL2Caches = lpTemp->lpNext;
        free(lpTemp);
    }
}

void discoverCachesWithoutCPUID() {
    double cycleCollection [COLLECTION_SIZE], val;
    unsigned short counter = 1;
    unsigned int cycles;
    int i, j;

    #ifdef    DEBUG_PRINT
        printf ("DEBUG: Discovering caches without using CPUID\n");
    #endif

    // First measure with 1KB
    for (j=0; j<COLLECTION_SIZE; j++) {
        cycleCollection[j] = allocateAndTest(1024, 512, 1);
    }

    val = get90thPercentile(cycleCollection, COLLECTION_SIZE);
    printf ("L1_dlat = %lf\n", val);

    // FIXME: L1 Latency values are being calculated incorrectly
    printf ("L1_ilat = %lf\n", val);

    // Start measuring from L2
    cycles = val;

    for (i=32768; i<4*1024*1024; i+= 32768) {
        for (j=0; j<COLLECTION_SIZE; j++) {
            cycleCollection[j] = allocateAndTest(i, 512, 1);
        }

        val = get90thPercentile(cycleCollection, COLLECTION_SIZE);

        if (val > 5*cycles) {
            #ifdef DEBUG_PRINT
            printf("DEBUG: Found 5x increase in 90th percentile at %d KB\n",
                i / 1024);
            #endif

            if (counter == 1)
                printf ("L%d_dlat = %lf\n", ++counter, val);
            else    // Assuming unified caches beyond L1
                printf ("L%d_lat = %lf\n", ++counter, val);

            cycles = val;

            if (counter == 3) {
                #ifdef DEBUG_PRINT
                printf("DEBUG: Discovered L2, stopping probing caches\n");
                #endif
                break;
            }
        }
    }

    #ifdef DEBUG_PRINT
    printf("DEBUG: Finished probing caches, now probing memory with size: "
        "%lf KB\n", 30 * i / 1024.0);
    #endif

    // Start with mem_size = 30i KB
    for (j=0; j<COLLECTION_SIZE; j++) {
        cycleCollection[j] = allocateAndTestMainMemory(30 * i, 1024, 0);
    }

    #ifdef DEBUG_PRINT
    printf("DEBUG: Finished probing memory\n");
    #endif

    printf("mem_lat = %lf\n",
        get90thPercentile(cycleCollection, COLLECTION_SIZE));
}

int isCacheProbeSupportedByCPUID(unsigned char processor) {
    int info[4];

    if (processor == PROC_INTEL) {
        __cpuid(info, 0, 0);
        if (info[0] >= 0x2)
            return 0;
    } else if (processor == PROC_AMD) {
        __cpuid(info, 0x80000000, 0);
        if (info[0] >= 0x80000006)
            return 0;
    }

    return -1;
}

int main( int argc, char * argv[] ) {
    double cycleCollection [COLLECTION_SIZE];
    int i, j;
    int family, model;
    double frequency = 0;
    FILE *cpuinfo = NULL;
    char line[1024], string[1024];

    #ifdef DEBUG_PRINT
    printf("DEBUG: Debug messages ON\n");
    #endif

    if (NULL == (cpuinfo = fopen("/proc/cpuinfo", "r"))) {
      printf("Error opening /proc/cpuinfo");
      return 1;
    }

    sscanf(line, "cpu MHz :%lf", &frequency);

    while (0 != fgets(line, 1024, cpuinfo)) {
        if (0 == strncmp(line, "cpu family", sizeof("cpu family") - 1)) {
            sscanf(line, "cpu family\t: %d", &family);
            continue;
        }
        if (0 == strncmp(line, "model", sizeof("model") - 1)) {
            sscanf(line, "model\t\t: %d", &model);
            continue;
        }
        if (0 == strncmp(line, "cpu MHz", sizeof("cpu MHz") - 1)) {
            sscanf(line, "cpu MHz\t\t: %s", &string);
            frequency = atof(string);
            continue;
        }
    }
    fclose(cpuinfo);

    #ifdef DEBUG_PRINT
    printf("Family: %d, Model: %d, Frequency: %f\n", family, model, frequency);
    #endif

    if (isCPUIDSupported()) {
        #ifdef DEBUG_PRINT
        printf("DEBUG: CPUID supported, getting processor name...\n");
        #endif

        char processorName[13];
        getProcessorName(processorName);

        if (strcmp(processorName, "GenuineIntel") == 0)
            processor = PROC_INTEL;
        else if (strcmp(processorName, "AuthenticAMD") == 0)
            processor = PROC_AMD;

        #ifdef DEBUG_PRINT
        printf("DEBUG: Processor name was: %s\n", processorName);
        #endif

        if (isCacheProbeSupportedByCPUID(processor) < 0) {
            #ifdef DEBUG_PRINT
            printf("DEBUG: Processor supports CPUID instruction but only "
                "limited information can be retrived. Falling back to probing "
                "using different memory sizes\n");
            #endif

            // CPUID of no use, instead measure using different memory sizes
            // and latency of access
            processor = PROC_UNKNOWN;
        } else {
            #ifdef DEBUG_PRINT
            printf("DEBUG: Cache probing using CPUID instruction supported\n");
            #endif
        }
    } else {
        #ifdef DEBUG_PRINT
        printf("DEBUG: CPUID instruction not supported\n");
        #endif
    }

    // Header
    printf("--\n\
-- Copyright (c) 2011-2013  University of Texas at Austin. All rights reserved.\n\
--\n\
-- $COPYRIGHT$\n\
--\n\
-- Additional copyrights may follow\n\
--\n\
-- This file is part of PerfExpert.\n\
--\n\
-- PerfExpert is free software: you can redistribute it and/or modify it under\n\
-- the terms of the The University of Texas at Austin Research License\n\
--\n\
-- PerfExpert is distributed in the hope that it will be useful, but WITHOUT ANY\n\
-- WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR\n\
-- A PARTICULAR PURPOSE.\n\
--\n\
-- Authors: Leonardo Fialho and Ashay Rane\n\
--\n\
-- $HEADER$\n\
--\n\
\n\
-- Generated using hound\n\
-- version 2.0\n\
\n\
--\n\
-- Create tables if not exist\n\
--\n\
CREATE TABLE IF NOT EXISTS hound (\n\
    family INTEGER NOT NULL,\n\
    model  INTEGER NOT NULL,\n\
    name   VARCHAR NOT NULL,\n\
    value  REAL    NOT NULL\n\
);\n\n");

    printf("INSERT INTO hound (family, model, name, value) VALUES "
        "(%d, %d, 'CPI_threshold', 0.5);\n", family, model);

    if (processor == PROC_UNKNOWN) {
        discoverCachesWithoutCPUID();    // Also prints values
    } else {
        cacheCollection caches = {0};

        if (processor == PROC_INTEL)
            discoverIntelCaches(&caches);
        else if (processor == PROC_AMD)
            discoverAMDCaches(&caches);

        #ifdef DEBUG_PRINT
        printf ("DEBUG: Finished discovering Intel / AMD caches\n");
        #endif

        // Now that we have sizes and characteristics of all caches,
        // measure the latencies to each cache

        // First, L1
        if (caches.lpL1Caches == NULL) {
            #ifdef DEBUG_PRINT
            printf("DEBUG: No L1 caches found!\n");
            #endif

            printf("Could not find L1 cache, exiting\n");
            return 1;
        }

        double L1_dlat = 1;
        volatile cacheList* lpCache = caches.lpL1Caches;
        volatile double cycleCollection [COLLECTION_SIZE];
        while (lpCache != NULL) {
            // We cannot measure TLB and iCACHE latencies this way, so just
            // measure caches
            if ((lpCache->info.cacheOrTLB == CACHE) &&
                (lpCache->info.type != INSTRUCTION)) {
                #ifdef DEBUG_PRINT
                if (lpCache->info.cacheOrTLB == CACHE)
                    printf("DEBUG: Measuring latency of L1 %s cache, line size:"
                        " %d bytes, total size: %lf KB\n",
                        getCacheType(lpCache->info.type),
                        lpCache->info.lineSize, lpCache->info.lineSize *
                        lpCache->info.lineCount / 1024.0);
                #endif

                for (i=0; i<COLLECTION_SIZE; i++)
                    cycleCollection[i] = allocateAndTest(
                        lpCache->info.lineSize * lpCache->info.lineCount,
                        lpCache->info.lineSize, 1);

                L1_dlat = get90thPercentile(cycleCollection, COLLECTION_SIZE);
                printf("INSERT INTO hound (family, model, name, value) VALUES "
                    "(%d, %d, 'L1_dlat', %lf);\n", L1_dlat, family, model);
                break;
            }

            lpCache = lpCache->lpNext;
        }

        // FIXME: L1 Latency values are being calculated incorrectly
        // printf ("L1_ilat = %.2lf\n", getIntelL1InstructionLatency());

        // Temporary fix:
        printf("INSERT INTO hound (family, model, name, value) VALUES "
            "(%d, %d, 'L1_ilat', %lf);\n", L1_dlat, family, model);

        // Then L2
        lpCache = caches.lpL2Caches;
        while (lpCache != NULL) {
            // We cannot measure TLB and iCACHE latencies this way, so just
            // measure caches
            if ((lpCache->info.cacheOrTLB == CACHE) &&
                (lpCache->info.type != INSTRUCTION)) {
                #ifdef DEBUG_PRINT
                if (lpCache->info.cacheOrTLB == CACHE)
                    printf("DEBUG: Measuring latency of L2 %s cache, line size:"
                        " %d bytes, total size: %lf KB\n",
                        getCacheType(lpCache->info.type),
                        lpCache->info.lineSize, lpCache->info.lineSize *
                        lpCache->info.lineCount / 1024.0);
                #endif

                for (i=0; i<COLLECTION_SIZE; i++)
                    cycleCollection[i] = allocateAndTest(
                        lpCache->info.lineSize * lpCache->info.lineCount,
                        lpCache->info.lineSize, 1);

                if (lpCache->info.type == UNIFIED)
                    printf("INSERT INTO hound (family, model, name, value) "
                        "VALUES (%d, %d, 'L2_lat', %lf);\n",
                        get90thPercentile(cycleCollection, COLLECTION_SIZE),
                        family, model);
                else
                    printf("INSERT INTO hound (family, model, name, value) "
                        "VALUES (%d, %d, 'L2_dlat', %lf);\n",
                        get90thPercentile(cycleCollection, COLLECTION_SIZE),
                        family, model);
                break;
            }
            lpCache = lpCache->lpNext;
        }

        // L3...
        lpCache = caches.lpL3Caches;
        while (lpCache != NULL) {
            #ifdef DEBUG_PRINT
            if (lpCache->info.cacheOrTLB == CACHE)
                printf("DEBUG: Measuring latency of L3 %s cache, line size: %d "
                    "bytes, total size: %lf KB\n",
                    getCacheType(lpCache->info.type), lpCache->info.lineSize,
                    lpCache->info.lineSize * lpCache->info.lineCount / 1024.0);
            else
                printf("DEBUG: Was not expecting anything other than cache, "
                    "however found an item with code '%d' in the list, "
                    "ignoring...\n", lpCache->info.cacheOrTLB);
            #endif

            // There are no level 3 TLBs, so not making the extra check
            for (i=0; i<COLLECTION_SIZE; i++)
                cycleCollection[i] = allocateAndTest(
                    lpCache->info.lineSize * lpCache->info.lineCount,
                    lpCache->info.lineSize, 1);

            printf ("INSERT INTO hound (family, model, name, value) VALUES "
                "(%d, %d, 'L3_lat', %lf);\n",
                get90thPercentile(cycleCollection, COLLECTION_SIZE), family,
                model);

            // FIXME: Non-sensical to include a `break' here
            break;
            lpCache = lpCache->lpNext;
        }

        // Finally to the main memeory
        if (caches.lpL3Caches) {
            lpCache = caches.lpL3Caches;

            #ifdef DEBUG_PRINT
            printf("DEBUG: Using memory size as 30x L3 size = %lf KB\n",
                30 * lpCache->info.lineSize * lpCache->info.lineCount / 1024.0);
            #endif
        } else if (caches.lpL2Caches) {
            lpCache = caches.lpL2Caches;

            #ifdef DEBUG_PRINT
            printf("DEBUG: Using memory size as 30x L2 size = %lf KB\n",
                30 * lpCache->info.lineSize * lpCache->info.lineCount / 1024.0);
            #endif
        } else {
            lpCache = caches.lpL1Caches;

            #ifdef DEBUG_PRINT
            printf("DEBUG: Using memory size as 30x L1 size = %lf KB\n",
                30 * lpCache->info.lineSize * lpCache->info.lineCount / 1024.0);
            #endif
        }

        for (i=0; i<COLLECTION_SIZE; i++) {
            cycleCollection[i] = allocateAndTestMainMemory(
            30 * lpCache->info.lineSize * lpCache->info.lineCount, 1024, 0);
        }

        destroyCacheLists(&caches);
        printf ("INSERT INTO hound (family, model, name, value) VALUES "
            "(%d, %d, 'mem_lat', %lf);\n",
            get90thPercentile(cycleCollection, COLLECTION_SIZE), family, model);
    }

    printf("INSERT INTO hound (family, model, name, value) VALUES "
        "(%d, %d, 'CPU_freq', %.0lf);\n", family, model, frequency * 1000000);

    double FP_lat, FP_slow_lat, BR_lat, BR_miss_lat, TLB_lat;

    FP_lat = getFPLatency();
    printf("INSERT INTO hound (family, model, name, value) VALUES "
        "(%d, %d, 'FP_lat', %lf);\n", family, model,
        FP_lat < 1.0 ? 1.0 : FP_lat);

    FP_slow_lat = getFPSlowLatency();
    printf("INSERT INTO hound (family, model, name, value) VALUES "
        "(%d, %d, 'FP_slow_lat', %lf);\n", family, model,
        FP_slow_lat < 1.0 ? 1.0 : FP_slow_lat);

    BR_lat = getPredictedBranchLatency();
    printf("INSERT INTO hound (family, model, name, value) VALUES "
        "(%d, %d, 'BR_lat', %lf);\n", family, model,
        BR_lat < 1.0 ? 1.0 : BR_lat);

    BR_miss_lat = getMisPredictedBranchLatency();
    printf("INSERT INTO hound (family, model, name, value) VALUES "
        "(%d, %d, 'BR_miss_lat', %lf);\n", family, model,
        BR_miss_lat < 1.0 ? 1.0 : BR_miss_lat);

    // Warm up TLBs?
    for (i = 0; i < 32; i++) {
        allocateAndTest(4096, 512, 1);
    }

    TLB_lat = getTLBLatency(512 * 1024);
    printf("INSERT INTO hound (family, model, name, value) VALUES "
        "(%d, %d, 'TLB_lat', %lf);\n\n", family, model,
        TLB_lat < 1.0 ? 1.0 : TLB_lat);

    return 0;
}
