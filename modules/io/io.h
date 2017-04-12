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

#ifndef PERFEXPERT_MODULE_IO_H_
#define PERFEXPERT_MODULE_IO_H_

#ifdef __cplusplus
extern "C" {
#endif

#define FOPEN 0
#define FCLOSE 1

#define FWRITE 2
#define FPRINTF 3
#define FPUTS 4

#define FREAD 5
#define FSCANF 6
#define FGETS 7

#define MAX_FUNCTIONS 8

/* Tools headers */
#include "tools/perfexpert/perfexpert_types.h"

/*  PerfExpert common headers */
#include "common/perfexpert_fork.h"

#ifdef PROGRAM_PREFIX
#undef PROGRAM_PREFIX
#endif
#define PROGRAM_PREFIX "[perfexpert_module_io]"

#define IO "IO"

typedef struct {
    char function_name [256]; //Function that originates an IO request
    long address;  //Address where that IO requests happens
    unsigned long count; //Number of times the request has been issued
} code_function_t;

//extern code_function_t code_function;


typedef struct {
    code_function_t *code;
    int size; // Total number of functions with IO requests for this specific function
} io_function_t;


/* Private module types */
typedef struct experiment {
    volatile perfexpert_list_item_t *next;
    volatile perfexpert_list_item_t *prev;
    test_t test;
    char *argv[MAX_ARGUMENTS_COUNT];
    int  argc;
} experiment_t;  


//extern io_data_t io_data;
/* Module types */
typedef struct {
//    double total;
//    double maximum;
//    double minimum;
    // Don't think I need the next three
    char *prefix[MAX_ARGUMENTS_COUNT];
    char *before[MAX_ARGUMENTS_COUNT];
    char *after[MAX_ARGUMENTS_COUNT];
    io_function_t data[MAX_FUNCTIONS];
    char *inputfile;
    int ignore_return_code;
} my_module_globals_t;

extern my_module_globals_t my_module_globals;

/* Module interface */
int module_load(void);
int module_init(void);
int module_fini(void);
int module_measure(void);
int module_analyze(void);

/* Module functions */
int parse_module_args(int argc, char *argv[]);
int output_analysis();

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_MODULE_IO_H_ */
