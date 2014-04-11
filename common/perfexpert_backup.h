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

#ifndef PERFEXPERT_BACKUP_H_
#define PERFEXPERT_BACKUP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "common/perfexpert_list.h"

/* Structure to hold a backup */
typedef struct perfexpert_backup {
    perfexpert_list_t *files;
} perfexpert_backup_t;

/* Structure to hold a file */
typedef struct perfexpert_backup_file {
    volatile perfexpert_list_item_t *next;
    volatile perfexpert_list_item_t *prev;
    perfexpert_list_t versions;
    int version_count;
} perfexpert_backup_file_t;

/* Structure to hold a version of a file */
typedef struct perfexpert_backup_file_version perfexpert_backup_file_version_t;
struct perfexpert_backup_file_version {
    volatile perfexpert_list_item_t *next;
    volatile perfexpert_list_item_t *prev;
    char *fullpath;
    char *pathonly;
    char *filename;
    char *copyfullpath;
    int  version;
    perfexpert_backup_file_version_t *original;
};

/* Function declarations */
int perfexpert_backup_create(perfexpert_backup_t *backup);
perfexpert_backup_file_version_t *perfexpert_backup_file(
    perfexpert_backup_t *backup, const char *file);
static perfexpert_backup_file_version_t *perfexpert_backup_add_version(
    perfexpert_backup_t *backup, perfexpert_backup_file_t *file,
    const char *version);
perfexpert_backup_file_version_t *perfexpert_backup_get_last_version(
    perfexpert_backup_t *backup, const char *file);

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_BACKUP_H */
