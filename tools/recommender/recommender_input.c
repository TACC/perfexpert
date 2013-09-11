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
#include "perfexpert_list.h"
#include "perfexpert_output.h"
#include "perfexpert_database.h"
#include "install_dirs.h"

/* parse_segment_params */
int parse_segment_params(perfexpert_list_t *segments_p) {
    segment_t *item;
    int  input_line = 0;
    char buffer[BUFFER_SIZE];
    char sql[BUFFER_SIZE];
    char *error_msg = NULL;
    int  rowid = 0;
    char *temp_str = NULL;

    OUTPUT_VERBOSE((4, "=== %s", _BLUE("Parsing measurements")));
    
    /* Which INPUT we are using? (just a double check) */
    if (NULL == globals.inputfile) {
        OUTPUT_VERBOSE((3, "using STDIN as input for perf measurements"));
    } else {
        OUTPUT_VERBOSE((3, "using (%s) as input for perf measurements",
                        globals.inputfile));
    }

    /* To improve SQLite performance and keep database clean */
    if (SQLITE_OK != sqlite3_exec(globals.db, "BEGIN TRANSACTION;", NULL, NULL,
                                  &error_msg)) {
        fprintf(stderr, "Error: SQL error: %s\n", error_msg);
        sqlite3_free(error_msg);
        return PERFEXPERT_ERROR;
    }

    bzero(buffer, BUFFER_SIZE);
    while (NULL != fgets(buffer, BUFFER_SIZE - 1, globals.inputfile_FP)) {
        node_t *node;
        int temp;
        
        input_line++;

        /* Ignore comments */
        if (0 == strncmp("#", buffer, 1)) {
            continue;
        }

        /* Is this line a new code bottleneck specification? */
        if (0 == strncmp("%", buffer, 1)) {
            char temp_str[BUFFER_SIZE];
            
            OUTPUT_VERBOSE((5, "   (%d) --- %s", input_line,
                            _GREEN("new bottleneck found")));

            /* Create a list item for this code bottleneck */
            item = (segment_t *)malloc(sizeof(segment_t));
            if (NULL == item) {
                OUTPUT(("%s", _ERROR("Error: out of memory")));
                return PERFEXPERT_ERROR;
            }
            perfexpert_list_item_construct((perfexpert_list_item_t *) item);
            
            /* Initialize some elements on segment */
            item->filename = NULL;
            item->line_number = 0;
            item->type = NULL;
            item->extra_info = NULL;
            item->section_info = NULL;
            item->loop_depth = 0;
            item->representativeness = 0;
            item->rowid = 0;
            item->function_name  = NULL;

            /* Add this item to 'segments' */
            perfexpert_list_append(segments_p, (perfexpert_list_item_t *) item);

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

            OUTPUT_VERBOSE((5, "        SQL: %s", _CYAN(sql)));
            
            /* Insert new code fragment into metrics database, retrieve id */
            if (SQLITE_OK != sqlite3_exec(globals.db, sql,
                perfexpert_database_get_int, (void *)&rowid, &error_msg)) {
                fprintf(stderr, "Error: SQL error: %s\n", error_msg);
                fprintf(stderr, "SQL clause: %s\n", sql);
                sqlite3_free(error_msg);
                return PERFEXPERT_ERROR;
            } else {
                OUTPUT_VERBOSE((5, "        ID: %d", rowid));
                /* Store the rowid on the segment structure */
                item->rowid = rowid;
            }

            continue;
        }

        node = (node_t *)malloc(sizeof(node_t) + strlen(buffer) + 1);
        if (NULL == node) {
            OUTPUT(("%s", _ERROR("Error: out of memory")));
            return PERFEXPERT_ERROR;
        }
        bzero(node, sizeof(node_t) + strlen(buffer) + 1);
        node->key = strtok(strcpy((char*)(node + 1), buffer), "=\r\n");
        node->value = strtok(NULL, "\r\n");

        /* OK, now it is time to check which parameter is this, and add it to
         * 'segments' (only code.* parameters) other parameter should fit into
         * the SQL DB.
         */

        /* Code param: code.filename */
        if (0 == strncmp("code.filename", node->key, 13)) {
            item->filename = (char *)malloc(strlen(node->value) + 1);
            if (NULL == item->filename) {
                OUTPUT(("%s", _ERROR("Error: out of memory")));
                return PERFEXPERT_ERROR;
            }
            /* Remove the "./src" string from filename */
            node->value = strstr(node->value, "./src");
            memmove(node->value, node->value + strlen("./src"),
                    1 + strlen(node->value + strlen("./src")));

            bzero(item->filename, strlen(node->value) + 1);
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
            item->type = (char *)malloc(strlen(node->value) + 1);
            if (NULL == item->type) {
                OUTPUT(("%s", _ERROR("Error: out of memory")));
                return PERFEXPERT_ERROR;
            }
            bzero(item->type, strlen(node->value) + 1);
            strcpy(item->type, node->value);
            OUTPUT_VERBOSE((10, "   (%d) %s [%s]", input_line,
                            _MAGENTA("type:"), item->type));
        }
        /* Code param: code.extra_info */
        if (0 == strncmp("code.extra_info", node->key, 15)) {
            item->extra_info = (char *)malloc(strlen(node->value) + 1);
            if (NULL == item->extra_info) {
                OUTPUT(("%s", _ERROR("Error: out of memory")));
                return PERFEXPERT_ERROR;
            }
            bzero(item->extra_info, strlen(node->value) + 1);
            strcpy(item->extra_info, node->value);
            OUTPUT_VERBOSE((10, "   (%d) %s [%s]", input_line,
                            _MAGENTA("extra info:"), item->extra_info));
        }
        /* Code param: code.representativeness */
        if (0 == strncmp("code.representativeness", node->key, 23)) {
            item->representativeness = atof(node->value);
            OUTPUT_VERBOSE((10, "   (%d) %s [%f], ", input_line,
                            _MAGENTA("representativeness:"),
                            item->representativeness));
        }
        /* Code param: code.section_info */
        if (0 == strncmp("code.section_info", node->key, 17)) {
            item->section_info = (char *)malloc(strlen(node->value) + 1);
            if (NULL == item->section_info) {
                OUTPUT(("%s", _ERROR("Error: out of memory")));
                return PERFEXPERT_ERROR;
            }
            bzero(item->section_info, strlen(node->value) + 1);
            strcpy(item->section_info, node->value);
            OUTPUT_VERBOSE((10, "   (%d) %s [%s]", input_line,
                            _MAGENTA("section info:"), item->section_info));
            free(node);
            continue;
        }
        /* Code param: code.function_name */
        if (0 == strncmp("code.function_name", node->key, 18)) {
            /* Remove everyting after the '.' (for OMP functions) */
            temp_str = node->value;
            strsep(&temp_str, ".");

            item->function_name = (char *)malloc(strlen(node->value) + 1);
            if (NULL == item->function_name) {
                OUTPUT(("%s", _ERROR("Error: out of memory")));
                return PERFEXPERT_ERROR;
            }
            bzero(item->function_name, strlen(node->value) + 1);
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
        /* Code param: perfexpert.loop_depth */
        if (0 == strncmp("perfexpert.loop-depth", node->key, 21)) {
            item->loop_depth = atof(node->value);
            OUTPUT_VERBOSE((10, "   (%d) %s [%f]", input_line,
                            _MAGENTA("loop depth:"), item->loop_depth));
        }

        /* Clean the node->key (remove undesired characters) */
        for (temp = 0; temp < strlen(node->key); temp++) {
            switch (node->key[temp]) {
                case '%':
                case '.':
                case '(':
                case ')':
                case '-':
                case ':':
                    node->key[temp] = '_';
                default:
                    break;
            }
        }
        
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
        free(node);
    }

    /* LOG this bottleneck */
    bzero(sql, BUFFER_SIZE);
    sprintf(sql, "SELECT * FROM %s WHERE id=%d;", globals.metrics_table, rowid);

    if (SQLITE_OK != sqlite3_exec(globals.db, sql, log_bottleneck, NULL,
                                  &error_msg)) {
        fprintf(stderr, "Error: SQL error: %s\n", error_msg);
        sqlite3_free(error_msg);
        return PERFEXPERT_ERROR;
    }


    /* To improve SQLite performance and keep database clean */
    if (SQLITE_OK != sqlite3_exec(globals.db, "END TRANSACTION;", NULL, NULL,
                                  &error_msg)) {
        fprintf(stderr, "Error: SQL error: %s\n", error_msg);
        sqlite3_free(error_msg);
        return PERFEXPERT_ERROR;
    }

    /* print a summary of 'segments' */
    OUTPUT_VERBOSE((4, "%d %s", perfexpert_list_get_size(segments_p),
                    _GREEN("code segment(s) found")));

    item = (segment_t *)perfexpert_list_get_first(segments_p);
    while ((perfexpert_list_item_t *)item != &(segments_p->sentinel)) {
        OUTPUT_VERBOSE((4, "   %s:%d", item->filename, item->line_number));
        item = (segment_t *)perfexpert_list_get_next(item);
    }
    
    /* TODO: Free memory */

    return PERFEXPERT_SUCCESS;
}

/* parse_metrics_file */
int parse_metrics_file(void) {
    FILE *metrics_FP;
    char buffer[BUFFER_SIZE];
    char sql[BUFFER_SIZE];
    char *error_msg = NULL;

    if (NULL == globals.metrics_file) {
        globals.metrics_file = (char *)malloc(strlen(METRICS_FILE) +
                                              strlen(PERFEXPERT_ETCDIR) + 2);
        if (NULL == globals.metrics_file) {
            OUTPUT(("%s", _ERROR("Error: out of memory")));
            return PERFEXPERT_ERROR;
        }
        bzero(globals.metrics_file, strlen(METRICS_FILE) +
              strlen(PERFEXPERT_ETCDIR) + 2);
        sprintf(globals.metrics_file, "%s/%s", PERFEXPERT_ETCDIR, METRICS_FILE);
    }

    OUTPUT_VERBOSE((7, "=== %s (%s)", _BLUE("Reading metrics file"),
                    globals.metrics_file));

    if (NULL == (metrics_FP = fopen(globals.metrics_file, "r"))) {
        OUTPUT(("%s (%s)", _ERROR("Error: unable to open metrics file"),
                globals.metrics_file));
        return PERFEXPERT_ERROR;
    } else {
        bzero(sql, BUFFER_SIZE);
        sprintf(sql, "CREATE TEMP TABLE %s ( ", globals.metrics_table);
        strcat(sql, "id INTEGER PRIMARY KEY, code_filename CHAR( 1024 ), ");
        strcat(sql, "code_line_number INTEGER, code_type CHAR( 128 ), ");
        strcat(sql, "code_extra_info CHAR( 1024 ), ");

        bzero(buffer, BUFFER_SIZE);
        while (NULL != fgets(buffer, BUFFER_SIZE - 1, metrics_FP)) {
            int temp;
            
            /* avoid comments */
            if ('#' == buffer[0]) {
                continue;
            }
            /* avoid blank and too short lines */
            if (10 >= strlen(buffer)) {
                continue;
            }
            /* replace some characters just to provide a safe SQL clause */
            for (temp = 0; temp < strlen(buffer); temp++) {
                switch (buffer[temp]) {
                    case '%':
                    case '.':
                    case '(':
                    case ')':
                    case '-':
                    case ':':
                        buffer[temp] = '_';
                    default:
                        break;
                }
            }
            buffer[strcspn(buffer, "\n")] = '\0'; // remove the '\n'
            strcat(sql, buffer);
            strcat(sql, " FLOAT, ");
        }
        sql[strlen(sql)-2] = '\0'; // remove the last ',' and '\n'
        strcat(sql, ");");
        OUTPUT_VERBOSE((10, "metrics SQL: %s", _CYAN(sql)));

        /* Create metrics table */
        if (SQLITE_OK != sqlite3_exec(globals.db, sql, NULL, NULL,
                                      &error_msg)) {
            fprintf(stderr, "Error: SQL error: %s\n", error_msg);
            fprintf(stderr, "SQL clause: %s\n", sql);
            sqlite3_free(error_msg);
            return PERFEXPERT_ERROR;
        }
        OUTPUT(("using temporary metric table (%s)", globals.metrics_table));
    }

    /* TODO: Free memory */

    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
