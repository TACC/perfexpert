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

#ifndef CACHE_POLICY_PLRU_H_
#define CACHE_POLICY_PLRU_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CACHE_SIM_H_
#include "cache_sim.h"
#endif

#ifndef _STDINT_H
#include <stdint.h>
#endif

/* Bit mapipulation macros, read as: ACTION(variable, bit_field) */
#define SET_BIT   (a, b) a |=  (1 << b)
#define CLEAR_BIT (a, b) a &= ~(1 << b)
#define TOGGLE_BIT(a, b) a ^=  (1 << b)
#define BIT_IS_SET(a, b) a  &  (1 << b)

/* Function declaration */
int policy_plru_init(cache_handle_t *cache);
int policy_plru_access(cache_handle_t *cache, const uint64_t line_id,
    const load_t load);

typedef struct {
    uint64_t line_id;
    uint8_t  load;
    uint8_t  padding[7];
} policy_plru_t;

#ifdef __cplusplus
}
#endif

#endif /* CACHE_POLICY_PLRU_H_ */
