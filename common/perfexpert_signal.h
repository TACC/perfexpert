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

#ifndef PERFEXPERT_SIGNAL_H_
#define PERFEXPERT_SIGNAL_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _STDIO_H
#include <stdio.h>
#endif

#ifndef _STDLIB_H
#include <stdlib.h>
#endif

#ifndef _EXECINFO_H
#include <execinfo.h>
#endif

/* perfexpert_sighandler */
static void perfexpert_sighandler(int sig) {
    void *buf[50];
    size_t size;

    size = backtrace(buf, 50);

    fprintf(stderr, "\nPerfExpert received an error signal (%d)\n\n", sig);
    fprintf(stderr, "Please, send a copy of this error and the compressed file "
        "of the temporary\ndirectory %s\nto fialho@utexas.edu. Also, include "
        "the command line you used to run PerfExpert.\n\nTo create a compressed"
        " file just run:\n\ntar -czvf error.tar.gz %s\n\nThis will help us to "
        "improve PerfExpert. You may also want join our mailing list\nto get "
        "some help. To do that, send a blank message to:\n\n                   "
        "perfexpert-subscribe@lists.tacc.utexas.edu\n\nBacktrace (%d):\n",
        globals.workdir, globals.workdir, size);

    backtrace_symbols_fd(buf, size, STDERR_FILENO);

    exit(1);
}

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_SIGNAL_H */
