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

#ifndef PERFEXPERT_UTIL_H_
#define PERFEXPERT_UTIL_H_

#ifdef __cplusplus
extern "C" {
#endif
    
#ifndef	_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifndef	_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifndef	_ERRNO_H
#include <errno.h>
#endif

#ifndef _STRING_H
#include <string.h>
#endif

#ifndef _STDLIB_H
#include <stdlib.h>
#endif

#ifndef _SQLITE3_H_
#include <sqlite3.h>
#endif

#ifndef INSTALL_DIRS_H
#include "install_dirs.h"
#endif

#ifndef PERFEXPERT_CONSTANTS_H
#include "perfexpert_constants.h"
#endif

#ifndef PERFEXPERT_OUTPUT_H
#include "perfexpert_output.h"
#endif

/* perfexpert_util_make_path: create an entire directory tree (if needed), like 'mkdir -p' */
static int perfexpert_util_make_path(char *path, int nmode) {
    int oumask;
    char *p = NULL;
    char *npath = NULL;
    struct stat sb;

    /* Check if we can create such directory */
    if (0 == stat(path, &sb)) {
        if (0 == S_ISDIR (sb.st_mode)) {
            OUTPUT(("%s '%s'",
                    _ERROR((char *)"file exists but is not a directory"), path));
            return PERFEXPERT_ERROR;
        }
        if (chmod(path, nmode)) {
            OUTPUT(("%s %s: %s", _ERROR((char *)"system error"), path,
                    strerror(errno)));
            return PERFEXPERT_ERROR;
        }
        return PERFEXPERT_SUCCESS;
    }

    /* Save a copy, so we can write to it */
    npath = (char *)malloc(strlen(path) + 1);
    if (NULL == npath) {
        OUTPUT(("%s", _ERROR((char *)"Error: out of memory")));
        return PERFEXPERT_ERROR;
    }
    bzero(npath, strlen(path) + 1);
    strncpy(npath, path, strlen(path));

    /* Check whether or not we need to do anything with intermediate dirs */
    /* Skip leading slashes */
    p = npath;
    while ('/' == *p) {
        p++;
    }
    while (p = strchr (p, '/')) {
        *p = '\0';
        if (0 != stat(npath, &sb)) {
            if (mkdir(npath, nmode)) {
                OUTPUT(("%s '%s': %s",
                        _ERROR((char *)"cannot create directory"), npath,
                        strerror(errno)));
                free(npath);
                return PERFEXPERT_ERROR;
            }
        } else if (0 == S_ISDIR(sb.st_mode)) {
            OUTPUT(("'%s': %s", npath,
                    _ERROR((char *)"file exists but is not a directory")));
            free (npath);
            return PERFEXPERT_ERROR;
        }
        *p++ = '/'; /* restore slash */
        while ('/' == *p) {
            p++;
        }
    }
    
    /* Create the final directory component */
    if (stat(npath, &sb) && mkdir(npath, nmode)) {
        OUTPUT(("%s '%s': %s", _ERROR((char *)"cannot create directory"),
                npath, strerror(errno)));
        free(npath);
        return PERFEXPERT_ERROR;
    }

    free(npath);
    return PERFEXPERT_SUCCESS;
}

/* perfexpert_database_disconnect */
static int perfexpert_database_disconnect(sqlite3 *db) {
    sqlite3_close(db);
}

// TODO: add hack to lock database on Lustre filesystems
// TODO: register a signal handler to enable database disconnection when a TERM or KILL signal is received
/* perfexpert_database_connect */
static int perfexpert_database_connect(sqlite3 **db, char *file) {
    /* Use default database if used does not define one */
    if (NULL == file) {
        file = (char *)malloc(strlen(RECOMMENDATION_DB) +
                                     strlen(PERFEXPERT_VARDIR) + 2);
        if (NULL == file) {
            OUTPUT(("%s", _ERROR((char *)"Error: out of memory")));
            return PERFEXPERT_ERROR;
        }
        bzero(file, strlen(RECOMMENDATION_DB) + strlen(PERFEXPERT_VARDIR) + 2);
        sprintf(file, "%s/%s", PERFEXPERT_VARDIR, RECOMMENDATION_DB);
    }

    /* Check if file exists and if it is writable */
    if (-1 == access(file, F_OK)) {
        OUTPUT(("%s (%s)", _ERROR((char *)"Error: file not found"), file));
        return PERFEXPERT_ERROR;
    }
    if (-1 == access(file, W_OK)) {
        OUTPUT(("%s (%s)",
                _ERROR((char *)"Error: you don't have permission to write"),
                file));
        return PERFEXPERT_ERROR;
    }
    
    /* Connect to the DB */
    if (SQLITE_OK != sqlite3_open(file, db)) {
        OUTPUT(("%s (%s), %s", _ERROR((char *)"Error: openning database"),
                file, sqlite3_errmsg(*db)));
        perfexpert_database_disconnect(*db);
        return PERFEXPERT_ERROR;
    }

    OUTPUT_VERBOSE((4, "connected to %s", file));
    return PERFEXPERT_SUCCESS;
}

/* perfexpert_database_get_int */
static int perfexpert_database_get_int(void *var, int count, char **val, char **names) {
    int *temp = (int *)var;
    if (NULL != val[0]) {
        *temp = atoi(val[0]);
    }
    return PERFEXPERT_SUCCESS;
}

/* perfexpert_database_get_double */
static int perfexpert_database_get_double(void *var, int count, char **val, char **names) {
    double *temp = (double *)var;
    if (NULL != val[0]) {
        *temp = atof(val[0]);
    }
    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_UTIL_H */
