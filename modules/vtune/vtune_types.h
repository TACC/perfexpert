/*
 * Copyright (c) 2011-2015  University of Texas at Austin. All rights reserved.
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

#ifndef PERFEXPERT_MODULE_VTUNE_TYPES_H_
#define PERFEXPERT_MODULE_VTUNE_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

/* PerfExpert common headers */
#include "common/perfexpert_hash.h"
#include "common/perfexpert_list.h"
#include "common/perfexpert_constants.h"

typedef struct {
    volatile perfexpert_list_item_t *next;
    volatile perfexpert_list_item_t *prev;
    char *name;   // Name of the counter (VTune name)
    char name_md5[33];
    long samples, value; //Number of samples and value
    int mpi_rank; // To which rank belongs this counter
    perfexpert_hash_handle_t hh_str;
} vtune_event_t;

typedef struct {
    char *prefix[MAX_ARGUMENTS_COUNT];
    char *before[MAX_ARGUMENTS_COUNT];
    char *after[MAX_ARGUMENTS_COUNT];
    char *mic;
    char *mic_prefix[MAX_ARGUMENTS_COUNT];
    char *mic_before[MAX_ARGUMENTS_COUNT];
    char *mic_after[MAX_ARGUMENTS_COUNT];
    char *inputfile;
//    char *res_folder;
    char res_folder [MAX_FILENAME];
    vtune_event_t *events_by_name;
    int ignore_return_code;
} my_module_globals_t;

typedef struct {
    volatile perfexpert_list_item_t *next;
    volatile perfexpert_list_item_t *prev;
    int id;
    char * name;
    perfexpert_list_t events_list;
    vtune_event_t *events_by_name;
//    vtune_event_t * events_by_id;
} vtune_hw_profile_t;

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_MODULE_VTUNE_TYPES_H_ */
