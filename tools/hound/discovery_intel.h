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

#ifndef DISCOVERY_INTEL_H_
#define DISCOVERY_INTEL_H_

#define CACHE_TABLE_SIZE 86
const intelCacheTableEntry intelCacheTable[CACHE_TABLE_SIZE] = {
    // cacheOrTLB, type, level, lineSize, lineCount, wayness
    0x01, { TLB, UNIFIED, DONT_CARE, PAGESIZE_4K, 32, 4 },
    0x02, { TLB, UNIFIED, DONT_CARE, PAGESIZE_4M, 2, FULLY_ASSOCIATIVE },
    0x03, { TLB, DATA, DONT_CARE, PAGESIZE_4K, 64, 4 },
    0x04, { TLB, DATA, DONT_CARE, PAGESIZE_4M, 8, 4 },
    0x05, { TLB, DATA, DONT_CARE, PAGESIZE_4M, 32, 4 },
    0x06, { CACHE, INSTRUCTION, 1, 32, 256, 4 },
    0x08, { CACHE, INSTRUCTION, 1, 64, 512, 4 },
    0x09, { CACHE, INSTRUCTION, 1, 64, 512, 4 },
    0x0a, { CACHE, DATA, 1, 32, 256, 2 },
    0x0c, { CACHE, DATA, 1, 32, 512, 4 },
    0x0d, { CACHE, DATA, 1, 64, 256, 4 },
    0x21, { CACHE, UNIFIED, 2, 64, 4096, 8 },
    0x22, { CACHE, UNIFIED, 3, 64, 8192, 4 },
    0x23, { CACHE, UNIFIED, 3, 64, 16384, 8 },
    0x25, { CACHE, UNIFIED, 3, 64, 32768, 8 },
    0x29, { CACHE, UNIFIED, 3, 64, 65536, 8 },
    0x2C, { CACHE, DATA, 1, 64, 512, 8 },
    0x30, { CACHE, INSTRUCTION, 1, 64, 512, 8 },
    0x39, { CACHE, UNIFIED, 2, 64, 2048, 4 },
    0x3A, { CACHE, UNIFIED, 2, 64, 3072, 6 },
    0x3B, { CACHE, UNIFIED, 2, 64, 2048, 2 },
    0x3C, { CACHE, UNIFIED, 2, 64, 4096, 4 },
    0x3D, { CACHE, UNIFIED, 2, 64, 6144, 6 },
    0x3E, { CACHE, UNIFIED, 2, 64, 8192, 4 },
    0x41, { CACHE, UNIFIED, 2, 32, 4096, 4 },
    0x42, { CACHE, UNIFIED, 2, 32, 8192, 4 },
    0x43, { CACHE, UNIFIED, 2, 32, 16384, 4 },
    0x44, { CACHE, UNIFIED, 2, 32, 32768, 4 },
    0x45, { CACHE, UNIFIED, 2, 32, 65536, 4 },
    0x46, { CACHE, UNIFIED, 3, 64, 65536, 4 },
    0x47, { CACHE, UNIFIED, 3, 64, 131072, 8 },
    0x48, { CACHE, UNIFIED, 2, 64, 49152, 12 },
    0x49, { CACHE, UNIFIED, 2, 64, 65536, 16 },
    0x4A, { CACHE, UNIFIED, 3, 64, 98304, 12 },
    0x4B, { CACHE, UNIFIED, 3, 64, 131072, 16 },
    0x4C, { CACHE, UNIFIED, 3, 64, 196608, 12 },
    0x4D, { CACHE, UNIFIED, 3, 64, 262144, 16 },
    0x4E, { CACHE, UNIFIED, 2, 64, 98304, 24 },
    0x50, { TLB, INSTRUCTION, DONT_CARE, PAGESIZE_4K|PAGESIZE_2M|PAGESIZE_4M, 64, FULLY_ASSOCIATIVE },
    0x51, { TLB, INSTRUCTION, DONT_CARE, PAGESIZE_4K|PAGESIZE_2M|PAGESIZE_4M, 128, FULLY_ASSOCIATIVE },
    0x52, { TLB, INSTRUCTION, DONT_CARE, PAGESIZE_4K|PAGESIZE_2M|PAGESIZE_4M, 256, FULLY_ASSOCIATIVE },
    0x55, { TLB, INSTRUCTION, DONT_CARE, PAGESIZE_2M|PAGESIZE_4M, 7, FULLY_ASSOCIATIVE },
    0x56, { TLB, DATA, 1, PAGESIZE_4M, 16, 4 },
    0x57, { TLB, DATA, 1, PAGESIZE_4K, 16, 4 },
    0x5A, { TLB, DATA, DONT_CARE, PAGESIZE_2M|PAGESIZE_4M, 32, 4 },
    0x5B, { TLB, DATA, DONT_CARE, PAGESIZE_4K|PAGESIZE_4M, 64, FULLY_ASSOCIATIVE },
    0x5C, { TLB, DATA, DONT_CARE, PAGESIZE_4K|PAGESIZE_4M, 128, FULLY_ASSOCIATIVE },
    0x5D, { TLB, DATA, DONT_CARE, PAGESIZE_4K|PAGESIZE_4M, 256, FULLY_ASSOCIATIVE },
    0x60, { CACHE, DATA, 1, 64, 256, 8 },
    0x66, { CACHE, DATA, 1, 64, 128, 4 },
    0x67, { CACHE, DATA, 1, 64, 256, 4 },
    0x68, { CACHE, DATA, 1, 64, 512, 4 },
    0x78, { CACHE, UNIFIED, 2, 64, 16384, 4 },
    0x79, { CACHE, UNIFIED, 2, 64, 2048, 8 },
    0x7A, { CACHE, UNIFIED, 2, 64, 4096, 8 },
    0x7B, { CACHE, UNIFIED, 2, 64, 8192, 8 },
    0x7C, { CACHE, UNIFIED, 2, 64, 16384, 8 },
    0x7D, { CACHE, UNIFIED, 2, 64, 32768, 8 },
    0x7F, { CACHE, UNIFIED, 2, 64, 8192, 2 },
    0x82, { CACHE, UNIFIED, 2, 32, 8192, 8 },
    0x83, { CACHE, UNIFIED, 2, 32, 16384, 8 },
    0x84, { CACHE, UNIFIED, 2, 32, 32768, 8 },
    0x85, { CACHE, UNIFIED, 2, 32, 65536, 8 },
    0x86, { CACHE, UNIFIED, 2, 64, 8192, 4 },
    0x87, { CACHE, UNIFIED, 2, 64, 16384, 8 },
    0xB0, { TLB, INSTRUCTION, DONT_CARE, PAGESIZE_4K, 128, 4 },
    0xB1, { TLB, INSTRUCTION, DONT_CARE, PAGESIZE_2M, 8, 4 },
    0xB2, { TLB, INSTRUCTION, DONT_CARE, PAGESIZE_4K, 64, 4 },
    0xB3, { TLB, DATA, DONT_CARE, PAGESIZE_4K, 128, 4 },
    0xB4, { TLB, DATA, DONT_CARE, PAGESIZE_4K, 256, 4 },
    0xCA, { TLB, UNIFIED, 2, PAGESIZE_4K, 512, 4 },
    0xD0, { CACHE, UNIFIED, 3, 64, 8192, 4 },
    0xD1, { CACHE, UNIFIED, 3, 64, 16384, 4 },
    0xD2, { CACHE, UNIFIED, 3, 64, 32768, 4 },
    0xD6, { CACHE, UNIFIED, 3, 64, 16384, 8 },
    0xD7, { CACHE, UNIFIED, 3, 64, 32768, 8 },
    0xD8, { CACHE, UNIFIED, 3, 64, 65536, 8 },
    0xDC, { CACHE, UNIFIED, 3, 64, 24576, 12 },
    0xDD, { CACHE, UNIFIED, 3, 64, 49152, 12 },
    0xDE, { CACHE, UNIFIED, 3, 64, 98304, 12 },
    0xE2, { CACHE, UNIFIED, 3, 64, 32768, 16 },
    0xE3, { CACHE, UNIFIED, 3, 64, 65536, 16 },
    0xE4, { CACHE, UNIFIED, 3, 64, 131072, 16 },
    0xEA, { CACHE, UNIFIED, 3, 64, 196608, 24 },
    0xEB, { CACHE, UNIFIED, 3, 64, 294912, 24 },
    0xEC, { CACHE, UNIFIED, 3, 64, 393216, 24 }
};

int insertIntoCorrectCacheList(cacheCollection* lpCaches, cacheInfo cache) {
    cacheList** lplpCacheList = NULL;
    if (1 == cache.level || DONT_CARE == cache.level) {
        // Insert into L1
        lplpCacheList = &lpCaches->lpL1Caches;
    } else if (2 == cache.level) {
        // Insert into L2
        lplpCacheList = &lpCaches->lpL2Caches;
    } else if (3 == cache.level) {
        // Insert into L3
        lplpCacheList = &lpCaches->lpL3Caches;
    } else {
        // Ignore;
#ifdef DEBUG_PRINT
        printf ("DEBUG: Found cache/TLB at unknown level: %d, ignoring...\n",
                cache.level);
#endif /* DEBUG_PRINT */
        
        return;
    }
    
#ifdef DEBUG_PRINT
 if (CACHE == cache.cacheOrTLB) {
 printf ("DEBUG: Found %s cache at level %d of size: %.2lf KB\n",
 getCacheType(cache.type), cache.level,
 cache.lineSize * cache.lineCount / 1024.0);
 } else if (TLB == cache.cacheOrTLB) {
 printf ("DEBUG: Found %s TLB at level %d with %d lines\n",
 getCacheType(cache.type), cache.level, cache.lineCount);
 }
#endif /* DEBUG_PRINT */

 return insertIntoCacheList(lplpCacheList, cache);
}

int mapIntelCache(cacheCollection* lpCaches, short code) {
#ifdef DEBUG_PRINT
    printf ("DEBUG: Mapping code 0x%X to cache...\n", code);
#endif /* DEBUG_PRINT */
    
    if (0xFF == code) {
        int i = 0, info[4];
        
#ifdef DEBUG_PRINT
        printf ("DEBUG: Invoking CPUID with leaf 4 for discovering deterministic cache parameters\n");
#endif /* DEBUG_PRINT */
        
        while(1) {
            __cpuid(info, 0x04, i);
            i++;
            
            if (0 == (info[0] & 0x0f)) {
                break;
            }
            int ways = ((info[1] & 0xffc00000) >> 22) + 1;
            int partitions = ((info[1] & 0x003ff000) >> 12) + 1;
            int sets = (info[2] & 0xffffffff) + 1;
            
            cacheInfo newCache;
            newCache.cacheOrTLB = CACHE;
            
            if (1 == (info[0] & 0x0f)) {
                newCache.type = DATA;
            } else if (2 == (info[0] & 0x0f)) {
                newCache.type = INSTRUCTION;
            } else if (3 == (info[0] & 0x0f)) {
                newCache.type = UNIFIED;
            } else {
                newCache.type = TYPE_UNKNOWN;
            }
            newCache.level = (info[0] & 0x70) >> 5;
            newCache.lineSize = (info[1] & 0x0fff) + 1;
            newCache.lineCount = ways*partitions*sets;
            newCache.wayness = ways;
            
            insertIntoCorrectCacheList(lpCaches, newCache);
        }
        return 0;
    }
    
    int i;
    // Search through the table
    for (i = 0; i < CACHE_TABLE_SIZE; i++) {
        if (intelCacheTable[i].code == code) {
            break;
        }
    }
    
    if (CACHE_TABLE_SIZE == i) {
#ifdef DEBUG_PRINT
        printf ("DEBUG: Could not find an entry for code: 0x%X, ignoring...\n",
                code);
#endif /* DEBUG_PRINT */
        
        return; // Did not find the entry
    }
    insertIntoCorrectCacheList(lpCaches, intelCacheTable[i].info);
    
    return 0;
}

/* Loop over all caches */
int discoverIntelCaches(cacheCollection* lpCaches) {
    int i, info[4];
    
    __cpuid(info, 0x2, 0);
    if ((info[EAX] & 0xff) == 1) {
        for (i = 1; i < 4; i++) {
            // If MSB is set, this value is one among reserved
            if (0 == (info[i] & 0x80000000)) {
                if (info[i] & 0xff) {
                    mapIntelCache(lpCaches, info[i] & 0xff);
                }
                if (info[i] & 0xff00) {
                    mapIntelCache(lpCaches, (info[i] & 0xff00) >> 8);
                }
                if (info[i] & 0xff0000) {
                    mapIntelCache(lpCaches, (info[i] & 0xff0000) >> 16);
                }
                if (info[i] & 0xff000000) {
                    mapIntelCache(lpCaches, (info[i] & 0xff000000) >> 24);
                }
            }
        }
        return 0;
    }
    printf ("Multiple invocations (%d) of CPUID:0x02 required, cannot proceed\n",
            info[EAX] & 0xff);
    
    return -ERR_MULT_INVOCATIONS;
}

#endif /* DISCOVERY_INTEL_H_ */
