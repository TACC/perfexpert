/*
 * Copyright (c) 2011-2017  University of Texas at Austin. All rights reserved.
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
 * Authors: Antonio Gomez-Iglesias
 *
 * $HEADER$
 */

#ifndef PERFEXPERT_UNWIND_H_
#define PERFEXPERT_UNWIND_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <libunwind.h>

/* Function declarations */
int perfexpert_unwind_get_file_line (unw_word_t addr, char *file, size_t len, int *line, char *code);

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_UNWIND_H */
