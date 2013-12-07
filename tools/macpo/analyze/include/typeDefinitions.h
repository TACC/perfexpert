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

#ifndef TYPEDEFINITIONS_H_
#define TYPEDEFINITIONS_H_

#include <list>

#define DONT_CARE            0

enum {
    INSTRUCTION = 0,
    DATA,
    UNIFIED,
    TYPE_UNKNOWN
};

enum {
    CACHE = 0,
    TLB
};

enum {
    UNKNOWN = -1,
    FULLY_ASSOCIATIVE = 0,
    DIRECT_MAPPED = 1
};

enum {
    PAGESIZE_4K = 1,
    PAGESIZE_2M = 2,
    PAGESIZE_4M = 4
};

typedef struct {
    unsigned char cacheOrTLB:1;
    unsigned char type:2;
    short level:4;
    int lineSize; // Also used as bitmap for page size for TLBs
    int lineCount;
    short wayness;
} cache_info_t;

typedef struct {
    std::list<cache_info_t> l1_cache_info;
    std::list<cache_info_t> l2_cache_info;
    std::list<cache_info_t> l3_cache_info;
} cache_lists_t;

typedef struct {
    short     code;
    cache_info_t info;
} intelCacheTableEntry;

const char* getCacheType(unsigned char type) {
    if (DATA == type) {
        return "data";
    }
    if (INSTRUCTION == type) {
        return "instruction";
    }
    if (UNIFIED == type) {
        return "unified";
    }
}

#endif /* TYPEDEFINITIONS_H_ */
