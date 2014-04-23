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

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "common/perfexpert_fake_globals.h"
#include "common/perfexpert_constants.h"
#include "common/perfexpert_fork.h"
#include "common/perfexpert_output.h"
#include "common/perfexpert_list.h"
#include "install_dirs.h"

static long long unsigned int perfexpert_alloc_account;

/* perfexpert_alloc_add */
void perfexpert_alloc_add(void *ptr, ssize_t size) {
}

/* perfexpert_alloc_del */
void perfexpert_alloc_del(void *ptr) {
}

/* perfexpert_alloc_free_all */
void perfexpert_alloc_free_all(void) {
}

#ifdef __cplusplus
}
#endif
