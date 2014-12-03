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
#define CACHE_SIM_SUCCESS 0
#define CACHE_SIM_ERROR   1

#define CACHE_SIM_L1_HIT  32
#define CACHE_SIM_L1_MISS 64

/* Macro to extract the offset, set, and tag of an address */
#ifndef CACHE_SIM_OFFSET
#define CACHE_SIM_OFFSET(a) \
    (a <<= ((sizeof(uint64_t)*8) - cache->offset_length)); \
    (a >>= ((sizeof(uint64_t)*8) - cache->offset_length));
#endif

#ifndef CACHE_SIM_SET
#define CACHE_SIM_SET(a) \
    (a <<= ((sizeof(uint64_t)*8) - cache->set_length - cache->offset_length)); \
    (a >>= ((sizeof(uint64_t)*8) - cache->set_length));
#endif

#ifndef CACHE_SIM_TAG
#define CACHE_SIM_TAG(a) (a >>= (cache->set_length + cache->offset_length));
#endif

/* Cache type */
typedef struct cache_handle cache_handle_t;

/* Policy function type declarations */
typedef int (*policy_init_fn_t)(cache_handle_t *);
typedef int (*policy_access_fn_t)(cache_handle_t *, uint64_t, uint64_t);

/* Cache structure */
struct cache_handle {
    /* provided information */
    int total_size;
    int line_size;
    int associativity;
    /* deducted information */
    int total_lines;
    int total_sets;
    int offset_length;
    int set_length;
    /* replacement policy (or algorithm) */
    policy_access_fn_t access_fn;
    /* data section (replacement algorithm dependent) */
    void *data;
    /* performance counters */
    uint64_t hit;
    uint64_t miss;
    uint64_t access;
};

/* Policy structure */
typedef struct {
    const char *name;
    policy_init_fn_t init_fn;
    policy_access_fn_t access_fn;
} policy_t;

/* Function declaration */
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
