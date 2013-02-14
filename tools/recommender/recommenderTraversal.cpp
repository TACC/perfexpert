/*
 * Copyright (c) 2013  University of Texas at Austin. All rights reserved.
 *
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * This file is part of OptTran and PerfExpert.
 *
 * OptTran as well PerfExpert are free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * OptTran and PerfExpert are distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with OptTran or PerfExpert. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Leonardo Fialho
 *
 * $HEADER$
 */

/* System standard headers */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/* Utility headers */
#include <rose.h>
#include <sage3.h>

/* OptTran headers */
#include "config.h"
#include "recommender.h"
#include "opttran_output.h"
#include "opttran_util.h"
#include "recommenderTraversal.h"

extern globals_t globals; // globals was defined on 'recommender.c'

void recommenderTraversal::visit(SgNode* node) {
    Sg_File_Info &fileInfo = *(node->get_file_info());
    int node_found = 0;
    
    /* extract code fragment for bottlenecks type 'loop' */
    if (((NULL != isSgForStatement(node)) || (NULL != isSgFortranDo(node))) &&
        (0 == strncmp("loop", item->type, 4)) &&
        (fileInfo.get_line() == item->line_number)) {
        /* found a loop on the exact line number */
        node_found = 1;
    }

    /* extract code fragments for bottlenecks type 'function' */
    if ((NULL != isSgFunctionDefinition(node)) &&
        (0 == strncmp("function", item->type, 8)) &&
        (fileInfo.get_line() == item->line_number)) {
        /* found a function on the exact line number */
        node_found = 1;
    }
    
    /* write it to a file */
    if (1 == node_found) {
        char *fragment_file = NULL;
        FILE *fragment_file_FP;
        
        OPTTRAN_OUTPUT_VERBOSE((8, "found a (%s) on (%s:%d)",
                                node->sage_class_name(),
                                fileInfo.get_filename(), fileInfo.get_line()));
        
        fragment_file = (char *)malloc(strlen(globals.opttrandir) +
                                       strlen(OPTTRAN_FRAGMENTS_DIR) +
                                       strlen(item->filename) + 10);
        if (NULL == fragment_file) {
            OPTTRAN_OUTPUT(("%s", _ERROR((char *)"Error: out of memory")));
            exit(OPTTRAN_ERROR);
        }
        bzero(fragment_file, (strlen(globals.opttrandir) +
                              strlen(OPTTRAN_FRAGMENTS_DIR) +
                              strlen(item->filename) + 10));
        sprintf(fragment_file, "%s/%s/%s_%d", globals.opttrandir,
                OPTTRAN_FRAGMENTS_DIR, item->filename, item->line_number);
        OPTTRAN_OUTPUT_VERBOSE((8, "extracting it to (%s)", fragment_file));
        
        fragment_file_FP = fopen(fragment_file, "w+");
        if (NULL == fragment_file_FP) {
            OPTTRAN_OUTPUT(("%s (%s)", _ERROR((char *)"error opening file"),
                            _ERROR(fragment_file)));
            return;
        }
        fprintf(fragment_file_FP, "%s",
                node->unparseToCompleteString().c_str());
        fclose(fragment_file_FP);
        
        /* Clean up */
        free(fragment_file);
    }
}

void recommenderTraversal::atTraversalStart() {
    OPTTRAN_OUTPUT_VERBOSE((9, "%s (%s)",
                            _YELLOW((char *)"starting traversal on"),
                            item->filename));
}

void recommenderTraversal::atTraversalEnd() {
    OPTTRAN_OUTPUT_VERBOSE((9, "%s (%s)",
                            _YELLOW((char *)"ending traversal on"),
                            item->filename));
}

// EOF
