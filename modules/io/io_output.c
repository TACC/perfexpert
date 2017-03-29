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

#ifndef PREFEXPERT_MODULE_IO_OUTPUT_H_
#define PREFEXPERT_MODULE_IO_OUTPUT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "io.h"


#include <stdio.h>
#include <string.h>

#define UNW_LOCAL_ONLY
#include <libunwind.h>

int perfexpert_unwind_get_file_line (unw_word_t addr, char *file, size_t len, int *line, char *executable) {
    char buf[256];
    char *p; 
    char out[256] = {0};

    if (executable == NULL)
        return -1;
    snprintf (buf, 256, "/usr/bin/addr2line -C -e %s -f -i %lx", executable, addr);
    FILE* f = popen (buf, "r");
    if (f == NULL)
    {  
        printf ("f is NULL!! \n"); 
//        perror (out);
        return PERFEXPERT_ERROR;
    }   

    printf ("f was correct\n");
    while (fgets(out, 256, f) != NULL) {
        printf ("Buffer: %s\n", out);
        if (out[0] != '?') {
            int l;
            int i = 0;
            char *p = out;

            // file name is until ':'
            while (*p != ':') {   
                p++;
            }

            *p++ = 0;
            // after file name follows line number
            strcpy (file , out);
            sscanf (p,"%d", line);
        }
        else {
            strcpy (file,"unkown");
            *line = 0;
        }
    }
    printf ("Out of the loop\n");
    pclose(f);
    return PERFEXPERT_SUCCESS;
}


int perfexpert_io_output(char *output, char * executable) {
    FILE *fp;
    ssize_t read;
    int line;
    size_t len;
    char *io_function, *function_name;
    unsigned long address;
    char filename[256];

    fp = fopen(output, "r");
    //Read line by line
    //Each line contains:
    //  io_function function_name address
    if (fp == NULL) {
        return PERFEXPERT_ERROR;
    }
    while ((read = getline(&line, &len, fp)) != -1) {
        sscanf(line, "%s %s %lu", &io_function, &function_name, &address);
        perfexpert_unwind_get_file_line (address, filename, 256, &line, executable);
    }
    fclose(fp);
    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

#endif /* PREFEXPERT_MODULE_IO_OUTPUT_H_ */ 

