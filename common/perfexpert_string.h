/*
 * Copyright (c) 2011-2015  University of Texas at Austin. All rights reserved.
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
 * Authors: Antonio Gomez-Iglesias, Leonardo Fialho and Ashay Rane
 *
 * $HEADER$
 */

#ifndef PERFEXPERT_STRING_H_
#define PERFEXPERT_STRING_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Function declarations */
int perfexpert_string_split(char *in, char **out, int token);
char* perfexpert_string_remove_spaces(char *str);
char* perfexpert_string_remove_char_pos(char *str, int token, int pos);
char* perfexpert_string_remove_char(char *str, int token);
char* perfexpert_string_replace_char(char *str, int out, int in);
char* perfexpert_string_to_lower(char *str);

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_STRING_H */
