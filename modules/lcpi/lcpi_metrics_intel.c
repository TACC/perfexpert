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

/* System standard headers */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Utility headers */
#include <matheval.h>

/* Tools headers */
#include "tools/perfexpert/perfexpert_types.h"

/* Modules headers */
#include "lcpi.h"
#include "lcpi_types.h"
#include "lcpi_metrics_intel.h"

/* PerfExpert common headers */
#include "common/perfexpert_alloc.h"
#include "common/perfexpert_constants.h"
#include "common/perfexpert_cpuinfo.h"
#include "common/perfexpert_hash.h"
#include "common/perfexpert_list.h"
#include "common/perfexpert_md5.h"
#include "common/perfexpert_output.h"

int metrics_intel_generate(void) {
    lcpi_metric_t *m = NULL, *temp = NULL;

    /* Check if the architecture is supported */
    if (SANDY_BRIDGE_EP != perfexpert_cpuinfo_get_model()) {
        OUTPUT(("%s", _ERROR("This architecture is not supported yet")));
        OUTPUT(("%s", _ERROR("Please contact fialho@tacc.utexas.edu to help")));
        OUTPUT(("%s", _ERROR("enable support to this processor")));
    }

    OUTPUT_VERBOSE((4, "%s",
        _YELLOW("Generating LCPI metrics (Intel native names)")));

    /* Generate LCPI metrics (be sure the events are ordered) */
    if (PERFEXPERT_SUCCESS != generate_ratio_floating_point()) {
        OUTPUT(("%s", _ERROR("generating ratio floating point")));
        return PERFEXPERT_ERROR;
    }
    if (PERFEXPERT_SUCCESS != generate_ratio_data_accesses()) {
        OUTPUT(("%s", _ERROR("generating ratio data access")));
        return PERFEXPERT_ERROR;
    }
    if (PERFEXPERT_SUCCESS != generate_gflops()) {
        OUTPUT(("%s", _ERROR("generating GFLOPS")));
        return PERFEXPERT_ERROR;
    }
    if (PERFEXPERT_SUCCESS != generate_overall()) {
        OUTPUT(("%s", _ERROR("generating overall")));
        return PERFEXPERT_ERROR;
    }
    if (PERFEXPERT_SUCCESS != generate_data_accesses()) {
        OUTPUT(("%s", _ERROR("generating data access")));
        return PERFEXPERT_ERROR;
    }
    if (PERFEXPERT_SUCCESS != generate_instruction_accesses()) {
        OUTPUT(("%s", _ERROR("generating instruction access")));
        return PERFEXPERT_ERROR;
    }
    if (PERFEXPERT_SUCCESS != generate_tlb_metrics()) {
        OUTPUT(("%s", _ERROR("generating TLB metrics")));
        return PERFEXPERT_ERROR;
    }
    if (PERFEXPERT_SUCCESS != generate_branch_metrics()) {
        OUTPUT(("%s", _ERROR("generating branch metrics")));
        return PERFEXPERT_ERROR;
    }
    if (PERFEXPERT_SUCCESS != generate_floating_point_instr()) {
        OUTPUT(("%s", _ERROR("generating floating point instructions")));
        return PERFEXPERT_ERROR;
    }

    perfexpert_hash_iter_str(my_module_globals.metrics_by_name, m, temp) {
        OUTPUT_VERBOSE((7, "   %s=%s", _CYAN(m->name),
            evaluator_get_string(m->expression)));
    }

    OUTPUT_VERBOSE((4, "(%d) %s",
        perfexpert_hash_count_str(my_module_globals.metrics_by_name),
        _MAGENTA("LCPI metrics")));


    return PERFEXPERT_SUCCESS;
}

/* generate_ratio_floating_point */
static int generate_ratio_floating_point(void) {
    char a[MAX_LCPI];
    lcpi_metric_t *m;

    bzero(a, MAX_LCPI);

    if ((SANDY_BRIDGE_EP == perfexpert_cpuinfo_get_model()) &&
        EVENT_AVAIL("FP_COMP_OPS_EXE.SSE_PACKED_DOUBLE") &&
        EVENT_AVAIL("FP_COMP_OPS_EXE.SSE_SCALAR_SINGLE") &&
        EVENT_AVAIL("FP_COMP_OPS_EXE.SSE_PACKED_SINGLE") &&
        EVENT_AVAIL("FP_COMP_OPS_EXE.SSE_SCALAR_DOUBLE") &&
        EVENT_AVAIL("SIMD_FP_256.PACKED_SINGLE") &&
        EVENT_AVAIL("SIMD_FP_256.PACKED_DOUBLE") &&
        EVENT_AVAIL("FP_COMP_OPS_EXE.X87") &&
        EVENT_AVAIL("INST_RETIRED.ANY")) {

        USE_EVENT("FP_COMP_OPS_EXE.SSE_PACKED_DOUBLE");
        USE_EVENT("FP_COMP_OPS_EXE.SSE_SCALAR_SINGLE");
        USE_EVENT("FP_COMP_OPS_EXE.SSE_PACKED_SINGLE");
        USE_EVENT("FP_COMP_OPS_EXE.SSE_SCALAR_DOUBLE");
        USE_EVENT("SIMD_FP_256.PACKED_SINGLE");
        USE_EVENT("SIMD_FP_256.PACKED_DOUBLE");
        USE_EVENT("FP_COMP_OPS_EXE.X87");
        USE_EVENT("INST_RETIRED.ANY");

        strcpy(a, "(SIMD_FP_256_PACKED_SINGLE + SIMD_FP_256_PACKED_DOUBLE + "
            "FP_COMP_OPS_EXE_X87 + FP_COMP_OPS_EXE_SSE_PACKED_SINGLE + "
            "FP_COMP_OPS_EXE_SSE_PACKED_DOUBLE + "
            "FP_COMP_OPS_EXE_SSE_SCALAR_SINGLE + "
            "FP_COMP_OPS_EXE_SSE_SCALAR_DOUBLE) / INST_RETIRED_ANY");
    } else {
        OUTPUT(("%s", _ERROR("unable to calculate 'ratio.floating_point'")));
        return PERFEXPERT_ERROR;
    }

    PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, m->name, (strlen("ratio.floating_point") + 1));
    strcpy(m->name, "ratio.floating_point");
    strcpy(m->name_md5, perfexpert_md5_string(m->name));
    m->value = 0.0;
    m->expression = evaluator_create(a);
    if (NULL == m->expression) {
        OUTPUT(("%s (%s) [%s]", _ERROR("invalid expression"), m->name, a));
        return PERFEXPERT_ERROR;
    }
    perfexpert_hash_add_str(my_module_globals.metrics_by_name, name_md5, m);

    return PERFEXPERT_SUCCESS;
}

/* generate_ratio_data_accesses */
static int generate_ratio_data_accesses(void) {
    char a[MAX_LCPI];
    lcpi_metric_t *m;

    bzero(a, MAX_LCPI);

    if ((SANDY_BRIDGE_EP == perfexpert_cpuinfo_get_model()) &&
        EVENT_AVAIL("MEM_UOPS_RETIRED.ALL_LOADS") &&
        EVENT_AVAIL("INST_RETIRED.ANY")) {
        USE_EVENT("MEM_UOPS_RETIRED.ALL_LOADS");
        USE_EVENT("INST_RETIRED.ANY");
        strcpy(a, "MEM_UOPS_RETIRED_ALL_LOADS / INST_RETIRED_ANY");
    } else {
        OUTPUT(("%s", _ERROR("unable to calculate 'ratio.data_accesses'")));
        return PERFEXPERT_ERROR;
    }

    PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, m->name, (strlen("ratio.data_accesses") + 1));
    strcpy(m->name, "ratio.data_accesses");
    strcpy(m->name_md5, perfexpert_md5_string(m->name));
    m->value = 0.0;
    m->expression = evaluator_create(a);
    if (NULL == m->expression) {
        OUTPUT(("%s (%s)", _ERROR("invalid expression"), m->name));
        return PERFEXPERT_ERROR;
    }
    perfexpert_hash_add_str(my_module_globals.metrics_by_name, name_md5, m);

    return PERFEXPERT_SUCCESS;
}

/* generate_gflops */
static int generate_gflops(void) {
    char o[MAX_LCPI], s[MAX_LCPI], p[MAX_LCPI];
    lcpi_metric_t *m;

    bzero(o, MAX_LCPI);
    bzero(s, MAX_LCPI);
    bzero(p, MAX_LCPI);

    if ((SANDY_BRIDGE_EP == perfexpert_cpuinfo_get_model()) &&
        EVENT_AVAIL("FP_COMP_OPS_EXE.SSE_PACKED_DOUBLE") &&
        EVENT_AVAIL("FP_COMP_OPS_EXE.SSE_SCALAR_SINGLE") &&
        EVENT_AVAIL("FP_COMP_OPS_EXE.SSE_PACKED_SINGLE") &&
        EVENT_AVAIL("FP_COMP_OPS_EXE.SSE_SCALAR_DOUBLE") &&
        EVENT_AVAIL("SIMD_FP_256.PACKED_SINGLE") &&
        EVENT_AVAIL("SIMD_FP_256.PACKED_DOUBLE") &&
        EVENT_AVAIL("FP_COMP_OPS_EXE.X87") &&
        EVENT_AVAIL("INST_RETIRED.ANY")) {

        USE_EVENT("FP_COMP_OPS_EXE.SSE_PACKED_DOUBLE");
        USE_EVENT("FP_COMP_OPS_EXE.SSE_SCALAR_SINGLE");
        USE_EVENT("FP_COMP_OPS_EXE.SSE_PACKED_SINGLE");
        USE_EVENT("FP_COMP_OPS_EXE.SSE_SCALAR_DOUBLE");
        USE_EVENT("SIMD_FP_256.PACKED_SINGLE");
        USE_EVENT("SIMD_FP_256.PACKED_DOUBLE");
        USE_EVENT("FP_COMP_OPS_EXE.X87");
        USE_EVENT("INST_RETIRED.ANY");

        strcpy(o, "(((SIMD_FP_256_PACKED_SINGLE * 8) + "
            "((SIMD_FP_256_PACKED_DOUBLE + FP_COMP_OPS_EXE_SSE_PACKED_SINGLE) * 4) + "
            "(FP_COMP_OPS_EXE_SSE_FP_PACKED_DOUBLE * 2) + "
            "FP_COMP_OPS_EXE_SSE_FP_SCALAR_SINGLE + "
            "FP_COMP_OPS_EXE_SSE_SCALAR_DOUBLE) / INST_RETIRED_ANY) / 8");

        strcpy(p, "(((SIMD_FP_256_PACKED_SINGLE * 8) + "
            "((SIMD_FP_256_PACKED_DOUBLE + FP_COMP_OPS_EXE_SSE_PACKED_SINGLE) * 4) + "
            "(FP_COMP_OPS_EXE_SSE_FP_PACKED_DOUBLE * 2)) / INST_RETIRED_ANY) / 8");

        strcpy(s, "((FP_COMP_OPS_EXE_SSE_FP_SCALAR_SINGLE + "
            "FP_COMP_OPS_EXE_SSE_SCALAR_DOUBLE) / INST_RETIRED_ANY) / 8");
    } else {
        OUTPUT(("%s", _ERROR("unable to calculate 'GFLOPS_(%%_max)'")));
        return PERFEXPERT_ERROR;
    }

    /* GFLOPS: overall */
    PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, m->name, (strlen("GFLOPS_(%_max).overall") + 1));
    strcpy(m->name, "GFLOPS_(%_max).overall");
    strcpy(m->name_md5, perfexpert_md5_string(m->name));
    m->value = 0.0;
    m->expression = evaluator_create(o);
    if (NULL == m->expression) {
        OUTPUT(("%s (%s)", _ERROR("invalid expression"), m->name));
        return PERFEXPERT_ERROR;
    }
    perfexpert_hash_add_str(my_module_globals.metrics_by_name, name_md5, m);

    /* GFLOPS: scalar */
    PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, m->name, (strlen("GFLOPS_(%_max).scalar") + 1));
    strcpy(m->name, "GFLOPS_(%_max).scalar");
    strcpy(m->name_md5, perfexpert_md5_string(m->name));
    m->value = 0.0;
    m->expression = evaluator_create(s);
    if (NULL == m->expression) {
        OUTPUT(("%s (%s)", _ERROR("invalid expression"), m->name));
        return PERFEXPERT_ERROR;
    }
    perfexpert_hash_add_str(my_module_globals.metrics_by_name, name_md5, m);

    /* GFLOPS: packed */
    PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, m->name, (strlen("GFLOPS_(%_max).packed") + 1));
    strcpy(m->name, "GFLOPS_(%_max).packed");
    strcpy(m->name_md5, perfexpert_md5_string(m->name));
    m->value = 0.0;
    m->expression = evaluator_create(p);
    if (NULL == m->expression) {
        OUTPUT(("%s (%s)", _ERROR("invalid expression"), m->name));
        return PERFEXPERT_ERROR;
    }
    perfexpert_hash_add_str(my_module_globals.metrics_by_name, name_md5, m);

    return PERFEXPERT_SUCCESS;
}

/* generate_overall */
static int generate_overall(void) {
    char a[MAX_LCPI];
    lcpi_metric_t *m;

    bzero(a, MAX_LCPI);

    /* Check the basic ones first */
    if ((SANDY_BRIDGE_EP == perfexpert_cpuinfo_get_model()) &&
        EVENT_AVAIL("CPU_CLK_UNHALTED.THREAD") &&
        EVENT_AVAIL("INST_RETIRED.ANY")) {

        USE_EVENT("CPU_CLK_UNHALTED.THREAD");
        USE_EVENT("INST_RETIRED.ANY");

        strcpy(a, "CPU_CLK_UNHALTED_THREAD / INST_RETIRED_ANY");

    } else {
        OUTPUT(("%s", _ERROR("unable to calculate 'overall'")));
        return PERFEXPERT_ERROR;
    }

    PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, m->name, (strlen("overall") + 1));
    strcpy(m->name, "overall");
    strcpy(m->name_md5, perfexpert_md5_string(m->name));
    m->value = 0.0;
    m->expression = evaluator_create(a);
    if (NULL == m->expression) {
        OUTPUT(("%s (%s)", _ERROR("invalid expression"), m->name));
        return PERFEXPERT_ERROR;
    }
    perfexpert_hash_add_str(my_module_globals.metrics_by_name, name_md5, m);

    return PERFEXPERT_SUCCESS;
}

/* generate_data_accesses */
static int generate_data_accesses(void) {
    char o[MAX_LCPI], L1[MAX_LCPI], L2[MAX_LCPI], L3[MAX_LCPI], miss[MAX_LCPI];
    lcpi_metric_t *m;

    bzero(L1, MAX_LCPI);
    bzero(L2, MAX_LCPI);
    bzero(L3, MAX_LCPI);
    bzero(miss, MAX_LCPI);
    bzero(o, MAX_LCPI);

    if ((SANDY_BRIDGE_EP == perfexpert_cpuinfo_get_model()) &&
        EVENT_AVAIL("INST_RETIRED.ANY") &&
        EVENT_AVAIL("MEM_UOPS_RETIRED.ALL_LOADS") &&
        EVENT_AVAIL("L2_RQSTS.DEMAND_DATA_RD_HIT") &&
        EVENT_AVAIL("LONGEST_LAT_CACHE.REFERENCE") &&
        EVENT_AVAIL("LONGEST_LAT_CACHE.MISS")) {

        USE_EVENT("INST_RETIRED.ANY");
        USE_EVENT("MEM_UOPS_RETIRED.ALL_LOADS");
        USE_EVENT("L2_RQSTS.DEMAND_DATA_RD_HIT");
        USE_EVENT("LONGEST_LAT_CACHE.REFERENCE");
        USE_EVENT("LONGEST_LAT_CACHE.MISS");

        strcpy(L1, "(MEM_UOPS_RETIRED_ALL_LOADS * L1_dlat) / INST_RETIRED_ANY");
        strcpy(L2, "(L2_RQSTS_DEMAND_DATA_RD_HIT * L2_lat) / INST_RETIRED_ANY");
        strcpy(L3, "(LONGEST_LAT_CACHE_REFERENCE * L3_lat) / INST_RETIRED_ANY");
        strcpy(miss, "(LONGEST_LAT_CACHE_MISS * mem_lat) / INST_RETIRED_ANY");
        strcpy(o, "((MEM_UOPS_RETIRED_ALL_LOADS * L1_dlat) + "
            "(L2_RQSTS_DEMAND_DATA_RD_HIT * L2_lat) + "
            "(LONGEST_LAT_CACHE_REFERENCE * L3_lat) + "
            "LONGEST_LAT_CACHE_MISS * mem_lat) / INST_RETIRED_ANY");
    } else {
        OUTPUT(("%s", _ERROR("unable to calculate 'data_accesses'")));
        return PERFEXPERT_ERROR;
    }

    /* Data accesses: overall */
    PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, m->name,
        (strlen("data_accesses.overall") + 1));
    strcpy(m->name, "data_accesses.overall");
    strcpy(m->name_md5, perfexpert_md5_string(m->name));
    m->value = 0.0;
    m->expression = evaluator_create(o);
    if (NULL == m->expression) {
        OUTPUT(("%s (%s)", _ERROR("invalid expression"), m->name));
        return PERFEXPERT_ERROR;
    }
    perfexpert_hash_add_str(my_module_globals.metrics_by_name, name_md5, m);

    /* Data accesses: L1 hits */
    PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, m->name,
        (strlen("data_accesses.L1_cache_hits") + 1));
    strcpy(m->name, "data_accesses.L1_cache_hits");
    strcpy(m->name_md5, perfexpert_md5_string(m->name));
    m->value = 0.0;
    m->expression = evaluator_create(L1);
    if (NULL == m->expression) {
        OUTPUT(("%s (%s)", _ERROR("invalid expression"), m->name));
        return PERFEXPERT_ERROR;
    }
    perfexpert_hash_add_str(my_module_globals.metrics_by_name, name_md5, m);

    /* Data accesses: L2 hits */
    PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, m->name,
        (strlen("data_accesses.L2_cache_hits") + 1));
    strcpy(m->name, "data_accesses.L2_cache_hits");
    strcpy(m->name_md5, perfexpert_md5_string(m->name));
    m->value = 0.0;
    m->expression = evaluator_create(L2);
    if (NULL == m->expression) {
        OUTPUT(("%s (%s)", _ERROR("invalid expression"), m->name));
        return PERFEXPERT_ERROR;
    }
    perfexpert_hash_add_str(my_module_globals.metrics_by_name, name_md5, m);

    /* Data accesses: L3 hits */
    PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, m->name,
        (strlen("data_accesses.L3_cache_hits") + 1));
    strcpy(m->name, "data_accesses.L3_cache_hits");
    strcpy(m->name_md5, perfexpert_md5_string(m->name));
    m->value = 0.0;
    m->expression = evaluator_create(L3);
    if (NULL == m->expression) {
        OUTPUT(("%s (%s)", _ERROR("invalid expression"), m->name));
        return PERFEXPERT_ERROR;
    }
    perfexpert_hash_add_str(my_module_globals.metrics_by_name, name_md5, m);

    /* Data accesses: LLC misses */
    PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, m->name,
        (strlen("data_accesses.LLC_misses_(memory)") + 1));
    strcpy(m->name, "data_accesses.LLC_misses_(memory)");
    strcpy(m->name_md5, perfexpert_md5_string(m->name));
    m->value = 0.0;
    m->expression = evaluator_create(miss);
    if (NULL == m->expression) {
        OUTPUT(("%s (%s)", _ERROR("invalid expression"), m->name));
        return PERFEXPERT_ERROR;
    }
    perfexpert_hash_add_str(my_module_globals.metrics_by_name, name_md5, m);

    return PERFEXPERT_SUCCESS;
}

/* generate_instruction_accesses */
static int generate_instruction_accesses(void) {
    char o[MAX_LCPI], L1[MAX_LCPI], L2[MAX_LCPI], miss[MAX_LCPI];
    lcpi_metric_t *m;

    bzero(L1, MAX_LCPI);
    bzero(L2, MAX_LCPI);
    bzero(miss, MAX_LCPI);
    bzero(o, MAX_LCPI);

    if ((SANDY_BRIDGE_EP == perfexpert_cpuinfo_get_model()) &&
        EVENT_AVAIL("INST_RETIRED.ANY") &&
        EVENT_AVAIL("ICACHE.MISSES") &&
        EVENT_AVAIL("L2_RQSTS.CODE_RD_HIT") &&
        EVENT_AVAIL("L2_RQSTS.CODE_RD_MISS")) {

        USE_EVENT("INST_RETIRED.ANY");
        USE_EVENT("ICACHE.MISSES");
        USE_EVENT("L2_RQSTS.CODE_RD_HIT");
        USE_EVENT("L2_RQSTS.CODE_RD_MISS");

        strcpy(L1, "(ICACHE_MISSES * L1_ilat) / INST_RETIRED_ANY");
        strcpy(L2, "(L2_RQSTS_CODE_RD_HIT * L2_lat) / INST_RETIRED_ANY");
        strcpy(miss, "(L2_RQSTS_CODE_RD_MISS * mem_lat) / INST_RETIRED_ANY");
        strcpy(o, "((ICACHE_MISSES * L1_ilat) + (L2_RQSTS_CODE_RD_HIT * L2_lat)"
            " + (L2_RQSTS_CODE_RD_MISS * mem_lat)) / INST_RETIRED_ANY");
    } else {
        OUTPUT(("%s", _ERROR("unable to calculate 'instruction_accesses'")));
        return PERFEXPERT_ERROR;
    }

    /* Instruction accesses: overall */
    PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, m->name,
        (strlen("instruction_accesses.overall") + 1));
    strcpy(m->name, "instruction_accesses.overall");
    strcpy(m->name_md5, perfexpert_md5_string(m->name));
    m->value = 0.0;
    m->expression = evaluator_create(o);
    if (NULL == m->expression) {
        OUTPUT(("%s (%s)", _ERROR("invalid expression"), m->name));
        return PERFEXPERT_ERROR;
    }
    perfexpert_hash_add_str(my_module_globals.metrics_by_name, name_md5, m);

    /* Instruction accesses: L1 hits */
    PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, m->name,
        (strlen("instruction_accesses.L1_hits") + 1));
    strcpy(m->name, "instruction_accesses.L1_hits");
    strcpy(m->name_md5, perfexpert_md5_string(m->name));
    m->value = 0.0;
    m->expression = evaluator_create(L1);
    if (NULL == m->expression) {
        OUTPUT(("%s (%s)", _ERROR("invalid expression"), m->name));
        return PERFEXPERT_ERROR;
    }
    perfexpert_hash_add_str(my_module_globals.metrics_by_name, name_md5, m);

    /* Instruction accesses: L2 hits */
    PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, m->name,
        (strlen("instruction_accesses.L2_hits") + 1));
    strcpy(m->name, "instruction_accesses.L2_hits");
    strcpy(m->name_md5, perfexpert_md5_string(m->name));
    m->value = 0.0;
    m->expression = evaluator_create(L2);
    if (NULL == m->expression) {
        OUTPUT(("%s (%s)", _ERROR("invalid expression"), m->name));
        return PERFEXPERT_ERROR;
    }
    perfexpert_hash_add_str(my_module_globals.metrics_by_name, name_md5, m);

    /* Instruction accesses: L2 misses */
    PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, m->name,
        (strlen("instruction_accesses.L2_misses") + 1));
    strcpy(m->name, "instruction_accesses.L2_misses");
    strcpy(m->name_md5, perfexpert_md5_string(m->name));
    m->value = 0.0;
    m->expression = evaluator_create(miss);
    if (NULL == m->expression) {
        OUTPUT(("%s (%s)", _ERROR("invalid expression"), m->name));
        return PERFEXPERT_ERROR;
    }
    perfexpert_hash_add_str(my_module_globals.metrics_by_name, name_md5, m);

    return PERFEXPERT_SUCCESS;
}

/* generate_tlb_metrics */
static int generate_tlb_metrics(void) {
    char a[MAX_LCPI], b[MAX_LCPI];
    lcpi_metric_t *m;

    bzero(a, MAX_LCPI);
    bzero(b, MAX_LCPI);

    if ((SANDY_BRIDGE_EP == perfexpert_cpuinfo_get_model()) &&
        EVENT_AVAIL("INST_RETIRED.ANY") &&
        EVENT_AVAIL("DTLB_LOAD_MISSES.WALK_DURATION") &&
        EVENT_AVAIL("ITLB_MISSES.WALK_DURATION")) {

        USE_EVENT("INST_RETIRED.ANY");
        USE_EVENT("DTLB_LOAD_MISSES.WALK_DURATION");
        USE_EVENT("ITLB_MISSES.WALK_DURATION");

        strcpy(a, "DTLB_LOAD_MISSES_WALK_DURATION / INST_RETIRED_ANY");
        strcpy(b, "ITLB_MISSES_WALK_DURATION / INST_RETIRED_ANY");

    } else {
        OUTPUT(("%s", "unable to calculate TLB metrics"));
        return PERFEXPERT_ERROR;
    }

    PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, m->name, (strlen("data_TLB.overall") + 1));
    strcpy(m->name, "data_TLB.overall");
    strcpy(m->name_md5, perfexpert_md5_string(m->name));
    m->value = 0.0;
    m->expression = evaluator_create(a);
    if (NULL == m->expression) {
        OUTPUT(("%s (%s)", _ERROR("invalid expression"), m->name));
        return PERFEXPERT_ERROR;
    }
    perfexpert_hash_add_str(my_module_globals.metrics_by_name, name_md5, m);

    PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, m->name, (strlen("instruction_TLB.overall") + 1));
    strcpy(m->name, "instruction_TLB.overall");
    strcpy(m->name_md5, perfexpert_md5_string(m->name));
    m->value = 0.0;
    m->expression = evaluator_create(b);
    if (NULL == m->expression) {
        OUTPUT(("%s (%s)", _ERROR("invalid expression"), m->name));
        return PERFEXPERT_ERROR;
    }
    perfexpert_hash_add_str(my_module_globals.metrics_by_name, name_md5, m);

    return PERFEXPERT_SUCCESS;
}

/* generate_branch_metrics */
static int generate_branch_metrics(void) {
    char o[MAX_LCPI], a[MAX_LCPI], b[MAX_LCPI];
    lcpi_metric_t *m;

    bzero(o, MAX_LCPI);
    bzero(a, MAX_LCPI);
    bzero(b, MAX_LCPI);

    if ((SANDY_BRIDGE_EP == perfexpert_cpuinfo_get_model()) &&
        EVENT_AVAIL("INST_RETIRED.ANY") &&
        EVENT_AVAIL("BR_INST_RETIRED.ALL_BRANCHES") &&
        EVENT_AVAIL("BR_MISP_RETIRED.ALL_BRANCHES")) {

        USE_EVENT("INST_RETIRED.ANY");
        USE_EVENT("BR_INST_RETIRED.ALL_BRANCHES");
        USE_EVENT("BR_MISP_RETIRED.ALL_BRANCHES");

        strcpy(o, "((BR_INST_RETIRED_ALL_BRANCHES * BR_lat) "
        "+ (BR_MISP_RETIRED_ALL_BRANCHES * BR_miss_lat)) / PAPI_TOT_INS");
        strcpy(a,
            "(BR_INST_RETIRED_ALL_BRANCHES * BR_lat) / INST_RETIRED_ANY");
        strcpy(b,
            "(BR_MISP_RETIRED_ALL_BRANCHES * BR_miss_lat) / INST_RETIRED_ANY");
    } else {
        OUTPUT(("%s", _ERROR("unable to calculate 'branch_instructions'")));
        return PERFEXPERT_ERROR;
    }

    /* Overall */
    PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, m->name,
        (strlen("branch_instructions.overall") + 1));
    strcpy(m->name, "branch_instructions.overall");
    strcpy(m->name_md5, perfexpert_md5_string(m->name));
    m->value = 0.0;
    m->expression = evaluator_create(o);
    if (NULL == m->expression) {
        OUTPUT(("%s (%s)", _ERROR("invalid expression"), m->name));
        return PERFEXPERT_ERROR;
    }
    perfexpert_hash_add_str(my_module_globals.metrics_by_name, name_md5, m);

    /* Correctly predicted */
    PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, m->name,
        (strlen("branch_instructions.correctly_predicted") + 1));
    strcpy(m->name, "branch_instructions.correctly_predicted");
    strcpy(m->name_md5, perfexpert_md5_string(m->name));
    m->value = 0.0;
    m->expression = evaluator_create(a);
    if (NULL == m->expression) {
        OUTPUT(("%s (%s)", _ERROR("invalid expression"), m->name));
        return PERFEXPERT_ERROR;
    }
    perfexpert_hash_add_str(my_module_globals.metrics_by_name, name_md5, m);

    /* Mispredicted */
    PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, m->name,
        (strlen("branch_instructions.mispredicted") + 1));
    strcpy(m->name, "branch_instructions.mispredicted");
    strcpy(m->name_md5, perfexpert_md5_string(m->name));
    m->value = 0.0;
    m->expression = evaluator_create(b);
    if (NULL == m->expression) {
        OUTPUT(("%s (%s)", _ERROR("invalid expression"), m->name));
        return PERFEXPERT_ERROR;
    }
    perfexpert_hash_add_str(my_module_globals.metrics_by_name, name_md5, m);

    return PERFEXPERT_SUCCESS;
}

/* generate_floating_point_instr */
static int generate_floating_point_instr(void) {
    char overall[MAX_LCPI], fast[MAX_LCPI], slow[MAX_LCPI], a[MAX_LCPI];
    lcpi_metric_t *m;

    bzero(a, MAX_LCPI);
    bzero(overall, MAX_LCPI);
    bzero(fast, MAX_LCPI);
    bzero(slow, MAX_LCPI);

    if ((SANDY_BRIDGE_EP == perfexpert_cpuinfo_get_model()) &&
        EVENT_AVAIL("FP_COMP_OPS_EXE.SSE_PACKED_DOUBLE") &&
        EVENT_AVAIL("FP_COMP_OPS_EXE.SSE_SCALAR_SINGLE") &&
        EVENT_AVAIL("FP_COMP_OPS_EXE.SSE_PACKED_SINGLE") &&
        EVENT_AVAIL("FP_COMP_OPS_EXE.SSE_SCALAR_DOUBLE") &&
        EVENT_AVAIL("SIMD_FP_256.PACKED_SINGLE") &&
        EVENT_AVAIL("SIMD_FP_256.PACKED_DOUBLE") &&
        EVENT_AVAIL("FP_COMP_OPS_EXE.X87") &&
        EVENT_AVAIL("ARITH.FPU_DIV") &&
        EVENT_AVAIL("INST_RETIRED.ANY")) {

        USE_EVENT("FP_COMP_OPS_EXE.SSE_PACKED_DOUBLE");
        USE_EVENT("FP_COMP_OPS_EXE.SSE_SCALAR_SINGLE");
        USE_EVENT("FP_COMP_OPS_EXE.SSE_PACKED_SINGLE");
        USE_EVENT("FP_COMP_OPS_EXE.SSE_SCALAR_DOUBLE");
        USE_EVENT("SIMD_FP_256.PACKED_SINGLE");
        USE_EVENT("SIMD_FP_256.PACKED_DOUBLE");
        USE_EVENT("FP_COMP_OPS_EXE.X87");
        USE_EVENT("ARITH.FPU_DIV");
        USE_EVENT("INST_RETIRED.ANY");

        strcpy(a, "(SIMD_FP_256_PACKED_SINGLE + SIMD_FP_256_PACKED_DOUBLE + "
            "FP_COMP_OPS_EXE_X87 + FP_COMP_OPS_EXE_SSE_PACKED_SINGLE + "
            "FP_COMP_OPS_EXE_SSE_PACKED_DOUBLE + "
            "FP_COMP_OPS_EXE_SSE_SCALAR_SINGLE + "
            "FP_COMP_OPS_EXE_SSE_SCALAR_DOUBLE)");
        sprintf(overall, "(%s + ARITH_FPU_DIV) / INST_RETIRED_ANY", a);
        sprintf(fast, "%s / INST_RETIRED_ANY", a);
        strcpy(slow, "ARITH_FPU_DIV / INST_RETIRED_ANY");
    } else {
        OUTPUT(("%s", _ERROR("unable to calculate 'ratio.floating_point'")));
        return PERFEXPERT_ERROR;
    }

    /* Floating-point instructions: overall */
    PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, m->name,
        (strlen("FP_instructions.overall") + 1));
    strcpy(m->name, "FP_instructions.overall");
    strcpy(m->name_md5, perfexpert_md5_string(m->name));
    m->value = 0.0;
    m->expression = evaluator_create(overall);
    if (NULL == m->expression) {
        OUTPUT(("%s (%s)", _ERROR("invalid expression"), m->name));
        return PERFEXPERT_ERROR;
    }
    perfexpert_hash_add_str(my_module_globals.metrics_by_name, name_md5, m);

    /* Floating-point instructions: fast */
    PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, m->name,
        (strlen("FP_instructions.fast_FP_instructions") + 1));
    strcpy(m->name, "FP_instructions.fast_FP_instructions");
    strcpy(m->name_md5, perfexpert_md5_string(m->name));
    m->value = 0.0;
    m->expression = evaluator_create(fast);
    if (NULL == m->expression) {
        OUTPUT(("%s (%s)", _ERROR("invalid expression"), m->name));
        return PERFEXPERT_ERROR;
    }
    perfexpert_hash_add_str(my_module_globals.metrics_by_name, name_md5, m);

    /* Floating-point instructions: slow */
    PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, m->name,
        (strlen("FP_instructions.slow_FP_instructions") + 1));
    strcpy(m->name, "FP_instructions.slow_FP_instructions");
    strcpy(m->name_md5, perfexpert_md5_string(m->name));
    m->value = 0.0;
    m->expression = evaluator_create(slow);
    if (NULL == m->expression) {
        OUTPUT(("%s (%s)", _ERROR("invalid expression"), m->name));
        return PERFEXPERT_ERROR;
    }
    perfexpert_hash_add_str(my_module_globals.metrics_by_name, name_md5, m);

    return PERFEXPERT_SUCCESS;
}

// EOF
