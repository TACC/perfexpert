/*
 * Copyright (c) 2013  University of Texas at Austin. All rights reserved.
 *
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

#ifndef RECOMMENDER_H
#define RECOMMENDER_H

#ifndef OPTTRAN_CONSTANTS_H
#include "opttran_constants.h"
#endif

#ifndef OPTTRAN_LIST_H
#include "opttran_list.h"
#endif

#ifndef _GETOPT_H_
#include <getopt.h> /* To parse command line arguments */
#endif

#ifndef	_CTYPE_H_
#include <ctype.h>
#endif

/** Structure to hold global variables */
typedef struct {
    int  verbose;
    int  verbose_level;
    int  use_stdin;
    int  use_stdout;
    int  use_opttran;
    char *inputfile;
    char *outputfile;
    char *dbfile;
    char *opttrandir;
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
    {0, 0, 0, 0}
};

/** Structure to help STDIN parsing */
typedef struct node {
    char *key;
    char *value;
} node_t;

/** Structure to hold performance measurements (PerfExpert) */
typedef struct measurements {
    double overall;
    double data_accesses_overall;
    double data_accesses_L1d_hits;
    double data_accesses_L2d_hits;
    double data_accesses_L2d_misses;
    double data_accesses_L3d_misses;
    double ratio_floating_point;
    double ratio_data_accesses;
    double instruction_accesses_overall;
    double instruction_accesses_L1i_hits;
    double instruction_accesses_L2i_hits;
    double instruction_accesses_L2i_misses;
    double instruction_TLB_overall;
    double data_TLB_overall;
    double branch_instructions_overall;
    double branch_instructions_correctly_predicted;
    double branch_instructions_mispredicted;
    double floating_point_instr_overall;
    double floating_point_instr_fast_FP_instr;
    double floating_point_instr_slow_FP_instr;
    double percent_GFLOPS_max_overall;
    double percent_GFLOPS_max_packed;
    double percent_GFLOPS_max_scalar;
}  measurements_t;

/** Structure to hold code segments */
typedef struct segment {
    volatile struct opttran_list_item_t *next; /** Pointer to next list item     */
    volatile struct opttran_list_item_t *prev; /** Pointer to previous list item */
    char   *filename;
    int    line_number;
    char   *type;
    char   *extra_info;
    char   *section_info;
    double representativeness;
    struct measurements data;
} segment_t;

/* Function definitions */
static void show_help(void);
static int  parse_env_vars(void);
static int  parse_cli_params(int argc, char *argv[]);
static int  parse_segment_params(opttran_list_t *segments_p, FILE *inputfile_p);
static int  output_recommendations(void *NotUsed, int argc, char **argv,
                                   char **azColName);
static int  query_database(void);
static int  calculate_weigths(void);
static int  select_recommendations(void);

#endif /* RECOMMENDER_H */
