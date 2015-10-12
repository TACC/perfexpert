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

// A hotspot is each one of the functions evaluated with VTune. In the
// output that we process, it corresponds to one line like:
// bpnn_zero_weights','backprop','0','0','800018','0','0','0','0','0','0','0','0','0','0','0','0','0','0','100003','4000006','0','0
// 'name' is the first column of the line.
// The events is a list of vtune_event_t
typedef struct {
    volatile perfexpert_list_item_t *next;
    volatile perfexpert_list_item_t *prev;
    char *name;
    char name_md5[33];

    perfexpert_list_t events;
    vtune_event_t *events_by_name;
    perfexpert_hash_handle_t hh_str;
} vtune_hotspots_t;

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
//    volatile perfexpert_list_item_t *next;
//    volatile perfexpert_list_item_t *prev;
    int id;
    char * name;
    perfexpert_list_t hotspots;
    vtune_hotspots_t * hotspots_by_name;
//    perfexpert_list_t events_list;
//    vtune_event_t *events_by_name;
//    vtune_event_t * events_by_id;
} vtune_hw_profile_t;

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_MODULE_VTUNE_TYPES_H_ */
