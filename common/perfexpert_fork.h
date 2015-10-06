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

#ifndef PERFEXPERT_FORK_H_
#define PERFEXPERT_FORK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "common/perfexpert_list.h"

/** Ninja structure to hold a list of tests to perform */
typedef struct test {
    volatile perfexpert_list_item_t *next;
    volatile perfexpert_list_item_t *prev;
    char *info;   // just to make the output meaningful
    char *input;  // send via STDIN
    char *output; // collected via STDOUT
} test_t;

/* Function declarations */
int perfexpert_fork_and_wait(test_t *test, char *argv[]);

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_FORK_H */
