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

/* perfexpert_database_disconnect */
static int perfexpert_database_disconnect(sqlite3 *db) {
    /* Close DB connection */
    sqlite3_close(db);

    return PERFEXPERT_SUCCESS;
}

/* perfexpert_database_connect */
static int perfexpert_database_connect(sqlite3 **db, char *file) {
    /* Use default database if the user does not define one */
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

#endif /* PERFEXPERT_DATABASE_H */
