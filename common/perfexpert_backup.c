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
#include <string.h>

#include "common/perfexpert_fake_globals.h"
#include "common/perfexpert_alloc.h"
#include "common/perfexpert_backup.h"
#include "common/perfexpert_constants.h"
#include "common/perfexpert_list.h"
#include "common/perfexpert_output.h"
#include "common/perfexpert_util.h"

/* perfexpert_backup_create */
int perfexpert_backup_create(perfexpert_backup_t *backup) {
    /* Sanity check */
    if (NULL == backup) {
        OUTPUT(("%s", _ERROR((char *)"backup handle is null")));
        return PERFEXPERT_ERROR;
    }

    perfexpert_list_construct((perfexpert_list_t *)backup);

    return PERFEXPERT_SUCCESS;
}

/* perfexpert_backup_file */
perfexpert_backup_file_version_t *perfexpert_backup_file(
    perfexpert_backup_t *backup, const char *file) {
    perfexpert_backup_file_t *filehandle = NULL;
    perfexpert_backup_file_version_t *versionhandle = NULL;

    /* Sanity check */
    if (NULL == backup) {
        OUTPUT(("%s", _ERROR((char *)"backup handle is null")));
        return NULL;
    }
    if (PERFEXPERT_SUCCESS != perfexpert_util_file_exists(file)) {
        OUTPUT(("%s", _ERROR((char *)"file does not exists")));
        return NULL;
    }
    if (PERFEXPERT_SUCCESS != perfexpert_util_file_is_readable(file)) {
        OUTPUT(("%s", _ERROR((char *)"file is not readable")));
        return NULL;
    }
    if (PERFEXPERT_SUCCESS != perfexpert_util_file_is_writable(file)) {
        OUTPUT(("%s", _ERROR((char *)"file is not writable")));
        return NULL;
    }

    /* Check if a backup of this file exists, if doesn't, create it */
    if (NULL == (versionhandle =
        perfexpert_backup_get_last_version(backup, file))) {
        /* Allocate some memory... and initialize the structure */
        PERFEXPERT_ALLOC(perfexpert_backup_file_t, filehandle,
            sizeof(perfexpert_backup_file_t));
        perfexpert_list_item_construct((perfexpert_list_item_t *)filehandle);
        perfexpert_list_construct((perfexpert_list_t *)&(filehandle->versions));
        filehandle->version_count = 0;

        /* Add file to backup set */
        perfexpert_list_append((perfexpert_list_t *)backup,
            (perfexpert_list_item_t*)filehandle);
    }

    /* Add a version to this file */
    if (NULL == (versionhandle = perfexpert_backup_add_version(backup,
        filehandle, file))) {
        OUTPUT(("%s", _ERROR((char *)"adding backup file version")));
    }
    filehandle->version_count++;

    return versionhandle;
}

/* perfexpert_backup_add_version */
static perfexpert_backup_file_version_t *perfexpert_backup_add_version(
    perfexpert_backup_t *backup, perfexpert_backup_file_t *filehandle,
    const char *file) {
    perfexpert_backup_file_version_t *versionhandle = NULL;
    char *t = NULL;

    /* Sanity check (somehow this is redundant... but doesn't hurt, right?) */
    if (NULL == backup) {
        OUTPUT(("%s", _ERROR((char *)"backup handle is null")));
        return NULL;
    }
    if (NULL == filehandle) {
        OUTPUT(("%s", _ERROR((char *)"file handle is null")));
        return NULL;
    }
    if (PERFEXPERT_SUCCESS != perfexpert_util_file_exists(file)) {
        OUTPUT(("%s", _ERROR((char *)"file does not exists")));
        return NULL;
    }
    if (PERFEXPERT_SUCCESS != perfexpert_util_file_is_readable(file)) {
        OUTPUT(("%s", _ERROR((char *)"file is not readable")));
        return NULL;
    }
    if (PERFEXPERT_SUCCESS != perfexpert_util_file_is_writable(file)) {
        OUTPUT(("%s", _ERROR((char *)"file is not writable")));
        return NULL;
    }

    /* Allocate some memory... and initialize the structure (partially) */
    PERFEXPERT_ALLOC(perfexpert_backup_file_version_t, versionhandle,
        sizeof(perfexpert_backup_file_version_t));

    if (PERFEXPERT_SUCCESS != perfexpert_util_path_only(file,
        &(versionhandle->pathonly))) {
        OUTPUT(("%s", _ERROR((char *)"unable to identify the file path")));
        goto CLEANUP;
    }

    if (PERFEXPERT_SUCCESS != perfexpert_util_filename_only(file,
        &(versionhandle->filename))) {
        OUTPUT(("%s", _ERROR((char *)"unable to identify the filename")));
        goto CLEANUP;
    }

    PERFEXPERT_ALLOC(char, versionhandle->fullpath, (2 +
        strlen(versionhandle->pathonly) + strlen(versionhandle->filename)));
    sprintf(versionhandle->fullpath, "%s/%s", versionhandle->pathonly,
        versionhandle->filename);

    versionhandle->version = filehandle->version_count + 1;

    /* Create a copy and set the proper value on the structure */
    PERFEXPERT_ALLOC(char, t, (strlen(globals.workdir) +
        strlen(versionhandle->pathonly) + 9));
    sprintf(t, "%s/backup/%s", globals.workdir, versionhandle->pathonly);
    if (PERFEXPERT_SUCCESS != perfexpert_util_make_path(t)) {
        OUTPUT(("%s (%s)", _ERROR((char *)"creating backup path"), t));
        goto CLEANUP;
    }
    PERFEXPERT_DEALLOC(t);

    PERFEXPERT_ALLOC(char, t, (20 + strlen(globals.workdir) +
        strlen(versionhandle->pathonly) + strlen(versionhandle->filename)));
    sprintf(t, "%s/backup/%s/%s_%d", globals.workdir, versionhandle->pathonly,
        versionhandle->filename, versionhandle->version);
    if (PERFEXPERT_SUCCESS != perfexpert_util_file_copy(t, file)) {
        OUTPUT(("%s (%s)", _ERROR((char *)"unable to backup file"), t));
        goto CLEANUP;
    }
    versionhandle->copyfullpath = t;

    /* Add version to file handle */
    perfexpert_list_append((perfexpert_list_t *)filehandle,
        (perfexpert_list_item_t*)versionhandle);

    /* Set the original version */
    versionhandle->original = (perfexpert_backup_file_version_t *)
        perfexpert_list_get_first((perfexpert_list_t *)&(filehandle->versions));

    return versionhandle;

    CLEANUP:
    PERFEXPERT_DEALLOC(versionhandle->fullpath);
    PERFEXPERT_DEALLOC(versionhandle->pathonly);
    PERFEXPERT_DEALLOC(versionhandle->filename);
    PERFEXPERT_DEALLOC(versionhandle);
    PERFEXPERT_DEALLOC(t);

    return NULL;
}

perfexpert_backup_file_version_t *perfexpert_backup_get_last_version(
    perfexpert_backup_t *b, const char *file) {
    perfexpert_backup_file_t *f = NULL;
    perfexpert_backup_file_version_t *v = NULL;
    char *pathonly = NULL, *filename = NULL, *fullpath = NULL;

    if (PERFEXPERT_SUCCESS != perfexpert_util_path_only(file, &pathonly)) {
        OUTPUT(("%s", _ERROR((char *)"unable to identify the file path")));
        goto CLEANUP;
    }

    if (PERFEXPERT_SUCCESS != perfexpert_util_filename_only(file, &filename)) {
        OUTPUT(("%s", _ERROR((char *)"unable to identify the filename")));
        goto CLEANUP;
    }

    PERFEXPERT_ALLOC(char, fullpath, (strlen(pathonly) + strlen(filename) + 2));
    sprintf(fullpath, "%s/%s", pathonly, filename);

    perfexpert_list_for(f, (perfexpert_list_t *)b, perfexpert_backup_file_t) {
        v = (perfexpert_backup_file_version_t *)perfexpert_list_get_first(
            (perfexpert_list_t *)&(f->versions));
        if (0 == strcmp(fullpath, v->fullpath)) {
            goto CLEANUP;
        }
        v = NULL;
    }

    CLEANUP:
    PERFEXPERT_DEALLOC(fullpath);
    PERFEXPERT_DEALLOC(pathonly);
    PERFEXPERT_DEALLOC(filename);

    return v;
}

#ifdef __cplusplus
}
#endif
