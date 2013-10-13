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

#ifndef ANALYZER_OUTPUT_H_
#define ANALYZER_OUTPUT_H_

#ifdef __cplusplus
extern "C" {
#endif

#define GET_COMPILER_INFO_PROGRAM "perfexpert_get_compiler_info.pl"

/* Function definitions */
static int output_analysis(profile_t *profile, procedure_t *hotspot);
static int output_metrics(profile_t *profile, procedure_t *hotspot,
	FILE *file_FP);
static int print_compiler_info(procedure_t *hotspot, FILE *file_FP);

#ifdef __cplusplus
}
#endif

#endif /* ANALYZER_OUTPUT_H_ */
