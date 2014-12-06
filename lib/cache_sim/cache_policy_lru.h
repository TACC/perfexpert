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

#ifndef CACHE_POLICY_LRU_H_
#define CACHE_POLICY_LRU_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CACHE_SIM_H_
#include "cache_sim.h"
#endif

#ifndef _STDINT_H
#include <stdint.h>
#endif

int policy_lru_init(cache_handle_t *cache);
int policy_lru_access(cache_handle_t *cache, uint64_t line_id);

typedef struct {
    uint64_t line_id;
    uint64_t age;
} policy_lru_t;

#ifdef __cplusplus
}
#endif

#endif /* CACHE_POLICY_LRU_H_ */
