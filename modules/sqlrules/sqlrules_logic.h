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

#ifndef PERFEXPERT_MODULE_SQLRULES_LOGIC_H_
#define PERFEXPERT_MODULE_SQLRULES_LOGIC_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Structure to hold strategies to select recommendations */
typedef struct {
    volatile perfexpert_list_item_t *next;
    volatile perfexpert_list_item_t *prev;
    char *name;
    char *query;
} rule_t;

/* Function declarations */
static int accumulate_rules(void *rules, int c, char **val, char **names);
static int select_recom_hotspot(void *var, int c, char **val, char **names);
static int print_recom(void *var, int count, char **val, char **names);

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_MODULE_SQLRULES_LOGIC_H_ */
