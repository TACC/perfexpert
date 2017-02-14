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
 * Authors: Antonio Gomez-Iglesias, Leonardo Fialho and Ashay Rane
 *
 * $HEADER$
 */

/* System standard headers */
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

/* Modules headers */
#include "lcpi.h"

/* PerfExpert common headers */
#include "common/perfexpert_constants.h"


int is_flat_mode(void) {
    char output[10];
    // On KNL, memkind-hbw-nodes should be present
    FILE *fp = popen("memkind-hbw-nodes", "r");
    if (fp == NULL)
        return PERFEXPERT_ERROR;
    fgets(output, 10, fp);
    int val = strtol (output, NULL, 10);
    return val;
}


int metrics_knl(void) {
    char s[MAX_LCPI];

    OUTPUT(("HPCToolkit still not supported for KNL"));
    return PERFEXPERT_ERROR;
}


int metrics_knl_vtune(void) {
    char s[MAX_LCPI];

    /* Set the events on the measurement module */
    USE_EVENT("INST_RETIRED.ANY_P");
    USE_EVENT("BR_INST_RETIRED.ALL_BRANCHES");
    USE_EVENT("BR_MISP_RETIRED.ALL_BRANCHES");
    USE_EVENT("CPU_CLK_UNHALTED.THREAD_P");
    USE_EVENT("PAGE_WALKS.WALKS"); // This can be used to measure TLB misses
    USE_EVENT("PAGE_WALKS.D_SIDE_CYCLES");
    USE_EVENT("PAGE_WALKS.I_SIDE_CYCLES");
    USE_EVENT("ICACHE.MISSES");
    USE_EVENT("LONGEST_LAT_CACHE.MISS");
    USE_EVENT("LONGEST_LAT_CACHE.REFERENCE");
    USE_EVENT("MEM_UOPS_RETIRED.ALL_LOADS");
    USE_EVENT("MEM_UOPS_RETIRED.L2_MISS_LOADS_PS");
    USE_EVENT("UOPS_RETIRED.ALL"); 
    USE_EVENT("UOPS_RETIRED.SCALAR_SIMD");
    USE_EVENT("UOPS_RETIRED.PACKED_SIMD");

//    USE_EVENT("NO_ALLOC_CYCLES.ALL"); //Is this one OK?
    USE_EVENT("UNC_E_RPQ_INSERTS");
    USE_EVENT("UNC_E_WPQ_INSERTS");
    USE_EVENT("UNC_E_EDC_ACCESS.HIT_CLEAN");
    USE_EVENT("UNC_E_EDC_ACCESS.HIT_DIRTY");
    USE_EVENT("UNC_E_EDC_ACCESS.MISS_CLEAN");
    USE_EVENT("UNC_E_EDC_ACCESS.MISS_DIRTY");

    /* Set the profile total cycles and total instructions counters */
    my_module_globals.measurement->total_cycles_counter = "CPU_CLK_UNHALTED.THREAD_P";
    my_module_globals.measurement->total_inst_counter = "INST_RETIRED.ANY_P";

    /* ratio.data_accesses */
    bzero(s, MAX_LCPI);
    strcpy(s, "MEM_UOPS_RETIRED.ALL_LOADS / INST_RETIRED.ANY_P");
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("ratio.data_accesses", s)) {
        return PERFEXPERT_ERROR;
    }


    bzero(s, MAX_LCPI);
    strcpy(s, "(UOPS_RETIRED.SCALAR_SIMD+UOPS_RETIRED.PACKED_SIMD) / UOPS_RETIRED.ALL");
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("SIMD overall",s)) {
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
//        "(LONGEST_LAT_CACHE.REFERENCE * L3_lat) + "
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
    strcpy(s, "(LONGEST_LAT_CACHE.REFERENCE * L2_lat) / INST_RETIRED.ANY_P");
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("data_accesses.L2_cache_hits", s)) {
        return PERFEXPERT_ERROR;
    }

    /* data_accesses.LLC_misses */
    bzero(s, MAX_LCPI);
    strcpy(s, "(LONGEST_LAT_CACHE.MISS * mem_lat) / INST_RETIRED.ANY_P");
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("data_accesses.LLC_misses", s)) {
        return PERFEXPERT_ERROR;
    }


    bzero(s, MAX_LCPI);
    strcpy(s, "(ICACHE.MISSES * L1_ilat) / INST_RETIRED.ANY_P");
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("instruction_accesses.L1_hits", s)) {
        return PERFEXPERT_ERROR;
    }

    bzero(s, MAX_LCPI);
    strcpy(s, "FETCH_STALL.ICACHE_FILL_PENDING_CYCLES / CPU_CLK_UNHALTED.THREAD_P");
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("instruction_accesses.L1_misses", s)) {
        return PERFEXPERT_ERROR;
    }

    /* instruction_TLB.overall */
    bzero(s, MAX_LCPI);
    strcpy(s, "PAGE_WALKS.WALKS / INST_RETIRED.ANY_P");
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("instruction_TLB.overall", s)) {
        return PERFEXPERT_ERROR;
    }

    bzero(s, MAX_LCPI);
    strcpy(s, "(PAGE_WALKS.D_SIDE_CYCLES+PAGE_WALKS.I_SIDE_CYCLES) / CPU_CLK_UNHALTED.THREAD_P");
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("instruction_TLB.time", s)) {
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
   
    bzero(s, MAX_LCPI);
    strcpy(s, "((UNC_E_RPQ_INSERTS - UNC_E_EDC_ACCESS.HIT_CLEAN / UNC_E_EDC_ACCESS.MISS_CLEAN - "
              "UNC_E_EDC_ACCESS.HIT_DIRTY / UNC_E_EDC_ACCESS.MISS_DIRTY) * 64 / CPU_CLK_UNHALTED.THREAD_P) + "
              "(UNC_E_WPQ_INSERTS * 64 / CPU_CLK_UNHALTED.THREAD_P)");
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("mcdram.overall", s)) {
        return PERFEXPERT_ERROR;
    }

    /* MCDRAM Read BW */
    bzero(s, MAX_LCPI);
    // This is for Cache Mode
    if (is_flat_mode()) {
        strcpy(s, "UNC_E_RPQ_INSERTS * 64 / CPU_CLK_UNHALTED.THREAD_P");
    }
    else {
        strcpy(s, "(UNC_E_RPQ_INSERTS - UNC_E_EDC_ACCESS.HIT_CLEAN / UNC_E_EDC_ACCESS.MISS_CLEAN - "
                  "UNC_E_EDC_ACCESS.HIT_DIRTY / UNC_E_EDC_ACCESS.MISS_DIRTY) * 64 / CPU_CLK_UNHALTED.THREAD_P");
    }
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("mcdram.read_bandwidth", s)) {
        return PERFEXPERT_ERROR;
    }

    /* MCDRAM Write BW */
    bzero(s, MAX_LCPI);
    // I should add this: DCLK_Events_CAS_Reads
    strcpy(s, "UNC_E_WPQ_INSERTS * 64 / CPU_CLK_UNHALTED.THREAD_P");
    if (PERFEXPERT_SUCCESS != lcpi_add_metric("mcdram.write_bandwidth", s)) {
        return PERFEXPERT_ERROR;
    }

    return PERFEXPERT_SUCCESS;
}

// EOF
