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

#include <gmp.h>

void factorial(mpf_t mi_result, unsigned long n, unsigned long stop);

void nCk(mpf_t result, unsigned long n, unsigned long k);

double pHit(unsigned long i, unsigned long assoc, unsigned long distance,
        double pHitSet);

void setDouble(mpf_t* n, double num);

void accumulateStride(mpf_t* current, double normalized);

double getDouble(mpf_t* n);

double hit_probability(long distance, long size, short associativity,
    int linesize);
