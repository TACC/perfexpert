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

#ifndef PREFEXPERT_MODULE_IO_OUTPUT_H_
#define PREFEXPERT_MODULE_IO_OUTPUT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "io.h"


#include <stdio.h>
#include <string.h>

#define UNW_LOCAL_ONLY
#include <libunwind.h>


static int perfexpert_unwind_get_file_line (unw_word_t addr, char *file, size_t len, int *line, char *executable);
int generate_raw_output(char *output_file, char * executable);
int output_analysis();

#ifdef __cplusplus
}
#endif

#endif /* PREFEXPERT_MODULE_IO_OUTPUT_H_ */ 

