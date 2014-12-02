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

/* headers */
#include <stdio.h>
#include "cache_sim.h"

/* main */
int main(int argc, char *argv[]) {
    cache_handle_t *cache;
    int i = 0;

    if (NULL == (cache = cache_sim_init(32768, 64, 8, "lru"))) {
        printf("Error\n");
        exit(1);
    } else {
        printf("Ok!\n");
    }

    for (i = 0; i < 4096; i+=4) {
        cache->access_fn(cache, i);
    }
    for (i = 0; i < 1024*8*4096; i+=4) {
        cache->access_fn(cache, i);
    }
    for (i = 0; i < 4096; i+=4) {
        cache->access_fn(cache, i);
    }

    cache_sim_fini(cache);

    exit(0);
}

// EOF
