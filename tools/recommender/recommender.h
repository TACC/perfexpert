/*
 * Copyright (c) 2013  University of Texas at Austin. All rights reserved.
 *
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

#ifndef RECOMMENDER_H_
#define RECOMMENDER_H_

#ifndef OPTTRAN_CONSTANTS_H_
#include "opttran_constants.h"
#endif

#ifndef OPTTRAN_LIST_H_
#include "opttran_list.h"
#endif

#ifndef _GETOPT_H_
#include <getopt.h> /* To parse command line arguments */
#endif

#ifndef	_CTYPE_H_
#include <ctype.h>
#endif

#ifndef	_STDIO_H_
#include <stdio.h> /* To use FILE type on globals */
#endif

/** Buffers size, will be used for:
 * - parsing INPUT file
 * - parsing metrics file
 * - storing SQL statements
 * - maybe something else
 */
#define BUFFER_SIZE 4096

/** Default values for some parameters */
// TODO: these default files have to be replaced by the correct install dir path
#define RECOMMENDATION_DB "../contrib/recommendation.db"
#define METRICS_FILE      "../contrib/recommender-metrics.txt"
#define OPTTRAN_RECO_FILE "recommendations.txt"

/** Structure to hold global variables */
typedef struct {
    int  verbose;
    int  verbose_level;
    int  use_stdin;
    int  use_stdout;
    int  use_opttran;
    char *inputfile;
    char *outputfile;
    FILE *outputfile_FP;
    char *dbfile;
    sqlite3 *db;
    char *opttrandir;
    char *metrics_file;
    int  use_temp_metrics;
    char *metrics_table;
    int  colorful;
} globals_t;

extern globals_t globals; /**< Variable to hold global options */

/* WARNING: to include opttran_output.h globals have to be defined firts */
#ifndef OPTTRAN_OUTPUT_H
#include "opttran_output.h"
#endif

/** Structure to handle command line arguments. Try to keep the content of
 *  this structure compatible with the parse_cli_params() and show_help().
 */
static struct option long_options[] = {
    {"verbose_level", required_argument, NULL, 'l'},
    {"stdin",         no_argument,       NULL, 'i'},
    {"inputfile",     required_argument, NULL, 'f'},
    {"help",          no_argument,       NULL, 'h'},
    {"database",      required_argument, NULL, 'd'},
    {"outputfile",    required_argument, NULL, 'o'},
    {"opttran",       required_argument, NULL, 'a'},
    {"metricfile",    required_argument, NULL, 'm'},
    {"newmetrics",    no_argument,       NULL, 'n'},
    {"colorful",      no_argument,       NULL, 'c'},
    {0, 0, 0, 0}
};

/** Structure to help STDIN parsing */
typedef struct node {
    char *key;
    char *value;
} node_t;

/** Structure to hold code segments */
typedef struct segment {
    volatile struct opttran_list_item_t *next; /** Pointer to next list item */
    volatile struct opttran_list_item_t *prev; /** Pointer to previous list item */
    char   *filename;
    int    line_number;
    char   *type;
    char   *extra_info;
    char   *section_info;
    double representativeness;
} segment_t;

#endif /* RECOMMENDER_H */
