/*
 * Copyright (c) 2013  University of Texas at Austin. All rights reserved.
 *
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * This file is part of PerfExpert.
 *
 * PerfExpert is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * PerfExpert is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with PerfExpert. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Leonardo Fialho
 *
 * $HEADER$
 */

#ifdef __cplusplus
extern "C" {
#endif

/* System standard headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Utility headers */
#include <sqlite3.h>

/* PerfExpert headers */
#include "config.h"
#include "recommender.h"
#include "perfexpert_alloc.h"
#include "perfexpert_database.h"
#include "perfexpert_list.h"
#include "perfexpert_output.h"
#include "perfexpert_string.h"
#include "install_dirs.h"

/* parse_segment_params */
int parse_segment_params(perfexpert_list_t *segments) {
    segment_t *item = NULL;
    char *error_msg = NULL;
    char *temp_str = NULL;
    char buffer[BUFFER_SIZE];
    char sql[BUFFER_SIZE];
    int  input_line = 0;
    int  rowid = 0;

    OUTPUT_VERBOSE((4, "%s", _BLUE("Parsing measurements")));
    
    /* To improve SQLite performance and keep database clean */
    if (SQLITE_OK != sqlite3_exec(globals.db, "BEGIN TRANSACTION;", NULL, NULL,
        &error_msg)) {
        OUTPUT(("%s %s", _ERROR("Error: SQL error"), error_msg));
        sqlite3_free(error_msg);
        return PERFEXPERT_ERROR;
    }

    bzero(buffer, BUFFER_SIZE);
    while (NULL != fgets(buffer, BUFFER_SIZE - 1, globals.inputfile_FP)) {
        node_t *node = NULL;
        int temp = 0;
        
        input_line++;

        /* Ignore comments and blank lines */
        if ((0 == strncmp("#", buffer, 1)) ||
            (strspn(buffer, " \t\r\n") == strlen(buffer))) {
            continue;
        }

        /* Remove the end \n character */
        buffer[strlen(buffer) - 1] = '\0';

        /* Is this line a new code bottleneck specification? */
        if (0 == strncmp("%", buffer, 1)) {
            char temp_str[BUFFER_SIZE];
            
            OUTPUT_VERBOSE((5, "   (%d) %s", input_line,
                _GREEN("new bottleneck found")));

            /* Create a list item for this code bottleneck */
            PERFEXPERT_ALLOC(segment_t, item, sizeof(segment_t));
            perfexpert_list_item_construct((perfexpert_list_item_t *)item);
            
            /* Initialize some elements on segment */
            perfexpert_list_construct((perfexpert_list_t *)&(item->functions));
            item->function_name = NULL;
            item->section_info = NULL;
            item->extra_info = NULL;
            item->filename = NULL;
            item->type = NULL;
            item->rowid = 0;
            item->runtime = 0.0;
            item->loopdepth = 0;
            item->importance = 0.0;
            item->line_number = 0;

            /* Add this item to 'segments' */
            perfexpert_list_append(segments, (perfexpert_list_item_t *)item);

            /* Create the SQL statement for this new segment */
            bzero(temp_str, BUFFER_SIZE);
            sprintf(temp_str,
                "INSERT INTO %s (code_filename) VALUES ('new_code-%d'); ",
                globals.metrics_table, (int)getpid());
            bzero(sql, BUFFER_SIZE);
            strcat(sql, temp_str);
            bzero(temp_str, BUFFER_SIZE);
            sprintf(temp_str,
                "SELECT id FROM %s WHERE code_filename = 'new_code-%d';",
                globals.metrics_table, (int)getpid());
            strcat(sql, temp_str);

            OUTPUT_VERBOSE((5, "     SQL: %s", _CYAN(sql)));
            
            /* Insert new code fragment into metrics database, retrieve id */
            if (SQLITE_OK != sqlite3_exec(globals.db, sql,
                perfexpert_database_get_int, (void *)&rowid, &error_msg)) {
                OUTPUT(("%s %s", _ERROR("Error: SQL error"), error_msg));
                OUTPUT(("   SQL: %s", _CYAN(sql)));
                sqlite3_free(error_msg);
                return PERFEXPERT_ERROR;
            }

            /* Store the rowid on the segment structure */
            OUTPUT_VERBOSE((5, "     ID: %d", rowid));
            item->rowid = rowid;
 
            continue;
        }

        PERFEXPERT_ALLOC(node_t, node, (sizeof(node_t) + strlen(buffer) + 1));
        node->key = strtok(strcpy((char*)(node + 1), buffer), "=\r\n");
        node->value = strtok(NULL, "\r\n");

        /* OK, now it is time to check which parameter is this, and add it to
         * 'segments' (only code.* parameters) other parameter should fit into
         * the SQL DB.
         */

        /* Code param: code.filename */
        if (0 == strncmp("code.filename", node->key, 13)) {
            /* Remove the "./src" string from filename */
            node->value = strstr(node->value, "./src");
            memmove(node->value, node->value + strlen("./src"), 1 +
                strlen(node->value + strlen("./src")));
            PERFEXPERT_ALLOC(char, item->filename, (strlen(node->value) + 1));
            strcpy(item->filename, node->value);
            OUTPUT_VERBOSE((10, "   (%d) %s [%s]", input_line,
                _MAGENTA("filename:"), item->filename));
        }
        /* Code param: code.line_number */
        if (0 == strncmp("code.line_number", node->key, 16)) {
            item->line_number = atoi(node->value);
            OUTPUT_VERBOSE((10, "   (%d) %s [%d]", input_line,
                _MAGENTA("line number:"), item->line_number));
        }
        /* Code param: code.type */
        if (0 == strncmp("code.type", node->key, 9)) {
            PERFEXPERT_ALLOC(char, item->type, (strlen(node->value) + 1));
            strcpy(item->type, node->value);
            OUTPUT_VERBOSE((10, "   (%d) %s [%s]", input_line,
                _MAGENTA("type:"), item->type));
        }
        /* Code param: code.extra_info */
        if (0 == strncmp("code.extra_info", node->key, 15)) {
            PERFEXPERT_ALLOC(char, item->extra_info, (strlen(node->value) + 1));
            strcpy(item->extra_info, node->value);
            OUTPUT_VERBOSE((10, "   (%d) %s [%s]", input_line,
                _MAGENTA("extra info:"), item->extra_info));
        }
        /* Code param: code.importance */
        if (0 == strncmp("code.importance", node->key, 15)) {
            item->importance = atof(node->value);
            OUTPUT_VERBOSE((10, "   (%d) %s [%f], ", input_line,
                _MAGENTA("importance:"), item->importance));
        }
        /* Code param: code.section_info */
        if (0 == strncmp("code.section_info", node->key, 17)) {
            PERFEXPERT_ALLOC(char, item->section_info,
                (strlen(node->value) + 1));
            strcpy(item->section_info, node->value);
            OUTPUT_VERBOSE((10, "   (%d) %s [%s]", input_line,
                _MAGENTA("section info:"), item->section_info));
            PERFEXPERT_DEALLOC(node);
            continue;
        }
        /* Code param: code.function_name */
        if (0 == strncmp("code.function_name", node->key, 18)) {
            /* Remove everyting after the '.' (for OMP functions) */
            temp_str = node->value;
            strsep(&temp_str, ".");
            PERFEXPERT_ALLOC(char, item->function_name,
                (strlen(node->value) + 1));
            strcpy(item->function_name, node->value);
            OUTPUT_VERBOSE((10, "   (%d) %s [%s]", input_line,
                _MAGENTA("function name:"), item->function_name));
        }
        /* Code param: code.runtime */
        if (0 == strncmp("code.runtime", node->key, 12)) {
            item->runtime = atof(node->value);
            OUTPUT_VERBOSE((10, "   (%d) %s [%f], ", input_line,
                _MAGENTA("runtime:"), item->runtime));
        }
        /* Code param: code.loopdepth */
        if (0 == strncmp("code.loopdepth", node->key, 21)) {
            item->loopdepth = atoi(node->value);
            OUTPUT_VERBOSE((10, "   (%d) %s [%d]", input_line,
                _MAGENTA("loop depth:"), item->loopdepth));
        }

        /* Clean the node->key (remove undesired characters) */
        perfexpert_string_replace_char(node->key, '%', '_');
        perfexpert_string_replace_char(node->key, '.', '_');
        perfexpert_string_replace_char(node->key, '(', '_');
        perfexpert_string_replace_char(node->key, ')', '_');
        perfexpert_string_replace_char(node->key, '-', '_');
        perfexpert_string_replace_char(node->key, ':', '_');

        /* Assemble the SQL query */
        bzero(sql, BUFFER_SIZE);
        sprintf(sql, "UPDATE %s SET %s='%s' WHERE id=%d;",
            globals.metrics_table, node->key, node->value, rowid);

        /* Update metrics table */
        if (SQLITE_OK != sqlite3_exec(globals.db, sql, NULL, NULL,
            &error_msg)) {
            OUTPUT_VERBOSE((4, "   (%d) %s (%s = %s)", input_line,
                _RED("ignored line"), node->key, node->value));
            sqlite3_free(error_msg);
        } else {
            OUTPUT_VERBOSE((10, "   (%d) %s", input_line, sql));
        }
        PERFEXPERT_DEALLOC(node);
    }

    /* LOG this bottleneck */
    bzero(sql, BUFFER_SIZE);
    sprintf(sql, "SELECT * FROM %s WHERE id=%d;", globals.metrics_table, rowid);

    if (SQLITE_OK != sqlite3_exec(globals.db, sql, log_bottleneck, NULL,
        &error_msg)) {
        OUTPUT(("%s %s", _ERROR("Error: SQL error"), error_msg));
        sqlite3_free(error_msg);
        return PERFEXPERT_ERROR;
    }

    /* To improve SQLite performance and keep database clean */
    if (SQLITE_OK != sqlite3_exec(globals.db, "END TRANSACTION;", NULL, NULL,
        &error_msg)) {
        OUTPUT(("%s %s", _ERROR("Error: SQL error"), error_msg));
        sqlite3_free(error_msg);
        return PERFEXPERT_ERROR;
    }

    /* print a summary of 'segments' */
    OUTPUT_VERBOSE((4, "   (%d) %s", perfexpert_list_get_size(segments),
        _MAGENTA("code segment(s) found")));

    item = (segment_t *)perfexpert_list_get_first(segments);
    while ((perfexpert_list_item_t *)item != &(segments->sentinel)) {
        OUTPUT_VERBOSE((4, "     %s (line %d)", _YELLOW(item->filename),
            item->line_number));
        item = (segment_t *)perfexpert_list_get_next(item);
    }
    
    return PERFEXPERT_SUCCESS;
}

/* parse_metrics_file */
int parse_metrics_file(void) {
    char buffer[BUFFER_SIZE];
    char sql[BUFFER_SIZE];
    char *error_msg = NULL;
    FILE *metrics_FP;

    if (NULL == globals.metrics_file) {
        PERFEXPERT_ALLOC(char, globals.metrics_file,
            (strlen(METRICS_FILE) + strlen(PERFEXPERT_ETCDIR) + 2));
        sprintf(globals.metrics_file, "%s/%s", PERFEXPERT_ETCDIR, METRICS_FILE);
    }

    OUTPUT_VERBOSE((7, "=== %s (%s)", _BLUE("Reading metrics file"),
        globals.metrics_file));

    if (NULL == (metrics_FP = fopen(globals.metrics_file, "r"))) {
        OUTPUT(("%s (%s)", _ERROR("Error: unable to open metrics file"),
            globals.metrics_file));
        return PERFEXPERT_ERROR;
    }

    bzero(sql, BUFFER_SIZE);
    sprintf(sql, "CREATE TEMP TABLE %s ( ", globals.metrics_table);
    strcat(sql, "id INTEGER PRIMARY KEY, code_filename CHAR( 1024 ), ");
    strcat(sql, "code_line_number INTEGER, code_type CHAR( 128 ), ");
    strcat(sql, "code_extra_info CHAR( 1024 ), ");

    bzero(buffer, BUFFER_SIZE);
    while (NULL != fgets(buffer, BUFFER_SIZE - 1, metrics_FP)) {
        int temp;
            
        /* Ignore comments and blank lines */
        if ((0 == strncmp("#", buffer, 1)) ||
            (strspn(buffer, " \t\r\n") == strlen(buffer))) {
            continue;
        }

        /* Remove the end \n character */
        buffer[strlen(buffer) - 1] = '\0';

        /* replace some characters just to provide a safe SQL clause */
        perfexpert_string_replace_char(buffer, '%', '_');
        perfexpert_string_replace_char(buffer, '.', '_');
        perfexpert_string_replace_char(buffer, '(', '_');
        perfexpert_string_replace_char(buffer, ')', '_');
        perfexpert_string_replace_char(buffer, '-', '_');
        perfexpert_string_replace_char(buffer, ':', '_');

        strcat(sql, buffer);
        strcat(sql, " FLOAT, ");
    }
    sql[strlen(sql)-2] = '\0'; // remove the last ',' and '\n'
    strcat(sql, ");");
    OUTPUT_VERBOSE((10, "metrics SQL: %s", _CYAN(sql)));

    /* Create metrics table */
    if (SQLITE_OK != sqlite3_exec(globals.db, sql, NULL, NULL, &error_msg)) {
        OUTPUT(("%s %s", _ERROR("Error: SQL error"), error_msg));
        OUTPUT(("   SQL: %s", _CYAN(sql)));
        sqlite3_free(error_msg);
        return PERFEXPERT_ERROR;
    }

    OUTPUT(("using temporary metric table (%s)", globals.metrics_table));
    PERFEXPERT_DEALLOC(globals.metrics_file);

    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
