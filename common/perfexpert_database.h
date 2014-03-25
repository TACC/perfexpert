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

/* Function declarations */
int perfexpert_database_update(char **file);
int perfexpert_database_disconnect(sqlite3 *db);
int perfexpert_database_connect(sqlite3 **db, const char *file);
int perfexpert_database_get_int(void *var, int count, char **val, char **names);
int perfexpert_database_get_long_long_int(void *var, int count, char **val,
    char **names);
int perfexpert_database_get_double(void *var, int count, char **val,
    char **names);
int perfexpert_database_get_string(void *var, int count, char **val,
    char **names);

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_DATABASE_H_ */
