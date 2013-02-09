/*
 * Copyright (C) 2013 The University of Texas at Austin
 *
 * This file is part of PerfExpert.
 *
 * PerfExpert is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * PerfExpert is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with PerfExpert. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Ashay Rane
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

#define    COLLECTION_SIZE    50

char processor = PROC_UNKNOWN;

double get_90th_percentile(volatile double* lpCycles, unsigned int count) {
    int i, j;
    double temp, value;

    // Sort elements
    for (i = 0; i < count - 1; i++) {
        for (j = i + 1; j < count; j++) {
            if (lpCycles[i] > lpCycles[j]) {
                temp        = lpCycles[i];
                lpCycles[i] = lpCycles[j];
                lpCycles[j] = temp;
            }
        }
    }

    // Return the 90th percentile from the sorted list
    value = lpCycles[(int) (0.9 * count)];
    if (1.0 > value) {
        return 1.0;
    }
    return value;
}

int cache_list_insert(cacheList** lplpCacheList, cacheInfo cache) {
    while (NULL != *lplpCacheList) {
        lplpCacheList = &((*lplpCacheList)->lpNext);
    }

    if (NULL == ((*lplpCacheList) = (cacheList*) malloc(sizeof(cacheList)))) {
        return -ERR_MEMFAILED;
    }
    
    (*lplpCacheList)->info = cache;
    (*lplpCacheList)->lpNext = NULL;
    return 0;
}

void cache_list_destroy(cacheCollection* lpCaches) {
    cacheList* lpTemp;

    while (NULL != (lpTemp = lpCaches->lpL1Caches)) {
        lpCaches->lpL1Caches = lpTemp->lpNext;
        free(lpTemp);
    }

    while (NULL != (lpTemp = lpCaches->lpL2Caches)) {
        lpCaches->lpL2Caches = lpTemp->lpNext;
        free(lpTemp);
    }

    while (NULL != (lpTemp = lpCaches->lpL2Caches)) {
        lpCaches->lpL2Caches = lpTemp->lpNext;
        free(lpTemp);
    }
}

void discoverCachesWithoutCPUID() {
    int i, j;
    unsigned int cycles;
    unsigned short counter = 1;
    double cycleCollection[COLLECTION_SIZE], val;

#ifdef DEBUG_PRINT
    printf("DEBUG: Discovering caches without using CPUID\n");
#endif /* DEBUG_PRINT */

    // First measure with 1KB
    for (j = 0; j < COLLECTION_SIZE; j++) {
        cycleCollection[j] = allocateAndTest(1024, /* lpCache->info.lineSize */ 512, 1);
    }

    val = get_90th_percentile(cycleCollection, COLLECTION_SIZE);
    printf("L1_dlat = %.2lf\n", val);

    // FIXME: L1 Latency values are being calculated incorrectly
    printf("L1_ilat = %.2lf\n", val);

    // Start measuring from L2
    cycles = val;

    for (i = 32768; i < 4 * 1024 * 1024; i += 32768) {
        for (j = 0; j < COLLECTION_SIZE; j++) {
            cycleCollection[j] = allocateAndTest(i, 512, 1);
        }
        val = get_90th_percentile(cycleCollection, COLLECTION_SIZE);

        if (val > 5*cycles) {
#ifdef DEBUG_PRINT
            printf("DEBUG: Found 5x increase in 90th percentile at %d KB\n",
                    i / 1024);
#endif /* DEBUG_PRINT */
            if (1 == counter) {
                printf("L%d_dlat = %.2lf\n", ++counter, val);
            } else { // Assuming unified caches beyond L1
                printf("L%d_lat = %.2lf\n", ++counter, val);
            }
            cycles = val;

            if (3 == counter) {
#ifdef DEBUG_PRINT
                printf("DEBUG: Discovered L2, stopping probing caches\n");
#endif /* DEBUG_PRINT */
                break;
            }
        }
    }
#ifdef DEBUG_PRINT
    printf("DEBUG: Finished probing caches, now probing memory with size: %.2lf KB\n",
            30 * i / 1024.0);
#endif /* DEBUG_PRINT */
    // Start with mem_size = 30i KB
    for (j = 0; j < COLLECTION_SIZE; j++) {
        cycleCollection[j] = allocateAndTestMainMemory(30 * i, 1024, 0);
    }
#ifdef DEBUG_PRINT
    printf("DEBUG: Finished probing memory\n");
#endif /* DEBUG_PRINT */
    printf("mem_lat = %.2lf\n", get_90th_percentile(cycleCollection,
                                                     COLLECTION_SIZE));
}

int isCacheProbeSupportedByCPUID(unsigned char processor) {
    int info[4];

    if (PROC_INTEL == processor) {
        __cpuid(info, 0, 0);
        if (info[0] >= 0x2) {
            return 0;
        }
    } else if (PROC_AMD == processor) {
        __cpuid(info, 0x80000000, 0);
        if (info[0] >= 0x80000006) {
            return 0;
        }
    }
    return -1;
}

int main(int argc, char * argv[]) {
    FILE* fp;
    int i, j;
    char line [256] = { 0 };
    double frequency = 0;
    double cycleCollection [COLLECTION_SIZE];
    double FP_lat, FP_slow_lat, BR_lat, BR_miss_lat, TLB_lat;

#ifdef DEBUG_PRINT
    printf("DEBUG: Debug messages ON\n");
#endif /* DEBUG_PRINT */
    
    if (isCPUIDSupported()) {
        char processorName[13];
        
#ifdef DEBUG_PRINT
        printf("DEBUG: CPUID supported, getting processor name...\n");
#endif /* DEBUG_PRINT */

        getProcessorName(processorName);
        if (0 == strcmp(processorName, "GenuineIntel")) {
            processor = PROC_INTEL;
        } else if (0 == strcmp(processorName, "AuthenticAMD")) {
            processor = PROC_AMD;
        }
#ifdef DEBUG_PRINT
        printf("DEBUG: Processor name was: %s\n", processorName);
#endif /* DEBUG_PRINT */

        if (0 > isCacheProbeSupportedByCPUID(processor)) {
#ifdef DEBUG_PRINT
            printf("DEBUG: Processor supports CPUID instruction but only limited information can be retrived. Falling back to probing using different memory sizes\n");
#endif /* DEBUG_PRINT */

            // CPUID of no use, instead measure using different memory sizes and latency of access
            processor = PROC_UNKNOWN;
        } else {
#ifdef DEBUG_PRINT
            printf("DEBUG: Cache probing using CPUID instruction supported\n");
#endif /* DEBUG_PRINT */
        }
#ifdef DEBUG_PRINT
    } else {
        printf("DEBUG: CPUID instruction not supported\n");
#endif /* DEBUG_PRINT */
    }

    // Version string
    printf("version = 1.0\n\n");
    printf("# Generated using hound\n");
    printf("CPI_threshold = 0.5\n");

    if (processor == PROC_UNKNOWN) {
        discoverCachesWithoutCPUID();    // Also prints values
    } else {
        double L1_dlat = 1;
        volatile cacheList* lpCache;
        volatile double cycleCollection [COLLECTION_SIZE];

        cacheCollection caches = { 0 };

        if (PROC_INTEL == processor) {
            discoverIntelCaches(&caches);
        } else if (PROC_AMD == processor) {
            discoverAMDCaches(&caches);
        }

#ifdef DEBUG_PRINT
        printf("DEBUG: Finished discovering Intel / AMD caches\n");
#endif /* DEBUG_PRINT */

        // Now that we have sizes and characteristics of all caches,
        // measure the latencies to each cache

        // First, L1
        if (NULL == caches.lpL1Caches) {
#ifdef DEBUG_PRINT
            printf("DEBUG: No L1 caches found!\n");
#endif /* DEBUG_PRINT */
            printf("DEBUG: Could not find L1 cache, exiting\n");
            return 1;
        }

        lpCache = caches.lpL1Caches;
        while (NULL != lpCache) {
            if ((CACHE == lpCache->info.cacheOrTLB) &&
                (INSTRUCTION != lpCache->info.type)) {
                // We cannot measure TLB and iCACHE latencies this way,
                // so just measure caches
#ifdef DEBUG_PRINT
                if (CACHE == lpCache->info.cacheOrTLB) {
                    printf("DEBUG: Measuring latency of L1 %s cache, line size: %d bytes, total size: %.1lf KB\n",
                           getCacheType(lpCache->info.type),
                           lpCache->info.lineSize,
                           lpCache->info.lineSize * lpCache->info.lineCount / 1024.0);
                }
#endif /* DEBUG_PRINT */

                for (i = 0; i < COLLECTION_SIZE; i++)
                    cycleCollection[i] = allocateAndTest(lpCache->info.lineSize * lpCache->info.lineCount,
                                                         lpCache->info.lineSize,
                                                         1);

                L1_dlat = get_90th_percentile(cycleCollection, COLLECTION_SIZE);
                printf("L1_dlat = %.2lf\n", L1_dlat);
                break;
            }
            lpCache = lpCache->lpNext;
        }

        // FIXME: L1 Latency values are being calculated incorrectly
        // printf("L1_ilat = %.2lf\n", getIntelL1InstructionLatency());
        // Temporary fix:
        printf("L1_ilat = %.2lf\n", L1_dlat);

        // Then L2
        lpCache = caches.lpL2Caches;
        while (NULL != lpCache) {
            if ((CACHE == lpCache->info.cacheOrTLB) &&
                (INSTRUCTION != lpCache->info.type)) {
                // We cannot measure TLB and iCACHE latencies this way, so just measure caches
#ifdef DEBUG_PRINT
                if (CACHE == lpCache->info.cacheOrTLB) {
                    printf("DEBUG: Measuring latency of L2 %s cache, line size: %d bytes, total size: %.1lf KB\n",
                           getCacheType(lpCache->info.type),
                           lpCache->info.lineSize,
                           lpCache->info.lineSize * lpCache->info.lineCount / 1024.0);
                }
#endif /* DEBUG_PRINT */
                for (i = 0; i < COLLECTION_SIZE; i++) {
                    cycleCollection[i] = allocateAndTest(lpCache->info.lineSize * lpCache->info.lineCount,
                                                         lpCache->info.lineSize,
                                                         1);
                }
                if (UNIFIED == lpCache->info.type) {
                    printf("L2_lat = %.2lf\n",
                           get_90th_percentile(cycleCollection,
                                               COLLECTION_SIZE));
                } else {
                    printf("L2_dlat = %.2lf\n",
                           get_90th_percentile(cycleCollection,
                                               COLLECTION_SIZE));
                }
                break;
            }
            lpCache = lpCache->lpNext;
        }

        // L3...
        lpCache = caches.lpL3Caches;
        while (NULL != lpCache) {
#ifdef DEBUG_PRINT
            if (CACHE == lpCache->info.cacheOrTLB) {
                printf("DEBUG: Measuring latency of L3 %s cache, line size: %d bytes, total size: %.1lf KB\n",
                       getCacheType(lpCache->info.type), lpCache->info.lineSize,
                       lpCache->info.lineSize * lpCache->info.lineCount / 1024.0);
            } else {
                printf("DEBUG: Was not expecting anything other than cache, however found an item with code '%d' in the list, ignoring...\n",
                       lpCache->info.cacheOrTLB);
            }
#endif /* DEBUG_PRINT */
            // There are no level 3 TLBs, so not making the extra check
            for (i = 0; i < COLLECTION_SIZE; i++) {
                cycleCollection[i] = allocateAndTest(lpCache->info.lineSize * lpCache->info.lineCount,
                                                     lpCache->info.lineSize, 1);
            }
            printf("L3_lat = %.2lf\n", get_90th_percentile(cycleCollection,
                                                           COLLECTION_SIZE));

            // FIXME: Non-sensical to include a `break' here
            break;
            lpCache = lpCache->lpNext;
        }

        // Finally to the main memeory
        if (caches.lpL3Caches) {
            lpCache = caches.lpL3Caches;
#ifdef DEBUG_PRINT
            printf("DEBUG: Using memory size as 30x L3 size = %.2lf KB\n",
                   30 * lpCache->info.lineSize * lpCache->info.lineCount / 1024.0);
#endif /* DEBUG_PRINT */
        } else if (caches.lpL2Caches) {
            lpCache = caches.lpL2Caches;
#ifdef    DEBUG_PRINT
            printf("DEBUG: Using memory size as 30x L2 size = %.2lf KB\n",
                   30 * lpCache->info.lineSize * lpCache->info.lineCount / 1024.0);
#endif /* DEBUG_PRINT */
        } else {
            lpCache = caches.lpL1Caches;
#ifdef    DEBUG_PRINT
            printf("DEBUG: Using memory size as 30x L1 size = %.2lf KB\n",
                   30 * lpCache->info.lineSize * lpCache->info.lineCount / 1024.0);
#endif /* DEBUG_PRINT */
        }
        for (i = 0; i < COLLECTION_SIZE; i++)
            cycleCollection[i] = allocateAndTestMainMemory(30 * lpCache->info.lineSize * lpCache->info.lineCount,
                                                           /* lpCache->info.lineSize */
                                                           1024, 0);

        cache_list_destroy(&caches);
        printf("mem_lat = %.2lf\n", get_90th_percentile(cycleCollection,
                                                        COLLECTION_SIZE));
    }

    // Lookup operating frequency in /proc/cpuinfo
    fp = fopen("/proc/cpuinfo", "r");
    if (NULL == fp) {
        perror ("Error opening /proc/cpuinfo for reading processor frequency");
        return 1;
    }
    while (fgets(line, 256, fp)) {
        if (0 == strncmp ("cpu MHz", line, 7)) {
            break;
        }
    }
    fclose(fp);
    if ('\0' == line[0]) {
        printf("Did not find cpu MHz string in /proc/cpuinfo, is it a valid file?\n");
        return 1;
    }

    sscanf(line, "cpu MHz :%lf", &frequency);
    printf("CPU_freq = %.0lf\n", frequency*1000000);

    FP_lat = getFPLatency();
    printf("FP_lat = %.2lf\n", FP_lat < 1.0 ? 1.0 : FP_lat);

    FP_slow_lat = getFPSlowLatency();
    printf("FP_slow_lat = %.2lf\n", FP_slow_lat < 1.0 ? 1.0 : FP_slow_lat);

    BR_lat = getPredictedBranchLatency();
    printf("BR_lat = %.2lf\n", BR_lat < 1.0 ? 1.0 : BR_lat);

    BR_miss_lat = getMisPredictedBranchLatency();
    printf("BR_miss_lat = %.2lf\n", BR_miss_lat < 1.0 ? 1.0 : BR_miss_lat);

    // Warm up TLBs?
    for (i = 0; i < 32; i++) {
        allocateAndTest(4096, /* lpCache->info.lineSize */ 512, 1);
    }
    TLB_lat = getTLBLatency(512*1024);
    printf("TLB_lat = %.2lf\n", TLB_lat < 1.0 ? 1.0 : TLB_lat);

    return 0;
}
