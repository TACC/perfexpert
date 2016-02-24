/*
 * Copyright (c) 2011-2016  University of Texas at Austin. All rights reserved.
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

/* System standard headers */
#include <string.h>

/* Modules headers */
#include "lcpi.h"

/* PerfExpert common headers */
#include "common/perfexpert_constants.h"

int metrics_haswell(void) {
    char s[MAX_LCPI];

    /* Set the events on the measurement module */
    USE_EVENT("ARITH.DIVIDER_UOPS");
    USE_EVENT("BR_INST_RETIRED.ALL_BRANCHES");
    USE_EVENT("BR_INST_RETIRED.ALL_BRANCHES");
    USE_EVENT("BR_MISP_RETIRED.ALL_BRANCHES");
    USE_EVENT("CPU_CLK_UNHALTED.THREAD_P");
    USE_EVENT("DTLB_LOAD_MISSES.WALK_DURATION");
    USE_EVENT("ICACHE.MISSES");
    USE_EVENT("ITLB_MISSES.WALK_DURATION");
    USE_EVENT("L2_RQSTS.DEMAND_DATA_RD_HIT");
    USE_EVENT("L2_RQSTS.CODE_RD_HIT");
    USE_EVENT("L2_RQSTS.CODE_RD_MISS");
    USE_EVENT("LONGEST_LAT_CACHE.MISS");
    USE_EVENT("LONGEST_LAT_CACHE.REFERENCE");
    USE_EVENT("MEM_UOPS_RETIRED.ALL_LOADS");
    USE_EVENT("INST_RETIRED.ANY_P");

    /* Set the profile total cycles and total instructions counters */
    my_module_globals.measurement->total_cycles_counter = "CPU_CLK_UNHALTED.THREAD_P";
    my_module_globals.measurement->total_inst_counter = "INST_RETIRED.ANY_P";

    /* ratio.data_accesses */
    bzero(s, MAX_LCPI);
    strcpy(s, "MEM_UOPS_RETIRED.ALL_LOADS / INST_RETIRED.ANY_P");
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("ratio.data_accesses", s)) {
        return PERFEXPERT_ERROR;
    }

    /* overall */
    bzero(s, MAX_LCPI);
    strcpy(s, "CPU_CLK_UNHALTED.THREAD_P / INST_RETIRED.ANY_P");
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("overall", s)) {
        return PERFEXPERT_ERROR;
    }

    /* data_accesses.overall */
    bzero(s, MAX_LCPI);
    strcpy(s, "((MEM_UOPS_RETIRED.ALL_LOADS * L1_dlat) + "
        "(L2_RQSTS.DEMAND_DATA_RD_HIT * L2_lat) + "
        "(LONGEST_LAT_CACHE.REFERENCE * L3_lat) + "
        "LONGEST_LAT_CACHE.MISS * mem_lat) / INST_RETIRED.ANY_P");
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("data_accesses.overall", s)) {
        return PERFEXPERT_ERROR;
    }

    /* data_accesses.L1_cache_hits */
    bzero(s, MAX_LCPI);
    strcpy(s, "(MEM_UOPS_RETIRED.ALL_LOADS * L1_dlat) / INST_RETIRED.ANY_P");
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("data_accesses.L1_cache_hits", s)) {
        return PERFEXPERT_ERROR;
    }

    /* data_accesses.L2_cache_hits */
    bzero(s, MAX_LCPI);
    strcpy(s, "(L2_RQSTS.DEMAND_DATA_RD_HIT * L2_lat) / INST_RETIRED.ANY_P");
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("data_accesses.L2_cache_hits", s)) {
        return PERFEXPERT_ERROR;
    }

    /* data_accesses.L3_cache_hits */
    bzero(s, MAX_LCPI);
    strcpy(s, "(LONGEST_LAT_CACHE.REFERENCE * L3_lat) / INST_RETIRED.ANY_P");
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("data_accesses.L3_cache_hits", s)) {
        return PERFEXPERT_ERROR;
    }

    /* data_accesses.LLC_misses */
    bzero(s, MAX_LCPI);
    strcpy(s, "(LONGEST_LAT_CACHE.MISS * mem_lat) / INST_RETIRED.ANY_P");
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("data_accesses.LLC_misses", s)) {
        return PERFEXPERT_ERROR;
    }

    /* instruction_accesses.overall */
    bzero(s, MAX_LCPI);
    strcpy(s, "((ICACHE.MISSES * L1_ilat) + (L2_RQSTS.CODE_RD_HIT * L2_lat)"
            " + (L2_RQSTS.CODE_RD_MISS * mem_lat)) / INST_RETIRED.ANY_P");
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("instruction_accesses.overall", s)) {
        return PERFEXPERT_ERROR;
    }

    /* instruction_accesses.L1_hits */
    bzero(s, MAX_LCPI);
    strcpy(s, "(ICACHE.MISSES * L1_ilat) / INST_RETIRED.ANY_P");
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("instruction_accesses.L1_hits", s)) {
        return PERFEXPERT_ERROR;
    }

    /* instruction_accesses.L2_hits */
    bzero(s, MAX_LCPI);
    strcpy(s, "(L2_RQSTS.CODE_RD_HIT * L2_lat) / INST_RETIRED.ANY_P");
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("instruction_accesses.L2_hits", s)) {
        return PERFEXPERT_ERROR;
    }

    /* instruction_accesses.L2_misses */
    bzero(s, MAX_LCPI);
    strcpy(s, "(L2_RQSTS.CODE_RD_MISS * mem_lat) / INST_RETIRED.ANY_P");
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("instruction_accesses.L2_misses", s)) {
        return PERFEXPERT_ERROR;
    }

    /* data_TLB.overall */
    bzero(s, MAX_LCPI);
    strcpy(s, "DTLB_LOAD_MISSES.WALK_DURATION / INST_RETIRED.ANY_P");
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("data_TLB.overall", s)) {
        return PERFEXPERT_ERROR;
    }

    /* instruction_TLB.overall */
    bzero(s, MAX_LCPI);
    strcpy(s, "ITLB_MISSES.WALK_DURATION / INST_RETIRED.ANY_P");
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("instruction_TLB.overall", s)) {
        return PERFEXPERT_ERROR;
    }

    /* branch_instructions.overall */
    bzero(s, MAX_LCPI);
    strcpy(s, "((BR_INST_RETIRED.ALL_BRANCHES * BR_lat) "
        "+ (BR_MISP_RETIRED.ALL_BRANCHES * BR_miss_lat)) / INST_RETIRED.ANY_P");
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("branch_instructions.overall", s)) {
        return PERFEXPERT_ERROR;
    }

    /* branch_instructions.correctly_predicted */
    bzero(s, MAX_LCPI);
    strcpy(s, "(BR_INST_RETIRED.ALL_BRANCHES * BR_lat) / INST_RETIRED.ANY_P");
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("branch_instructions.correctly_predicted", s)) {
        return PERFEXPERT_ERROR;
    }

    /* branch_instructions.mispredicted */
    bzero(s, MAX_LCPI);
    strcpy(s, "(BR_MISP_RETIRED.ALL_BRANCHES * BR_miss_lat) / INST_RETIRED.ANY_P");
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("branch_instructions.mispredicted", s)) {
        return PERFEXPERT_ERROR;
    }

    return PERFEXPERT_SUCCESS;
}


int metrics_haswell_vtune(void) {
    char s[MAX_LCPI];

    /* Set the events on the measurement module */
    USE_EVENT("ARITH.DIVIDER_UOPS");
    USE_EVENT("BR_INST_RETIRED.ALL_BRANCHES");
    USE_EVENT("BR_MISP_RETIRED.ALL_BRANCHES");
    USE_EVENT("CPU_CLK_UNHALTED.THREAD_P");
    USE_EVENT("DTLB_LOAD_MISSES.WALK_DURATION");
    USE_EVENT("FP_COMP_OPS_EXE.SSE_SCALAR_DOUBLE");
    USE_EVENT("FP_COMP_OPS_EXE.X87");
    USE_EVENT("ICACHE.MISSES");
    USE_EVENT("ITLB_MISSES.WALK_DURATION");
    USE_EVENT("L2_RQSTS.DEMAND_DATA_RD_HIT");
    USE_EVENT("L2_RQSTS.CODE_RD_HIT");
    USE_EVENT("L2_RQSTS.CODE_RD_MISS");
    USE_EVENT("LONGEST_LAT_CACHE.MISS");
    USE_EVENT("LONGEST_LAT_CACHE.REFERENCE");
    USE_EVENT("MEM_UOPS_RETIRED.ALL_LOADS");
    USE_EVENT("INST_RETIRED.ANY_P");

    /* Set the profile total cycles and total instructions counters */
    my_module_globals.measurement->total_cycles_counter = "CPU_CLK_UNHALTED.THREAD_P";
    my_module_globals.measurement->total_inst_counter = "INST_RETIRED.ANY_P";

    /* ratio.data_accesses */
    bzero(s, MAX_LCPI);
    strcpy(s, "MEM_UOPS_RETIRED.ALL_LOADS / INST_RETIRED.ANY_P");
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("ratio.data_accesses", s)) {
        return PERFEXPERT_ERROR;
    }

    /* overall */
    bzero(s, MAX_LCPI);
    strcpy(s, "CPU_CLK_UNHALTED.THREAD_P / INST_RETIRED.ANY_P");
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("overall", s)) {
        return PERFEXPERT_ERROR;
    }

    /* data_accesses.overall */
    bzero(s, MAX_LCPI);
    strcpy(s, "((MEM_UOPS_RETIRED.ALL_LOADS * L1_dlat) + "
        "(L2_RQSTS.DEMAND_DATA_RD_HIT * L2_lat) + "
        "(LONGEST_LAT_CACHE.REFERENCE * L3_lat) + "
        "LONGEST_LAT_CACHE.MISS * mem_lat) / INST_RETIRED.ANY_P");
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("data_accesses.overall", s)) {
        return PERFEXPERT_ERROR;
    }

    /* data_accesses.L1_cache_hits */
    bzero(s, MAX_LCPI);
    strcpy(s, "(MEM_UOPS_RETIRED.ALL_LOADS * L1_dlat) / INST_RETIRED.ANY_P");
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("data_accesses.L1_cache_hits", s)) {
        return PERFEXPERT_ERROR;
    }

    /* data_accesses.L2_cache_hits */
    bzero(s, MAX_LCPI);
    strcpy(s, "(L2_RQSTS.DEMAND_DATA_RD_HIT * L2_lat) / INST_RETIRED.ANY_P");
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("data_accesses.L2_cache_hits", s)) {
        return PERFEXPERT_ERROR;
    }

    /* data_accesses.L3_cache_hits */
    bzero(s, MAX_LCPI);
    strcpy(s, "(LONGEST_LAT_CACHE.REFERENCE * L3_lat) / INST_RETIRED.ANY_P");
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("data_accesses.L3_cache_hits", s)) {
        return PERFEXPERT_ERROR;
    }

    /* data_accesses.LLC_misses */
    bzero(s, MAX_LCPI);
    strcpy(s, "(LONGEST_LAT_CACHE.MISS * mem_lat) / INST_RETIRED.ANY_P");
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("data_accesses.LLC_misses", s)) {
        return PERFEXPERT_ERROR;
    }

    /* instruction_accesses.overall */
    bzero(s, MAX_LCPI);
    strcpy(s, "((ICACHE.MISSES * L1_ilat) + (L2_RQSTS.CODE_RD_HIT * L2_lat)"
            " + (L2_RQSTS.CODE_RD_MISS * mem_lat)) / INST_RETIRED.ANY_P");
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("instruction_accesses.overall", s)) {
        return PERFEXPERT_ERROR;
    }

    /* instruction_accesses.L1_hits */
    bzero(s, MAX_LCPI);
    strcpy(s, "(ICACHE.MISSES * L1_ilat) / INST_RETIRED.ANY_P");
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("instruction_accesses.L1_hits", s)) {
        return PERFEXPERT_ERROR;
    }

    /* instruction_accesses.L2_hits */
    bzero(s, MAX_LCPI);
    strcpy(s, "(L2_RQSTS.CODE_RD_HIT * L2_lat) / INST_RETIRED.ANY_P");
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("instruction_accesses.L2_hits", s)) {
        return PERFEXPERT_ERROR;
    }

    /* instruction_accesses.L2_misses */
    bzero(s, MAX_LCPI);
    strcpy(s, "(L2_RQSTS.CODE_RD_MISS * mem_lat) / INST_RETIRED.ANY_P");
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("instruction_accesses.L2_misses", s)) {
        return PERFEXPERT_ERROR;
    }

    /* data_TLB.overall */
    bzero(s, MAX_LCPI);
    strcpy(s, "DTLB_LOAD_MISSES.WALK_DURATION / INST_RETIRED.ANY_P");
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("data_TLB.overall", s)) {
        return PERFEXPERT_ERROR;
    }

    /* instruction_TLB.overall */
    bzero(s, MAX_LCPI);
    strcpy(s, "ITLB_MISSES.WALK_DURATION / INST_RETIRED.ANY_P");
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("instruction_TLB.overall", s)) {
        return PERFEXPERT_ERROR;
    }

    /* branch_instructions.overall */
    bzero(s, MAX_LCPI);
    strcpy(s, "((BR_INST_RETIRED.ALL_BRANCHES * BR_lat) "
        "+ (BR_MISP_RETIRED.ALL_BRANCHES * BR_miss_lat)) / INST_RETIRED.ANY_P");
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("branch_instructions.overall", s)) {
        return PERFEXPERT_ERROR;
    }

    /* branch_instructions.correctly_predicted */
    bzero(s, MAX_LCPI);
    strcpy(s, "(BR_INST_RETIRED.ALL_BRANCHES * BR_lat) / INST_RETIRED.ANY_P");
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("branch_instructions.correctly_predicted", s)) {
        return PERFEXPERT_ERROR;
    }

    /* branch_instructions.mispredicted */
    bzero(s, MAX_LCPI);
    strcpy(s, "(BR_MISP_RETIRED.ALL_BRANCHES * BR_miss_lat) / INST_RETIRED.ANY_P");
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("branch_instructions.mispredicted", s)) {
        return PERFEXPERT_ERROR;
    }

    return PERFEXPERT_SUCCESS;
}

// EOF
