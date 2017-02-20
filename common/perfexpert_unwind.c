/*
 * Copyright (c) 2011-2017  University of Texas at Austin. All rights reserved.
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
 * Authors: Antonio Gomez-Iglesias
 *
 * $HEADER$
 */

#ifdef __cplusplus
extern "C" {
#endif

#define UNW_LOCAL_ONLY
#include <libunwind.h>
#include <stdio.h>
#include "common/perfexpert_constants.h" 

int perfexpert_unwind_get_file_line (unw_word_t addr, char *file, size_t len, int *line, char *code) {
    static char buf[256];
    char *p; 

    // prepare command to be executed
    // our program need to be passed after the -e parameter
    sprintf (buf, "/usr/bin/addr2line -C -e %s -f -i %lx", code, addr);
    FILE* f = popen (buf, "r");

    if (f == NULL)
    {   
        perror (buf);
        return 0;
    }   

    // get function name
    fgets (buf, 256, f); 

    // get file and line
    fgets (buf, 256, f); 

    if (buf[0] != '?') {   
        int l;
        char *p = buf;

        // file name is until ':'
        while (*p != ':') {   
            p++;
        }   

        *p++ = 0;
        // after file name follows line number
        strcpy (file , buf);
        sscanf (p,"%d", line);
    }   
    else {   
        strcpy (file,"unkown");
        *line = 0;
    }   
    pclose(f);
    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif
