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

#ifdef __cplusplus
extern "C" {
#endif

/* System standard headers */
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dwarf.h>
#include <libdwarf.h>
#include <stdio.h>
#include <strings.h>
#include <sqlite3.h>

/* Modules headers */
#include "readelf.h"

/* PerfExpert common headers */
#include "common/perfexpert_output.h"

/* get_compiler_info */
int get_compiler_info(void) {
    Dwarf_Unsigned   cu_header_length, abbrev_offset, next_cu_header;
    Dwarf_Half       version, address_size;
    Dwarf_Signed     attrcount, i, language;
    Dwarf_Die        no_die = 0, cu_die;
    Dwarf_Attribute *attrs;
    Dwarf_Half       attrcode;
    Dwarf_Debug      dbg = 0;
    Dwarf_Error      err;
    char *compiler = NULL;
    int fd = -1;

    OUTPUT_VERBOSE((5, "%s", _BLUE("Extracting DWARF info")));

    /* Open the binary */
    if (0 > (fd = open(globals.program_full, O_RDONLY))) {
        OUTPUT(("%s [%s]", _ERROR("opening program binary"),
            globals.program_full));
        return PERFEXPERT_ERROR;
    }

    /* Initialize DWARF library  */
    if (DW_DLV_OK != dwarf_init(fd, DW_DLC_READ, 0, 0, &dbg, &err)) {
        OUTPUT(("%s", _ERROR("failed DWARF initialization")));
        return PERFEXPERT_ERROR;
    }

    /* Find compilation unit header */
    if (DW_DLV_ERROR == dwarf_next_cu_header(dbg, &cu_header_length,
        &version, &abbrev_offset, &address_size, &next_cu_header, &err)) {
        OUTPUT(("%s", _ERROR("reading DWARF CU header")));
        return PERFEXPERT_ERROR;
    }

    /* Expect the CU to have a single sibling, a DIE */
    if (DW_DLV_ERROR == dwarf_siblingof(dbg, no_die, &cu_die, &err)) {
        OUTPUT(("%s", _ERROR("getting sibling of CU")));
        return PERFEXPERT_ERROR;
    }

    /* Find the DIEs attributes */
    if (DW_DLV_OK != dwarf_attrlist(cu_die, &attrs, &attrcount, &err)) {
        OUTPUT(("%s", _ERROR("in dwarf_attlist")));
        return PERFEXPERT_ERROR;
    }

    /* For each attribute... */
    for (i = 0; i < attrcount; ++i) {
        if (DW_DLV_OK != dwarf_whatattr(attrs[i], &attrcode, &err)) {
            OUTPUT(("%s [%s]", _ERROR("in dwarf_whatattr")));
            return PERFEXPERT_ERROR;
        }
        if (DW_AT_producer == attrcode) {
            if (DW_DLV_OK != dwarf_formstring(attrs[i], &compiler, &err)) {
                OUTPUT(("%s [%s]", _ERROR("in dwarf_formstring")));
                return PERFEXPERT_ERROR;
            } else {
                OUTPUT_VERBOSE((5, "   Compiler: %s", _CYAN(compiler)));
            }
        }
        if (DW_AT_language == attrcode) {
            if (DW_DLV_OK != dwarf_formsdata(attrs[i], &language, &err)) {
                OUTPUT(("%s [%s]", _ERROR("in dwarf_formsdata")));
                return PERFEXPERT_ERROR;
            } else {
                OUTPUT_VERBOSE((5, "   Language: %d", language));
            }
        }
    }

    if (PERFEXPERT_SUCCESS != database_write(compiler, language)) {
        OUTPUT(("%s", _ERROR("writing to database")));
        return PERFEXPERT_ERROR;
    }

    /* Finalize DWARF library  */
    if (DW_DLV_OK != dwarf_finish(dbg, &err)) {
        OUTPUT(("%s", _ERROR("failed DWARF finalization")));
        return PERFEXPERT_ERROR;
    }

    close(fd);

    return PERFEXPERT_SUCCESS;
}

/* database_write */
static int database_write(const char *compiler, int language) {
    char *error = NULL, query[MAX_BUFFER_SIZE], compilershort[10];

    /* Check if the required table is available */
    char sql[] = "CREATE TABLE IF NOT EXISTS readelf_language ( \
        id             INTEGER PRIMARY KEY,                     \
        name           VARCHAR NOT NULL,                        \
        language       INTEGER,                                 \
        compiler       VARCHAR,                                 \
        compiler_short VARCHAR);";

    OUTPUT_VERBOSE((5, "%s", _BLUE("Writing to database")));

    if (SQLITE_OK != sqlite3_exec(globals.db, sql, NULL, NULL, &error)) {
        OUTPUT(("%s %s", _ERROR("SQL error"), error));
        sqlite3_free(error);
        return PERFEXPERT_ERROR;
    }

    bzero(compilershort, 10);
    if (NULL != strstr(compiler, "GNU C ")) {
        strcpy(compilershort, "gcc");
    }
    if (NULL != strstr(compiler, "GNU C++")) {
        strcpy(compilershort, "g++");
    }
    if (NULL != strstr(compiler, "GNU F")) {
        strcpy(compilershort, "gfortran");
    }
    if (NULL != strstr(compiler, "Intel(R) C ")) {
        strcpy(compilershort, "icc");
    }
    if (NULL != strstr(compiler, "Intel(R) C++")) {
        strcpy(compilershort, "icpc");
    }
    if (NULL != strstr(compiler, "Intel(R) Fortran")) {
        strcpy(compilershort, "ifort");
    }

    bzero(query, MAX_BUFFER_SIZE);
    sprintf(query, "INSERT INTO readelf_language (id, name, language, compiler,"
        " compiler_short) VALUES (%llu, '%s', %d, '%s', '%s');",
         globals.unique_id, globals.program, language, compiler, compilershort);

    if (SQLITE_OK != sqlite3_exec(globals.db, query, NULL, NULL, &error)) {
        OUTPUT(("%s %s %s", _ERROR("SQL error"), error, query));
        sqlite3_free(error);
        return PERFEXPERT_ERROR;
    }

    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
