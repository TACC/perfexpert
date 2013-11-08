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

#ifndef PERFEXPERT_DATABASE_H_
#define PERFEXPERT_DATABASE_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _STRING_H
#include <string.h>
#endif

#ifndef _STDLIB_H
#include <stdlib.h>
#endif

#ifndef _UNISTD_H
#include <unistd.h>
#endif

#ifndef _SQLITE3_H_
#include <sqlite3.h>
#endif

#include "common/perfexpert_alloc.h"
#include "common/perfexpert_constants.h"
#include "common/perfexpert_output.h"
#include "common/perfexpert_util.h"
#include "install_dirs.h"

/* perfexpert_database_update */
static inline int perfexpert_database_update(char **file) {
    char *sys_ver = NULL, *my_ver = NULL, *my_db = NULL;
    int  rc = PERFEXPERT_ERROR;
    FILE *ver_FP;

    PERFEXPERT_ALLOC(char, my_db,
        (strlen(getenv("HOME")) + strlen(PERFEXPERT_DB) + 3));
    sprintf(my_db, "%s/.%s", getenv("HOME"), PERFEXPERT_DB);

    PERFEXPERT_ALLOC(char, sys_ver,
        (strlen(PERFEXPERT_ETCDIR) + strlen(PERFEXPERT_DB) + 10));
    sprintf(sys_ver, "%s/%s.version", PERFEXPERT_ETCDIR, PERFEXPERT_DB);

    PERFEXPERT_ALLOC(char, my_ver,
        (strlen(getenv("HOME")) + strlen(PERFEXPERT_DB) + 11));
    sprintf(my_ver, "%s/.%s.version", getenv("HOME"), PERFEXPERT_DB);

    /* System version */
    if (NULL == (ver_FP = fopen(sys_ver, "r"))) {
        OUTPUT(("%s", _ERROR("unable to open sys DB version file")));
        goto CLEAN_UP;
    }
    if (0 == fscanf(ver_FP, "%s", sys_ver)) {
        OUTPUT(("%s", _ERROR("unable to read sys DB version file")));
        fclose(ver_FP);
        goto CLEAN_UP;
    }
    fclose(ver_FP);
    OUTPUT_VERBOSE((10, "      system database version (%s)", sys_ver));

    /* My version */
    if ((PERFEXPERT_SUCCESS == perfexpert_util_file_exists(my_db)) &&
        (PERFEXPERT_SUCCESS == perfexpert_util_file_exists(my_ver))) {
        OUTPUT_VERBOSE((10, "      found local database (%s)", my_db));
        if (NULL != (ver_FP = fopen(my_ver, "r"))) {
            if (0 == fscanf(ver_FP, "%s", my_ver)) {
                OUTPUT(("%s", _ERROR("unable to read user DB version file")));
                fclose(ver_FP);
                goto CLEAN_UP;
            }
        }
        fclose(ver_FP);
        OUTPUT_VERBOSE((10, "      local database version (%s)", my_ver));
    } else {
        OUTPUT_VERBOSE((10, "      local database not found, creating one"));
    }

    /* Compare */
    if (atof(my_ver) > atof(sys_ver)) {
        OUTPUT(("      local database is newer, reporting error"));
        goto CLEAN_UP;
    } else if (atof(my_ver) == atof(sys_ver)) {
        goto DATABASE_OK;
    } else {
        OUTPUT_VERBOSE((10, "      system database is newer, updating"));
    }

    /* Setup a new database */
    if (0 != system("perfexpert_setup_db.sh")) {
        OUTPUT(("%s", _ERROR("unable to setup PerfExpert database")));
        goto CLEAN_UP;
    }
    sprintf(sys_ver, "%s/%s.version", PERFEXPERT_ETCDIR, PERFEXPERT_DB);
    sprintf(my_ver, "%s/.%s.version", getenv("HOME"), PERFEXPERT_DB);
    unlink(my_ver);
    if (PERFEXPERT_SUCCESS != perfexpert_util_file_copy(my_ver, sys_ver)) {
        goto CLEAN_UP;
    }

    DATABASE_OK:
    *file = my_db;
    rc = PERFEXPERT_SUCCESS;

    CLEAN_UP:
    PERFEXPERT_DEALLOC(my_ver);
    PERFEXPERT_DEALLOC(sys_ver);

    return rc;
}

/* perfexpert_database_disconnect */
static inline int perfexpert_database_disconnect(sqlite3 *db) {
    sqlite3_close(db);

    return PERFEXPERT_SUCCESS;
}

/* perfexpert_database_connect */
static inline int perfexpert_database_connect(sqlite3 **db, const char *file) {
    char *my_file = (char *)file;
    int allocated = PERFEXPERT_FALSE;

    /* Use default database if the user does not define one */
    if (NULL == my_file) {
        PERFEXPERT_ALLOC(char, my_file,
            (strlen(PERFEXPERT_DB) + strlen(getenv("HOME")) + 3));
        sprintf(my_file, "%s/.%s", getenv("HOME"), PERFEXPERT_DB);
        allocated = PERFEXPERT_TRUE;
    }

    /* Check if file exists and if it is writable */
    if (-1 == access(my_file, F_OK)) {
        OUTPUT(("%s (%s)", _ERROR((char *)"file not found"), my_file));
        goto CLEAN_UP;
    }
    if (-1 == access(my_file, W_OK)) {
        OUTPUT(("%s (%s)",
            _ERROR((char *)"you don't have permissions to write on"), my_file));
        goto CLEAN_UP;
    }

    /* Connect to the DB */
    if (SQLITE_OK != sqlite3_open(my_file, db)) {
        OUTPUT(("%s (%s), %s", _ERROR((char *)"openning database"), my_file,
            sqlite3_errmsg(*db)));
        perfexpert_database_disconnect(*db);
        goto CLEAN_UP;
    }

    OUTPUT_VERBOSE((4, "      connected to %s", my_file));

    return PERFEXPERT_SUCCESS;

    CLEAN_UP:
    if (PERFEXPERT_TRUE == allocated) {
        PERFEXPERT_DEALLOC(my_file);
    }
    return PERFEXPERT_ERROR;
}

/* perfexpert_database_get_int */
static inline int perfexpert_database_get_int(void *var, int count, char **val,
    char **names) {
    int *temp = (int *)var;
    if (NULL != val[0]) {
        *temp = atoi(val[0]);
    }
    return PERFEXPERT_SUCCESS;
}

/* perfexpert_database_get_unsigned_long_long_int */
static inline int perfexpert_database_get_long_long_int(void *var,
    int count, char **val, char **names) {
    int *temp = (int *)var;
    if (NULL != val[0]) {
        *temp = strtoll(val[0], NULL, 10);
    }
    return PERFEXPERT_SUCCESS;
}

/* perfexpert_database_get_double */
static inline int perfexpert_database_get_double(void *var, int count,
    char **val, char **names) {
    double *temp = (double *)var;
    if (NULL != val[0]) {
        *temp = atof(val[0]);
    }
    return PERFEXPERT_SUCCESS;
}

/* perfexpert_database_get_string */
static inline int perfexpert_database_get_string(void *var, int count,
    char **val, char **names) {
    char **temp = (char **)var;
    if (NULL != val[0]) {
        PERFEXPERT_ALLOC(char, *temp, (strlen(val[0]) + 1));
        strcpy(*temp, val[0]);
    }
    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_DATABASE_H */
