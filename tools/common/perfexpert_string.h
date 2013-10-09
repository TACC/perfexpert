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

#ifndef PERFEXPERT_STRING_H_
#define PERFEXPERT_STRING_H_

#ifdef __cplusplus
extern "C" {
#endif
    
#ifndef _STRING_H
#include <string.h>
#endif

/* perfexpert_string_remove_char */
static inline char* perfexpert_string_remove_char(char *str, int token) {
    char *output = str;
    int i, j;

    for (i = 0, j = 0; i < strlen(str); i++, j++) {
        if (str[i] != (char)token) {
            output[j] = str[i];
        } else {
            j--;
        }
    }
    output[j] = 0;
    return output;
}

/* perfexpert_string_replace_char */
static inline char* perfexpert_string_replace_char(char *str, int out, int in) {
    int i;

    for (i = 0; i < strlen(str); i++) {
        if (str[i] == (char)out) {
            str[i] = (char)in;
        }
    }
    return str;
}

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_STRING_H */
