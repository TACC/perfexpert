/*
 * Copyright (c) 2011-2016  University of Texas at Austin. All rights reserved.
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
 * Authors: Antonio Gomez-Iglesias, Leonardo Fialho and Ashay Rane
 *
 * $HEADER$
 */

#ifndef PERFEXPERT_FAKE_GLOBALS_H_
#define PERFEXPERT_FAKE_GLOBALS_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Fake globals, just to compile the library */
 
 typedef struct {
    int verbose;
    int colorful;
    char *prefix;
    char *after;
    char *before;
    char *workdir;
    double threshold;
    int output_mode;
} globals_t;


extern globals_t globals; /* This variable is defined in the tool/module */

#ifndef PROGRAM_PREFIX
#define PROGRAM_PREFIX "[perfexpert]"
#endif

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_FAKE_GLOBALS_H_ */
