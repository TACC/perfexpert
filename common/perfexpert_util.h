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

#ifndef PERFEXPERT_UTIL_H_
#define PERFEXPERT_UTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Function declarations */
int perfexpert_util_make_path(const char *path);
int perfexpert_util_dir_exists(const char *dir);
int perfexpert_util_remove_dir(const char *dir);
int perfexpert_util_file_exists(const char *file);
int perfexpert_util_file_is_exec(const char *file);
int perfexpert_util_filename_only(const char *file, char **only);
int perfexpert_util_path_only(const char *file, char **path);
int perfexpert_util_file_copy(const char *to, const char *from);
int perfexpert_util_file_print(const char *file);

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_UTIL_H */
