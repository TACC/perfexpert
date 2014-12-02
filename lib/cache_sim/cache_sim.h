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

/* Macros */

/* Cache type */
typedef struct cache_handle cache_handle_t;

/* Policy function type declarations */
typedef int (*policy_fn_t)(cache_handle_t *, uint64_t);
typedef int (*policy_init_fn_t)(cache_handle_t *);

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
    policy_fn_t put_fn;
    policy_fn_t get_fn;
    /* data section */
    void *data;
    /* performance counters */
};

/* Policy structure */
typedef struct {
    const char *name;
    policy_init_fn_t init_fn;
    policy_fn_t put_fn;
    policy_fn_t get_fn;
} policy_t;

/* Function declaration */
cache_handle_t* cache_sim_init(const unsigned int total_size,
    const unsigned int line_size, const unsigned int associativity,
    const char *policy);

static cache_handle_t* cache_create(const unsigned int total_size,
    const unsigned int line_size, const unsigned int associativity);
static void cache_destroy(cache_handle_t *cache);
static int set_policy(cache_handle_t *cache, const char *policy);

#ifdef __cplusplus
}
#endif

#endif /* CACHE_SIM_H_ */
