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

#ifndef _SYS_STAT_H
#include <sys/stat.h>
#endif

#ifndef _ERRNO_H
#include <errno.h>
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

#ifndef _FCNTL_H
#include <fcntl.h>
#endif

#include "perfexpert_alloc.h"
#include "perfexpert_constants.h"
#include "perfexpert_output.h"

/* perfexpert_util_make_path (create directory tree like 'mkdir -p') */
static inline int perfexpert_util_make_path(const char *path, int nmode) {
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

/* perfexpert_util_dir_exists */
static inline int perfexpert_util_dir_exists(const char *dir) {
    struct stat sb;

    if (0 != stat(dir, &sb)) {
        OUTPUT_VERBOSE((1, "%s (%s)",
            _RED((char *)"directory not found or you don't have permissions"),
            dir));
        return PERFEXPERT_ERROR;
    }
    if (!S_ISDIR(sb.st_mode)) {
        OUTPUT_VERBOSE((1, "%s (%s)", _RED((char *)"is not a directory"), dir));
        return PERFEXPERT_ERROR;        
    }

    return PERFEXPERT_SUCCESS;
}

/* perfexpert_util_remove_dir */
static inline int perfexpert_util_remove_dir(const char *dir) {
    char *command = (char *)malloc(strlen(dir) + 8);

    if (PERFEXPERT_SUCCESS != perfexpert_util_dir_exists(dir)) {
        return PERFEXPERT_ERROR;
    }
    if (NULL == command) {
        return PERFEXPERT_ERROR;
    }
    bzero(command, strlen(dir) + 8);
    sprintf(command, "rm -rf %s", dir);
    if (-1 == system(command)) {
        free(command);
        return PERFEXPERT_ERROR;
    }
    free(command);

    return PERFEXPERT_SUCCESS;
}

/* perfexpert_util_file_exists */
static inline int perfexpert_util_file_exists(const char *file) {
    if (0 != access(file, F_OK)) {
        OUTPUT_VERBOSE((10, "%s (%s)", _RED((char *)"file not found"), file));
        return PERFEXPERT_ERROR;
    }
    return PERFEXPERT_SUCCESS;
}

/* perfexpert_util_file_is_exec */
static inline int perfexpert_util_file_is_exec(const char *file) {
    if (0 != access(file, X_OK)) {
        OUTPUT_VERBOSE((10, "%s (%s)",
            _RED((char *)"file is not executable or does not exist"), file));
        return PERFEXPERT_ERROR;
    }
    return PERFEXPERT_SUCCESS;
}

/* perfexpert_util_filename_only */
static inline int perfexpert_util_filename_only(const char *all, char **only) {
    char *temp = NULL, *token = NULL, *last = NULL;

    PERFEXPERT_ALLOC(char, temp, (strlen(all) + 1));
    strcpy(temp, all);

    token = strtok(temp, "/");
    while (token = strtok(NULL, "/")) {
        last = token;
    }

    if (NULL == last) {
        *only = (char *)all;
    } else {
        PERFEXPERT_ALLOC(char, token, (strlen(last) + 1));
        strcpy(token, last);
        *only = token;
    }

    PERFEXPERT_DEALLOC(temp);

    return PERFEXPERT_SUCCESS;
}

/* perfexpert_util_path_only */
static inline int perfexpert_util_path_only(const char *file, char **path) {
    char *prog = NULL, *temp = NULL, *given_path = NULL;

    if (PERFEXPERT_SUCCESS != perfexpert_util_filename_only(file, &prog)) {
        OUTPUT(("%s", _ERROR((char *)"Error: unable to extract path")));
        return PERFEXPERT_ERROR;
    }

    PERFEXPERT_ALLOC(char, temp, (strlen(file) - strlen(prog) + 1));
    strncpy(temp, file, (strlen(file) - strlen(prog)));

    if (NULL == realpath(temp, given_path)) {
        given_path = getcwd(NULL, 0);
    }

    PERFEXPERT_ALLOC(char, *path, strlen(given_path + 2));
    strcpy(*path, given_path);
    strcat(*path, "/");

    PERFEXPERT_DEALLOC(given_path);

    return PERFEXPERT_SUCCESS;
}

/* perfexpert_util_file_copy */
static inline int perfexpert_util_file_copy(const char *to, const char *from) {
    int     fd_to, fd_from;
    char    buffer[BUFFER_SIZE];
    ssize_t nread;
    int     saved_errno;

    fd_from = open(from, O_RDONLY);
    if (0 > fd_from) {
        return PERFEXPERT_ERROR;
    }

    fd_to = open(to, O_WRONLY | O_CREAT | O_EXCL, 0666);
    if (0 > fd_to) {
        goto out_error;
    }

    while (0 < (nread = read(fd_from, buffer, sizeof(buffer)))) {
        char *out_ptr = buffer;
        ssize_t nwritten;

        do {
            nwritten = write(fd_to, out_ptr, nread);

            if (0 <= nwritten) {
                nread -= nwritten;
                out_ptr += nwritten;
            } else if (errno != EINTR) {
                goto out_error;
            }
        } while (0 < nread);
    }

    if (0 == nread) {
        if (0 > close(fd_to)) {
            fd_to = -1;
            goto out_error;
        }
        close(fd_from);

        return PERFEXPERT_SUCCESS;
    }

  out_error:
    saved_errno = errno;

    close(fd_from);
    if (0 <= fd_to) {
        close(fd_to);
    }

    errno = saved_errno;
    return PERFEXPERT_ERROR;
}

/* perfexpert_util_file_print */
static inline int perfexpert_util_file_print(const char *file) {
    char   buffer[BUFFER_SIZE];
    FILE   *file_FP = NULL;
    size_t nread = 0;

    if (PERFEXPERT_SUCCESS != perfexpert_util_file_exists(file)) {
        return PERFEXPERT_ERROR;
    }

    file_FP = fopen(file, "r");
    if (NULL != file_FP) {
        while (0 < (nread = fread(buffer, 1, sizeof(buffer), file_FP))) {
            fwrite(buffer, 1, nread, stdout);
        }
        if (ferror(file_FP)) {
            return PERFEXPERT_ERROR;
        }
        fclose(file_FP);
    } else {
        return PERFEXPERT_ERROR;
    }

    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_UTIL_H */
