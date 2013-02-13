/*
 * Copyright (c) 2013  University of Texas at Austin. All rights reserved.
 *
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * This file is part of OptTran and PerfExpert.
 *
 * OptTran as well PerfExpert are free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * OptTran and PerfExpert are distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with OptTran or PerfExpert. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Leonardo Fialho
 *
 * $HEADER$
 */

#ifndef OPTTRAN_UTIL_H_
#define OPTTRAN_UTIL_H_

#ifndef	_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifndef	_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifndef	_ERRNO_H
#include <errno.h>
#endif

#ifndef _STRING_H_
#include <string.h>
#endif

#ifndef OPTTRAN_CONSTANTS_H
#include "opttran_constants.h"
#endif

#ifndef OPTTRAN_OUTPUT_H
#include "opttran_output.h"
#endif

/* make_path: create an entire directory tree (if needed), like 'mkdir -p' */
static int opttran_util_make_path(char *path, int nmode) {
    int oumask;
    char *p = NULL;
    char *npath = NULL;
    struct stat sb;

    /* Check if we can create such directory */
    if (0 == stat(path, &sb)) {
        if (0 == S_ISDIR (sb.st_mode)) {
            OPTTRAN_OUTPUT(("%s '%s'",
                            _ERROR("file exists but is not a directory"),
                            path));
            return OPTTRAN_ERROR;
        }
        if (chmod(path, nmode)) {
            OPTTRAN_OUTPUT(("%s %s: %s", _ERROR("system error"), path,
                            strerror(errno)));
            return OPTTRAN_ERROR;
        }
        return OPTTRAN_SUCCESS;
    }
    
    /* Save a copy, so we can write to it */
    npath = malloc(strlen(path));
    if (NULL == npath) {
        OPTTRAN_OUTPUT(("%s", _ERROR("Error: out of memory")));
        exit(OPTTRAN_ERROR);
    }
    strcpy(npath, path);
    
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
                OPTTRAN_OUTPUT(("%s '%s': %s",
                                _ERROR("cannot create directory"), npath,
                                strerror(errno)));
                free(npath);
                return OPTTRAN_ERROR;
            }
        } else if (0 == S_ISDIR(sb.st_mode)) {
            OPTTRAN_OUTPUT(("'%s': %s", npath,
                            _ERROR("file exists but is not a directory")));
            free (npath);
            return OPTTRAN_ERROR;
        }
        *p++ = '/'; /* restore slash */
        while ('/' == *p) {
            p++;
        }
    }
    
    /* Create the final directory component */
    if (stat(npath, &sb) && mkdir(npath, nmode)) {
        OPTTRAN_OUTPUT(("%s '%s': %s", _ERROR("cannot create directory"),
                        npath, strerror(errno)));
        free(npath);
        return OPTTRAN_ERROR;
    }

    free(npath);
    return OPTTRAN_SUCCESS;
}

#endif /* OPTTRAN_UTIL_H */
