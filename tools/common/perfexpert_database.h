/*
 * Copyright (c) 2013  University of Texas at Austin. All rights reserved.
 *
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * This file is part of PerfExpert.
 *
 * PerfExpert is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * PerfExpert is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with PerfExpert. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Leonardo Fialho
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

#include "perfexpert_alloc.h"
#include "perfexpert_constants.h"
#include "perfexpert_output.h"
#include "perfexpert_util.h"
#include "install_dirs.h"

/* perfexpert_database_update */
static inline int perfexpert_database_update(char **file) {
    // TODO: this functions requires much more tests
    FILE  *version_file_FP;
    char  temp_str[6][BUFFER_SIZE];
    float my_version = 0.0;
    float sys_version = 0.0;

    bzero(temp_str[0], BUFFER_SIZE);
    sprintf(temp_str[0], "%s/%s", PERFEXPERT_VARDIR, PERFEXPERT_DB);
    bzero(temp_str[1], BUFFER_SIZE);
    sprintf(temp_str[1], "%s/.%s", getenv("HOME"), PERFEXPERT_DB);
    bzero(temp_str[2], BUFFER_SIZE);
    sprintf(temp_str[2], "%s/.%s.version", PERFEXPERT_VARDIR, PERFEXPERT_DB);
    bzero(temp_str[3], BUFFER_SIZE);
    sprintf(temp_str[3], "%s/.%s.version", getenv("HOME"), PERFEXPERT_DB);
    bzero(temp_str[4], BUFFER_SIZE);
    sprintf(temp_str[4], "%s/.%s~", getenv("HOME"), PERFEXPERT_DB);
    bzero(temp_str[5], BUFFER_SIZE);

    if (PERFEXPERT_SUCCESS == perfexpert_util_file_exists(temp_str[1])) {
        OUTPUT_VERBOSE((10, "      found local database (%s)", temp_str[1]));
        if (PERFEXPERT_SUCCESS == perfexpert_util_file_exists(temp_str[3])) {
            /* My version */
            version_file_FP = fopen(temp_str[3], "r");
            if (version_file_FP) {
                fread(temp_str[5], 1, sizeof(temp_str[5]), version_file_FP);
                my_version = atof(temp_str[5]);
                if (ferror(version_file_FP)) {
                    return PERFEXPERT_ERROR;
                }
                fclose(version_file_FP);
            } else {
                return PERFEXPERT_ERROR;
            }
            OUTPUT_VERBOSE((10, "      local database version (%f)",
                my_version));

            /* System's version */
            version_file_FP = fopen(temp_str[2], "r");
            if (version_file_FP) {
                fread(temp_str[5], 1, sizeof(temp_str[5]), version_file_FP);
                sys_version = atof(temp_str[5]);
                if (ferror(version_file_FP)) {
                    return PERFEXPERT_ERROR;
                }
                fclose(version_file_FP);
            } else {
                return PERFEXPERT_ERROR;
            }
            OUTPUT_VERBOSE((10, "      system database version (%f)",
                sys_version));

            /* Compare */
            if (my_version > sys_version) {
                return PERFEXPERT_ERROR;
            } else if (my_version == sys_version) {
                goto DATABASE_OK;
            } else if (my_version < sys_version) {
                rename(temp_str[1], temp_str[4]);
                unlink(temp_str[3]);
            }
        } else {
            OUTPUT_VERBOSE((10, "      local database version unknown"));
            rename(temp_str[1], temp_str[4]);
        }
    }

    OUTPUT_VERBOSE((10, "      updating database"));
    if (PERFEXPERT_SUCCESS != perfexpert_util_file_copy(temp_str[1],
            temp_str[0])) {
        OUTPUT(("%s (%s) >> (%s)", _ERROR((char *)"Error: unable to copy file"),
            temp_str[0], temp_str[1]));
        return PERFEXPERT_ERROR;
    }
    if (PERFEXPERT_SUCCESS != perfexpert_util_file_copy(temp_str[3],
        temp_str[2])) {
        OUTPUT(("%s (%s) >> (%s)", _ERROR((char *)"Error: unable to copy file"),
            temp_str[0], temp_str[1]));
        return PERFEXPERT_ERROR;
    }

    DATABASE_OK:
    *file = (char *)malloc(strlen(temp_str[1] + 1));
    bzero(*file, strlen(temp_str[1] + 1));
    strcpy(*file, temp_str[1]);

    return PERFEXPERT_SUCCESS;
}

/* perfexpert_database_disconnect */
static inline int perfexpert_database_disconnect(sqlite3 *db) {
    sqlite3_close(db);

    return PERFEXPERT_SUCCESS;
}

/* perfexpert_database_connect */
static inline int perfexpert_database_connect(sqlite3 **db, const char *file) {
    char *my_file = (char *)file;
    /* Use default database if the user does not define one */
    if (NULL == my_file) {
        PERFEXPERT_ALLOC(char, my_file, (strlen(PERFEXPERT_DB) +
            strlen(PERFEXPERT_VARDIR) + 2));
        sprintf(my_file, "%s/%s", PERFEXPERT_VARDIR, PERFEXPERT_DB);
    }

    /* Check if file exists and if it is writable */
    if (-1 == access(my_file, F_OK)) {
        OUTPUT(("%s (%s)", _ERROR((char *)"Error: file not found"), my_file));
        return PERFEXPERT_ERROR;
    }
    if (-1 == access(my_file, W_OK)) {
        OUTPUT(("%s (%s)",
            _ERROR((char *)"Error: you don't have permission to write"),
            my_file));
        return PERFEXPERT_ERROR;
    }

    /* Connect to the DB */
    if (SQLITE_OK != sqlite3_open(my_file, db)) {
        OUTPUT(("%s (%s), %s", _ERROR((char *)"Error: openning database"),
            my_file, sqlite3_errmsg(*db)));
        perfexpert_database_disconnect(*db);
        PERFEXPERT_DEALLOC(my_file);
        return PERFEXPERT_ERROR;
    }

    OUTPUT_VERBOSE((4, "connected to %s", my_file));
    PERFEXPERT_DEALLOC(my_file);
    return PERFEXPERT_SUCCESS;
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

/* perfexpert_database_get_double */
static inline int perfexpert_database_get_double(void *var, int count,
    char **val, char **names) {
    double *temp = (double *)var;
    if (NULL != val[0]) {
        *temp = atof(val[0]);
    }
    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_DATABASE_H */
