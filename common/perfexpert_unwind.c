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

int perfexpert_unwind_get_file_line (unw_word_t addr, char *file, size_t len, int *line, char *executable) {
    static char buf[256];
    char *p; 

    // prepare command to be executed
    // our program need to be passed after the -e parameter
    sprintf (buf, "/usr/bin/addr2line -C -e %s -f -i %lx", executable, addr);
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

// Call this function to get a backtrace.
void capture_backtrace(char *executable) { 
    char name[256];
    unw_cursor_t cursor; unw_context_t uc; 
    unw_word_t ip, sp, offp;

    unw_getcontext(&uc);
    unw_init_local(&cursor, &uc);

    while (unw_step(&cursor) > 0)
    {   
        char file[256];
        int line = 0;

        name[0] = '\0';
        unw_get_proc_name(&cursor, name, 256, &offp);
        unw_get_reg(&cursor, UNW_REG_IP, &ip);
        unw_get_reg(&cursor, UNW_REG_SP, &sp);

        perfexpert_unwind_get_file_line ((long)ip, file, 256, &line, executable);
        printf("%s in file %s line %d\n", name, file, line);
    }   
}

#ifdef __cplusplus
}
#endif
