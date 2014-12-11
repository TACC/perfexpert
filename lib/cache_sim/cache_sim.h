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

#ifndef CACHE_SIM_H_
#define CACHE_SIM_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _STDLIB_H
#include <stdlib.h>
#endif

#ifndef _STDINT_H
#include <stdint.h>
#endif

/* Return codes */
#define CACHE_SIM_SUCCESS  0x000
#define CACHE_SIM_ERROR    0x001

#define CACHE_SIM_L1_HIT            0x010
#define CACHE_SIM_L1_HIT_PREFETCH   0x020
#define CACHE_SIM_L1_MISS           0x040
#define CACHE_SIM_L1_MISS_CONFLICT  0x080
#define CACHE_SIM_L1_PREFETCH_EVICT 0x100

/* Some limitations */
#define CACHE_SIM_SYMBOL_MAX_LENGTH 40

/* Type declaration: the cache itself */
typedef struct cache_handle cache_handle_t;

/* Functions declaration */
cache_handle_t* cache_sim_init(const unsigned int total_size,
    const unsigned int line_size, const unsigned int associativity,
    const char *policy);
int cache_sim_fini(cache_handle_t *cache);
int cache_sim_access(cache_handle_t *cache, const uint64_t address);

static cache_handle_t* cache_create(const unsigned int total_size,
    const unsigned int line_size, const unsigned int associativity);
static void cache_destroy(cache_handle_t *cache);
static int set_policy(cache_handle_t *cache, const char *policy);

#ifdef __cplusplus
}
#endif

#endif /* CACHE_SIM_H_ */
