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

#include "associative_cache.h"

void factorial(mpf_t mi_result, unsigned long n, unsigned long stop) {
    unsigned long int n1 = n;
    mpf_set_ui(mi_result, 1);
    while(n1 > stop)
        mpf_mul_ui(mi_result, mi_result, n1--);
}

void nCk(mpf_t result, unsigned long n, unsigned long k) {
    mpf_t fact1, fact2;

    mpf_init(fact1);
    mpf_init(fact2);

    factorial(fact1, n, k);
    factorial(fact2, n - k, 0);
    mpf_div(result, fact1, fact2);

    mpf_clear(fact1);
    mpf_clear(fact2);
}

double pHit(unsigned long i, unsigned long assoc, unsigned long distance,
        double pHitSet) {
    mpf_t mpHitSet, mpMissSet, mpResult;

    mpf_init(mpHitSet);
    mpf_init(mpMissSet);
    mpf_init(mpResult);

    mpf_set_ui(mpResult, 1);
    nCk(mpResult, distance, i);

    mpf_set_d(mpHitSet, pHitSet);
    mpf_set_d(mpMissSet, 1-pHitSet);

    mpf_pow_ui(mpHitSet, mpHitSet, i);
    mpf_pow_ui(mpMissSet, mpMissSet, distance-i);

    mpf_mul(mpResult, mpResult, mpHitSet);
    mpf_mul(mpResult, mpResult, mpMissSet);

    double result = mpf_get_d(mpResult);

    mpf_clear(mpResult);
    mpf_clear(mpMissSet);
    mpf_clear(mpHitSet);

    return result;
}

void setDouble(mpf_t* n, double num) {
    mpf_init (*n);
    mpf_set_d(*n, num);
}

void accumulateStride(mpf_t* current, double normalized) {
    mpf_t mpNorm, mpCube;
    mpf_init(mpNorm);
    mpf_init(mpCube);

    mpf_set_d(mpNorm, normalized);

    mpf_pow_ui(mpCube, mpNorm, 3);
    mpf_add(*current, *current, mpCube);
}

double getDouble(mpf_t* n) {
    double v = mpf_get_d(*n);
    mpf_clear(*n);

    return v;
}

double hit_probability(long distance, long size, short associativity,
    int linesize) {
    long numsets = (long) (size / ((double) (associativity * linesize)));
    double pHitSet = 1.0/numsets, p = 0;
    long i;
    long target = associativity - 1 < distance ? associativity - 1 : distance;

    for (i=0; i<=target; i++) {
        // Find probability that exactly `i' references map the same set
        p += pHit(i, associativity, distance, pHitSet);
    }

    return p;
}
