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

#include <papi.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sniffer.h"

int main(int argc, char* argv []) {
    int saved_sampling_freq[SAMPLING_CATEGORY_COUNT] = { 0 };
    lcpi_t metrics;

    if (PAPI_VER_CURRENT != PAPI_library_init(PAPI_VER_CURRENT)) {
        fprintf(stderr, "Failed initializing PAPI\n");
        return 1;
    }

    attach_sampling_rate(saved_sampling_freq);

    /* Generate LCPI metrics */
    get_ratio_floating_point(metrics.ratio_floating_point);
    get_ratio_data_accesses(metrics.ratio_data_accesses);
    get_gflops(metrics.gflops_overall, metrics.gflops_packed,
        metrics.gflops_scalar);
    get_overall(metrics.overall);
    get_data_accesses(metrics.data_accesses_overall,
        metrics.data_accesses_L1_hits, metrics.data_accesses_L2_hits,
        metrics.data_accesses_L3_hits, metrics.data_accesses_LLC_misses);
    get_instruction_accesses(metrics.instruction_accesses_overall,
        metrics.instruction_accesses_L1_hits,
        metrics.instruction_accesses_L2_hits,
        metrics.instruction_accesses_L2_misses);
    get_tlb(metrics.data_TLB_overall,
        metrics.instruction_TLB_overall);
    get_branch_instructions(metrics.branch_instructions_overall,
        metrics.branch_instructions_correctly_predicted,
        metrics.branch_instructions_mispredicted);
    get_floating_point_instr(metrics.floating_point_instr_overall,
        metrics.floating_point_instr_fast_FP_instr,
        metrics.floating_point_instr_slow_FP_instr);

    /* Generate files */
    write_experiment();
    write_lcpi(metrics);

    return 0;
}

/* write_lcpi */
void write_lcpi(lcpi_t metrics) {
    FILE* fp;

    fp = fopen("lcpi.conf", "w");
    if (NULL == fp) {
        fprintf(stderr, "Could not open file lcpi.conf for writing\n");
        exit(1);
    }

    fprintf(fp, "# LCPI config generated using sniffer\n# version = 1.0\n");
    fprintf(fp, "%s\n", metrics.ratio_floating_point);
    fprintf(fp, "%s\n", metrics.ratio_data_accesses);
    fprintf(fp, "%s\n", metrics.gflops_overall);
    fprintf(fp, "%s\n", metrics.gflops_packed);
    fprintf(fp, "%s\n", metrics.gflops_scalar);
    fprintf(fp, "%s\n", metrics.overall);
    fprintf(fp, "%s\n", metrics.data_accesses_overall);
    fprintf(fp, "%s\n", metrics.data_accesses_L1_hits);
    fprintf(fp, "%s\n", metrics.data_accesses_L2_hits);
    fprintf(fp, "%s\n", metrics.data_accesses_L3_hits);
    fprintf(fp, "%s\n", metrics.data_accesses_LLC_misses);
    fprintf(fp, "%s\n", metrics.instruction_accesses_overall);
    fprintf(fp, "%s\n", metrics.instruction_accesses_L1_hits);
    fprintf(fp, "%s\n", metrics.instruction_accesses_L2_hits);
    fprintf(fp, "%s\n", metrics.instruction_accesses_L2_misses);
    fprintf(fp, "%s\n", metrics.data_TLB_overall);
    fprintf(fp, "%s\n", metrics.instruction_TLB_overall);
    fprintf(fp, "%s\n", metrics.branch_instructions_overall);
    fprintf(fp, "%s\n", metrics.branch_instructions_correctly_predicted);
    fprintf(fp, "%s\n", metrics.branch_instructions_mispredicted);
    fprintf(fp, "%s\n", metrics.floating_point_instr_overall);
    fprintf(fp, "%s\n", metrics.floating_point_instr_fast_FP_instr);
    fprintf(fp, "%s\n", metrics.floating_point_instr_slow_FP_instr);

    fclose (fp);
}

/* write_experiment */
void write_experiment(void) {
    FILE* fp;
    int ret;
    int event_code;
    int exp_count = 0;
    int papi_tot_ins_code;
    short addCount, remaining;

    fp = fopen("experiment.conf", "w");
    if (NULL == fp) {
        fprintf(stderr, "Could not open file experiment.conf for writing\n");
        exit(1);
    }

    fprintf(fp, "# Experiment file generated using sniffer\n# version = 1.0\n");

    do {
        addCount = 0;
        remaining = 0;
        int j;
        int event_set = PAPI_NULL;
        
        if (PAPI_OK != (ret = PAPI_create_eventset(&event_set))) {
            fprintf(stderr, "Could not create PAPI event set\n");
            fprintf(stderr, "PAPI error message: %s.\n", PAPI_strerror(ret));
            fclose(fp);
            exit(1);
        }

        /* Add PAPI_TOT_INS as we will be measuring it in every run */
        PAPI_event_name_to_code("PAPI_TOT_INS", &papi_tot_ins_code);
        if (PAPI_OK != (ret = PAPI_add_event(event_set, papi_tot_ins_code))) {
            fprintf(stderr, "Could not add PAPI_TOT_INS to the event set\n");
            fprintf(stderr, "PAPI error message: %s.\n", PAPI_strerror(ret));
            fclose(fp);
            exit(1);
        }

        for (j = 0; j < EVENT_COUNT; j++) {
            if (PAPI_TOT_INS == j) {
                continue;
            }
            
            if (IS_EVENT_USED(j) && TOT_INS != j) {
                PAPI_event_name_to_code(event_list[j].PAPI_event_name,
                    &event_code);
                if (PAPI_OK == PAPI_add_event(event_set, event_code)) {
                    ADD_EVENT(j);
                    if (0 == remaining) { // New line
                        fprintf(fp, "%%\n", exp_count);
                    }
                    fprintf(fp, "%s:%d\n", event_list[j].PAPI_event_name,
                        event_list[j].sampling_freq);
                    addCount++;
                }
                remaining++;
            }
        }
        
        if (0 == addCount && 0 < remaining) {
            // Some events remain but could not be added to the event set
            fprintf(stderr,
                "Error: The following events are available but could not be added to the event set:\n");
            for (j = 0; j < EVENT_COUNT; j++) {
                if (IS_EVENT_USED(j)) {
                    fprintf(stderr, "%s ", event_list[j].PAPI_event_name);
                }
            }
            fprintf(stderr, "\n");
            break;
        } else {
            exp_count++;
        }
        if ((0 < addCount) && (0 < remaining)) {
            fprintf(fp, "PAPI_TOT_INS:%d\n", event_list[TOT_INS].sampling_freq);
        }
    } while (0 < remaining);

    fclose(fp);
}

/* get_floating_point_instr */
void get_floating_point_instr(char *overall, char *slow, char *fast) {
    char lcpi_slow[LCPI_DEF_LENGTH];
    char lcpi_fast[LCPI_DEF_LENGTH];

    if (IS_EVENT_AVAILABLE(FML_INS)) {
        USE_EVENT(FML_INS);
        if (IS_EVENT_AVAILABLE(FAD_INS)) {
            USE_EVENT(FAD_INS);
            strcpy(lcpi_fast, "((PAPI_FML_INS + PAPI_FAD_INS) * FP_lat)");
        } else {
            strcpy(lcpi_fast, "(PAPI_FML_INS * FP_lat)");
        }

    } else if (IS_EVENT_AVAILABLE(FP_COMP_OPS_EXE_SSE_DOUBLE_PRECISION) &&
        IS_EVENT_AVAILABLE(FP_COMP_OPS_EXE_SSE_FP) &&
        IS_EVENT_AVAILABLE(FP_COMP_OPS_EXE_SSE_FP_PACKED) &&
        IS_EVENT_AVAILABLE(FP_COMP_OPS_EXE_SSE_FP_SCALAR) &&
        IS_EVENT_AVAILABLE(FP_COMP_OPS_EXE_SSE_SINGLE_PRECISION) &&
        IS_EVENT_AVAILABLE(FP_COMP_OPS_EXE_X87)) {

        // Do a dummy test to see if the single big counter can be added or do we have to break it down into multiple parts
        if (0 == test_counter(FP_COMP_OPS_EXE_SSE_DOUBLE_PRECISION_SSE_FP_SSE_FP_PACKED_SSE_FP_SCALAR_SSE_SINGLE_PRECISION_X87)) {
            USE_EVENT(FP_COMP_OPS_EXE_SSE_DOUBLE_PRECISION_SSE_FP_SSE_FP_PACKED_SSE_FP_SCALAR_SSE_SINGLE_PRECISION_X87);
            DOWNGRADE_EVENT(FP_COMP_OPS_EXE_SSE_DOUBLE_PRECISION);
            DOWNGRADE_EVENT(FP_COMP_OPS_EXE_SSE_FP);
            DOWNGRADE_EVENT(FP_COMP_OPS_EXE_SSE_FP_PACKED);
            DOWNGRADE_EVENT(FP_COMP_OPS_EXE_SSE_FP_SCALAR);
            DOWNGRADE_EVENT(FP_COMP_OPS_EXE_SSE_SINGLE_PRECISION);
            DOWNGRADE_EVENT(FP_COMP_OPS_EXE_X87);
            
            sprintf(lcpi_fast, "(%s*FP_lat)",
                    event_list[FP_COMP_OPS_EXE_SSE_DOUBLE_PRECISION_SSE_FP_SSE_FP_PACKED_SSE_FP_SCALAR_SSE_SINGLE_PRECISION_X87].PAPI_event_name);
        } else {
            // Asume that we can add separately. If this is false, we will catch it at the time of generating LCPIs
            // Use the available counters
            USE_EVENT(FP_COMP_OPS_EXE_SSE_DOUBLE_PRECISION);
            USE_EVENT(FP_COMP_OPS_EXE_SSE_FP);
            USE_EVENT(FP_COMP_OPS_EXE_SSE_FP_PACKED);
            USE_EVENT(FP_COMP_OPS_EXE_SSE_FP_SCALAR);
            USE_EVENT(FP_COMP_OPS_EXE_SSE_SINGLE_PRECISION);
            USE_EVENT(FP_COMP_OPS_EXE_X87);
            
            sprintf(lcpi_fast, "((%s + %s + %s + %s + %s + %s) * FP_lat)",
                event_list[FP_COMP_OPS_EXE_SSE_DOUBLE_PRECISION].PAPI_event_name,
                event_list[FP_COMP_OPS_EXE_SSE_FP].PAPI_event_name,
                event_list[FP_COMP_OPS_EXE_SSE_FP_PACKED].PAPI_event_name,
                event_list[FP_COMP_OPS_EXE_SSE_FP_SCALAR].PAPI_event_name,
                event_list[FP_COMP_OPS_EXE_SSE_SINGLE_PRECISION].PAPI_event_name,
                event_list[FP_COMP_OPS_EXE_X87].PAPI_event_name);
        }
    } else if (IS_EVENT_AVAILABLE(FP_COMP_OPS_EXE_SSE_FP_PACKED_DOUBLE) &&
        IS_EVENT_AVAILABLE(FP_COMP_OPS_EXE_SSE_FP_SCALAR_SINGLE) &&
        IS_EVENT_AVAILABLE(FP_COMP_OPS_EXE_SSE_PACKED_SINGLE) &&
        IS_EVENT_AVAILABLE(FP_COMP_OPS_EXE_SSE_SCALAR_DOUBLE)) {

        if (0 == test_counter(FP_COMP_OPS_EXE_SSE_FP_SCALAR_SINGLE_SSE_SCALAR_DOUBLE)) {
            USE_EVENT(FP_COMP_OPS_EXE_SSE_FP_SCALAR_SINGLE_SSE_SCALAR_DOUBLE);
            DOWNGRADE_EVENT(FP_COMP_OPS_EXE_SSE_FP_SCALAR_SINGLE);
            DOWNGRADE_EVENT(FP_COMP_OPS_EXE_SSE_SCALAR_DOUBLE);
            USE_EVENT(FP_COMP_OPS_EXE_SSE_FP_PACKED_DOUBLE);
            USE_EVENT(FP_COMP_OPS_EXE_SSE_PACKED_SINGLE);
            
            sprintf(lcpi_fast, "((%s + %s + %s) * FP_lat)",
                event_list[FP_COMP_OPS_EXE_SSE_FP_PACKED_DOUBLE].PAPI_event_name,
                event_list[FP_COMP_OPS_EXE_SSE_PACKED_SINGLE].PAPI_event_name,
                event_list[FP_COMP_OPS_EXE_SSE_FP_SCALAR_SINGLE_SSE_SCALAR_DOUBLE].PAPI_event_name);
        } else {

            // Asume that we can add separately. If this is false, we will catch it at the time of generating LCPIs
            USE_EVENT(FP_COMP_OPS_EXE_SSE_FP_PACKED_DOUBLE);
            USE_EVENT(FP_COMP_OPS_EXE_SSE_FP_SCALAR_SINGLE);
            USE_EVENT(FP_COMP_OPS_EXE_SSE_PACKED_SINGLE);
            USE_EVENT(FP_COMP_OPS_EXE_SSE_SCALAR_DOUBLE);

            sprintf(lcpi_fast, "((%s + %s + %s + %s) * FP_lat)",
                event_list[FP_COMP_OPS_EXE_SSE_FP_PACKED_DOUBLE].PAPI_event_name,
                event_list[FP_COMP_OPS_EXE_SSE_FP_SCALAR_SINGLE].PAPI_event_name,
                event_list[FP_COMP_OPS_EXE_SSE_PACKED_SINGLE].PAPI_event_name,
                event_list[FP_COMP_OPS_EXE_SSE_SCALAR_DOUBLE].PAPI_event_name);
        }
    } else if (IS_EVENT_AVAILABLE(ARITH)) {
        USE_EVENT(ARITH);
        sprintf(lcpi_fast, "ARITH * FP_lat");
    } else {
        counter_err("floating-point_instr.fast_FP_inst");
    }
    
    if (IS_EVENT_AVAILABLE(FDV_INS)) {
        USE_EVENT(FDV_INS);
        strcpy(lcpi_slow, "(PAPI_FDV_INS * FP_slow_lat)");
    } else if (IS_EVENT_AVAILABLE(ARITH_CYCLES_DIV_BUSY)) {
        USE_EVENT(ARITH_CYCLES_DIV_BUSY);
        strcpy(lcpi_slow, "ARITH:CYCLES_DIV_BUSY");
    } else {
        counter_err("floating-point_instr.slow_FP_inst");
    }
    
    sprintf(overall, "floating-point_instr.overall = (%s + %s) / PAPI_TOT_INS",
        lcpi_fast, lcpi_slow);
    sprintf(fast, "floating-point_instr.fast_FP_instr = %s / PAPI_TOT_INS",
        lcpi_fast);
    sprintf(slow, "floating-point_instr.slow_FP_instr = %s / PAPI_TOT_INS",
        lcpi_slow);
}

/* get_branch_instructions */
void get_branch_instructions(char *overall, char *predicted,
    char *mispredicted) {
    if (IS_EVENT_AVAILABLE(BR_INS) && IS_EVENT_AVAILABLE(BR_MSP)) {
        USE_EVENT(BR_INS);
        USE_EVENT(BR_MSP);
        strcpy(overall,
            "branch_instructions.overall = (PAPI_BR_INS * BR_lat + PAPI_BR_MSP * BR_miss_lat) / PAPI_TOT_INS");
        strcpy(predicted,
            "branch_instructions.correctly_predicted = PAPI_BR_INS * BR_lat / PAPI_TOT_INS");
        strcpy(mispredicted,
            "branch_instructions.mispredicted = PAPI_BR_MSP * BR_miss_lat / PAPI_TOT_INS");
    } else {
        counter_err("branch_instructions");
    }
}

/* get_tlb */
void get_tlb(char *data, char *instruction) {
    /* data_TLB */
    if (IS_EVENT_AVAILABLE(DTLB_LOAD_MISSES_WALK_DURATION)) {
        USE_EVENT(DTLB_LOAD_MISSES_WALK_DURATION);
        strcpy(data,
            "data_TLB.overall = DTLB_LOAD_MISSES:WALK_DURATION / PAPI_TOT_INS");
    } else  if (IS_EVENT_AVAILABLE(TLB_DM)) {
        USE_EVENT(TLB_DM);
        strcpy(data, "data_TLB.overall = PAPI_TLB_DM * TLB_lat / PAPI_TOT_INS");
    } else if (IS_EVENT_AVAILABLE(DTLB_LOAD_MISSES_CAUSES_A_WALK) &&
               IS_EVENT_AVAILABLE(DTLB_STORE_MISSES_CAUSES_A_WALK)) {
        USE_EVENT(DTLB_LOAD_MISSES_CAUSES_A_WALK);
        USE_EVENT(DTLB_STORE_MISSES_CAUSES_A_WALK);
        strcpy(data,
            "data_TLB.overall = (DTLB_LOAD_MISSES:CAUSES_A_WALK + DTLB_STORE_MISSES:CAUSES_A_WALK) * TLB_lat / PAPI_TOT_INS");
    } else {
        counter_err("data_TLB.overall");
    }

    /* instruction_TLB */
    if (IS_EVENT_AVAILABLE(ITLB_MISSES_WALK_DURATION)) {
        USE_EVENT(ITLB_MISSES_WALK_DURATION);
        strcpy(instruction,
            "instruction_TLB.overall = ITLB_MISSES:WALK_DURATION / PAPI_TOT_INS");
    } else if (IS_EVENT_AVAILABLE(TLB_IM)) {
        USE_EVENT(TLB_IM);
        strcpy(instruction,
            "instruction_TLB.overall = PAPI_TLB_IM * TLB_lat / PAPI_TOT_INS");
    } else {
        counter_err("instruction_TLB.overall");
    }
}

/* get_instruction_accesses */
void get_instruction_accesses(char *overall, char *L1_hits, char *L2_hits,
    char *L2_misses) {

    char lcpi_L2_ICA[LCPI_DEF_LENGTH];
    char lcpi_L2_ICM[LCPI_DEF_LENGTH];
    char l1_hits[LCPI_DEF_LENGTH];

    if (IS_EVENT_AVAILABLE(L1_ICA)) {
        USE_EVENT(L1_ICA);
        sprintf(l1_hits, "%s", event_list[L1_ICA].PAPI_event_name);
    } else if (IS_EVENT_AVAILABLE(ICACHE)) {
        USE_EVENT(ICACHE);
        sprintf(l1_hits, "%s", event_list[ICACHE].PAPI_event_name);
    } else {
        counter_err("instruction_accesses_L1_hits");
    }

    if (IS_EVENT_AVAILABLE(L2_ICA)) {
        USE_EVENT(L2_ICA);
        strcpy(lcpi_L2_ICA, "PAPI_L2_ICA");
    } else if (IS_EVENT_AVAILABLE(L2_TCA) && IS_EVENT_AVAILABLE(L2_DCA)) {
        USE_EVENT(L2_TCA);
        USE_EVENT(L2_DCA);
        strcpy(lcpi_L2_ICA, "(PAPI_L2_TCA - PAPI_L2_DCA)");
    } else {
        counter_err("PAPI_L2_ICA");
    }
    
    if (IS_EVENT_AVAILABLE(L2_ICM)) {
        USE_EVENT(L2_ICM);
        strcpy(lcpi_L2_ICM, "PAPI_L2_ICM");
    } else if (IS_EVENT_AVAILABLE(L2_TCM) && IS_EVENT_AVAILABLE(L2_DCM)) {
        USE_EVENT(L2_TCM);
        USE_EVENT(L2_DCM);
        strcpy(lcpi_L2_ICM, "(PAPI_L2_TCM - PAPI_L2_DCM)");
    } else {
        counter_err("PAPI_L2_ICM");
    }

    sprintf(overall,
        "instruction_accesses.overall = (%s * L1_ilat + %s * L2_lat + %s * mem_lat) / PAPI_TOT_INS",
        l1_hits, lcpi_L2_ICA, lcpi_L2_ICM);
    sprintf(L1_hits,
        "instruction_accesses.L1i_hits = (%s * L1_ilat) / PAPI_TOT_INS",
        l1_hits);
    sprintf(L2_hits,
        "instruction_accesses.L2i_hits = %s * L2_lat / PAPI_TOT_INS",
        lcpi_L2_ICA);
    sprintf(L2_misses,
        "instruction_accesses.L2i_misses = %s * mem_lat / PAPI_TOT_INS",
        lcpi_L2_ICM);
}

/* get_data_accesses */
void get_data_accesses(char *overall, char *L1_hits, char *L2_hits,
    char *L3_hits, char *LLC_misses) {

    char l1[LCPI_DEF_LENGTH];
    char l2[LCPI_DEF_LENGTH];

    bzero(l1, LCPI_DEF_LENGTH);
    bzero(l2, LCPI_DEF_LENGTH);

    if (IS_EVENT_AVAILABLE(L1_DCA)) {
        USE_EVENT(L1_DCA);
        strcpy(l1, "PAPI_L1_DCA");
    } else if (IS_EVENT_AVAILABLE(LD_INS)) {
        USE_EVENT(LD_INS);
        strcpy(l1, "PAPI_LD_INS");
    } else {
        counter_err("data_accesses_L1_hits");
    }

    if (IS_EVENT_AVAILABLE(L2_RQSTS_DEMAND_DATA_RD_HIT)) {
        USE_EVENT(L2_RQSTS_DEMAND_DATA_RD_HIT);
        strcpy(l2, "L2_RQSTS:DEMAND_DATA_RD_HIT");
    } else if (IS_EVENT_AVAILABLE(L2_DCA)) {
        USE_EVENT(L2_DCA);
        strcpy(l2, "PAPI_L2_DCA");
    } else if (IS_EVENT_AVAILABLE(L2_TCA) && IS_EVENT_AVAILABLE(L2_ICA)) {
        USE_EVENT(L2_TCA);
        USE_EVENT(L2_ICA);
        strcpy(l2, "(PAPI_L2_TCA - PAPI_L2_ICA)");
    } else {
        counter_err("data_accesses_L2_hits");
    }

    if ((IS_EVENT_AVAILABLE(L3_TCA)) &&
        (IS_EVENT_AVAILABLE(L3_TCM))) {
        USE_EVENT(L3_TCA);
        USE_EVENT(L3_TCM);
        strcpy(L3_hits,
            "data_accesses.L3d_hits = ((PAPI_L3_TCA - PAPI_L3_TCM) * L3_lat) / PAPI_TOT_INS");
    }

    if ((IS_EVENT_AVAILABLE(L3_TCA)) &&
        (IS_EVENT_AVAILABLE(L3_TCM))) {
        USE_EVENT(L3_TCA);
        USE_EVENT(L3_TCM);
        strcpy(LLC_misses,
            "data_accesses.LLC_misses = (PAPI_L3_TCM * mem_lat) / PAPI_TOT_INS");
        sprintf(overall,
            "data_accesses.overall = (%s * L1_dlat + %s * L2_lat + (PAPI_L3_TCA - PAPI_L3_TCM) * L3_lat + PAPI_L3_TCM * mem_lat) / PAPI_TOT_INS",
            l1, l2);
    } else if (IS_EVENT_AVAILABLE(L2_DCM)) {
        USE_EVENT(L2_DCM);
        sprintf(LLC_misses,
            "data_accesses.LLC_misses = (%s * mem_lat) / PAPI_TOT_INS", l2);
        sprintf(overall,
            "data_accesses.overall = (%s * L1_dlat + %s * L2_lat + PAPI_L2_DCM * mem_lat) / PAPI_TOT_INS",
            l1, l2);
    } else if (IS_EVENT_AVAILABLE(L2_TCM) && IS_EVENT_AVAILABLE(L2_ICM)) {
        USE_EVENT(L2_TCM);
        USE_EVENT(L2_ICM);
        sprintf(LLC_misses,
            "data_accesses.LLC_misses = (%s * mem_lat) / PAPI_TOT_INS", l2);
        sprintf(overall,
            "data_accesses.overall = (%s * L1_dlat + %s * L2_lat + (PAPI_L2_TCM - PAPI_L2_ICM) * mem_lat) / PAPI_TOT_INS",
            l1, l2);
    } else {
        counter_err("data_accesses_LLC_misses");
    }
    
    sprintf(L1_hits, "data_accesses.L1d_hits = (%s * L1_dlat) / PAPI_TOT_INS", l1);
    sprintf(L2_hits, "data_accesses.L2d_hits = (%s * L2_lat) / PAPI_TOT_INS", l2);
}

/* get_gflops */
void get_gflops(char *overall, char *packed, char *scalar) {
    if (IS_EVENT_AVAILABLE(RETIRED_SSE_OPERATIONS_ALL)) {

        USE_EVENT(RETIRED_SSE_OPERATIONS_ALL);

        sprintf(overall,
            "GFLOPS_(%%_max).overall = (%s / PAPI_TOT_CYC) / 4",
            event_list[RETIRED_SSE_OPERATIONS_ALL].PAPI_event_name);        
        strcpy(packed, "");
        strcpy(scalar, "");

    } else if (IS_EVENT_AVAILABLE(RETIRED_SSE_OPS_ALL)) {

        USE_EVENT(RETIRED_SSE_OPS_ALL);

        sprintf(overall,
            "GFLOPS_(%%_max).overall = (%s / PAPI_TOT_CYC) / 4",
            event_list[RETIRED_SSE_OPS_ALL].PAPI_event_name);        
        strcpy(packed, "");
        strcpy(scalar, "");

    } else if (IS_EVENT_AVAILABLE(SSEX_UOPS_RETIRED_PACKED_DOUBLE) &&
        IS_EVENT_AVAILABLE(SSEX_UOPS_RETIRED_PACKED_SINGLE) &&
        IS_EVENT_AVAILABLE(SSEX_UOPS_RETIRED_SCALAR_DOUBLE) &&
        IS_EVENT_AVAILABLE(SSEX_UOPS_RETIRED_SCALAR_SINGLE)) {

        // Do a dummy test to see if the single big counter can be added or do we have to break it down into multiple parts
        if (0 == test_counter(SSEX_UOPS_RETIRED_SCALAR_DOUBLE_SCALAR_SINGLE)) {
            USE_EVENT(SSEX_UOPS_RETIRED_PACKED_DOUBLE);
            USE_EVENT(SSEX_UOPS_RETIRED_PACKED_SINGLE);
            USE_EVENT(SSEX_UOPS_RETIRED_SCALAR_DOUBLE_SCALAR_SINGLE);
            DOWNGRADE_EVENT(SSEX_UOPS_RETIRED_SCALAR_DOUBLE);
            DOWNGRADE_EVENT(SSEX_UOPS_RETIRED_SCALAR_SINGLE);
            
            sprintf(overall,
                "GFLOPS_(%%_max).overall = ((%s*2 + %s*4 + %s) / PAPI_TOT_CYC) / 4",
                event_list[SSEX_UOPS_RETIRED_PACKED_DOUBLE].PAPI_event_name,
                event_list[SSEX_UOPS_RETIRED_PACKED_SINGLE].PAPI_event_name,
                event_list[SSEX_UOPS_RETIRED_SCALAR_DOUBLE_SCALAR_SINGLE].PAPI_event_name);
            sprintf(packed,
                "GFLOPS_(%%_max).packed = ((%s*2 + %s*4) / PAPI_TOT_CYC) / 4",
                event_list[SSEX_UOPS_RETIRED_PACKED_DOUBLE].PAPI_event_name,
                event_list[SSEX_UOPS_RETIRED_PACKED_SINGLE].PAPI_event_name);
            sprintf(scalar,
                "GFLOPS_(%%_max).scalar = (%s / PAPI_TOT_CYC) / 4",
                event_list[SSEX_UOPS_RETIRED_SCALAR_DOUBLE_SCALAR_SINGLE].PAPI_event_name);
        } else {
            // Asume that we can add separately. If this is false, we will catch it at the time of generating LCPIs
            // Use the available counters
            USE_EVENT(SSEX_UOPS_RETIRED_PACKED_DOUBLE);
            USE_EVENT(SSEX_UOPS_RETIRED_PACKED_SINGLE);
            USE_EVENT(SSEX_UOPS_RETIRED_SCALAR_DOUBLE);
            USE_EVENT(SSEX_UOPS_RETIRED_SCALAR_SINGLE);
            
            sprintf(overall,
                "GFLOPS_(%%_max).overall = ((%s*2 + %s*4 + %s + %s) / PAPI_TOT_CYC) / 4",
                event_list[SSEX_UOPS_RETIRED_PACKED_DOUBLE].PAPI_event_name,
                event_list[SSEX_UOPS_RETIRED_PACKED_SINGLE].PAPI_event_name,
                event_list[SSEX_UOPS_RETIRED_SCALAR_SINGLE].PAPI_event_name,
                event_list[SSEX_UOPS_RETIRED_SCALAR_DOUBLE].PAPI_event_name);
            sprintf(packed,
                "GFLOPS_(%%_max).packed = ((%s*2 + %s*4) / PAPI_TOT_CYC) / 4",
                event_list[SSEX_UOPS_RETIRED_PACKED_DOUBLE].PAPI_event_name,
                event_list[SSEX_UOPS_RETIRED_PACKED_SINGLE].PAPI_event_name);
            sprintf(scalar,
                "GFLOPS_(%%_max).scalar = (%s + %s / PAPI_TOT_CYC) / 4",
                event_list[SSEX_UOPS_RETIRED_SCALAR_DOUBLE].PAPI_event_name,
                event_list[SSEX_UOPS_RETIRED_SCALAR_SINGLE].PAPI_event_name);
        }
    } else if (IS_EVENT_AVAILABLE(SIMD_COMP_INST_RETIRED_PACKED_DOUBLE) &&
        IS_EVENT_AVAILABLE(SIMD_COMP_INST_RETIRED_PACKED_SINGLE) &&
        IS_EVENT_AVAILABLE(SIMD_COMP_INST_RETIRED_SCALAR_DOUBLE) &&
        IS_EVENT_AVAILABLE(SIMD_COMP_INST_RETIRED_SCALAR_SINGLE)) {

        // Do a dummy test to see if the single big counter can be added or do we have to break it down into multiple parts
        if (0 == test_counter(SIMD_COMP_INST_RETIRED_SCALAR_SINGLE_SCALAR_DOUBLE)) {
            USE_EVENT(SIMD_COMP_INST_RETIRED_PACKED_DOUBLE);
            USE_EVENT(SIMD_COMP_INST_RETIRED_PACKED_SINGLE);
            DOWNGRADE_EVENT(SIMD_COMP_INST_RETIRED_SCALAR_DOUBLE);
            DOWNGRADE_EVENT(SIMD_COMP_INST_RETIRED_SCALAR_SINGLE);
            USE_EVENT(SIMD_COMP_INST_RETIRED_SCALAR_SINGLE_SCALAR_DOUBLE);

            sprintf(overall,
                "GFLOPS_(%%_max).overall = ((%s*2 + %s*4 + %s) / PAPI_TOT_CYC) / 4",
                event_list[SIMD_COMP_INST_RETIRED_PACKED_DOUBLE].PAPI_event_name,
                event_list[SIMD_COMP_INST_RETIRED_PACKED_SINGLE].PAPI_event_name,
                event_list[SIMD_COMP_INST_RETIRED_SCALAR_SINGLE_SCALAR_DOUBLE].PAPI_event_name);
            sprintf(packed,
                "GFLOPS_(%%_max).packed = ((%s*2 + %s*4) / PAPI_TOT_CYC) / 4",
                event_list[SIMD_COMP_INST_RETIRED_PACKED_DOUBLE].PAPI_event_name,
                event_list[SIMD_COMP_INST_RETIRED_PACKED_SINGLE].PAPI_event_name);
            sprintf(scalar,
                "GFLOPS_(%%_max).scalar = (%s / PAPI_TOT_CYC) / 4",
                event_list[SIMD_COMP_INST_RETIRED_SCALAR_SINGLE_SCALAR_DOUBLE].PAPI_event_name);
        } else {
            USE_EVENT(SIMD_COMP_INST_RETIRED_PACKED_DOUBLE);
            USE_EVENT(SIMD_COMP_INST_RETIRED_PACKED_SINGLE);
            USE_EVENT(SIMD_COMP_INST_RETIRED_SCALAR_DOUBLE);
            USE_EVENT(SIMD_COMP_INST_RETIRED_SCALAR_SINGLE);
            
            sprintf(overall,
                "GFLOPS_(%%_max).overall = ((%s*2 + %s*4 + %s + %s) / PAPI_TOT_CYC) / 4",
                event_list[SIMD_COMP_INST_RETIRED_PACKED_DOUBLE].PAPI_event_name,
                event_list[SIMD_COMP_INST_RETIRED_PACKED_SINGLE].PAPI_event_name,
                event_list[SIMD_COMP_INST_RETIRED_SCALAR_SINGLE].PAPI_event_name,
                event_list[SIMD_COMP_INST_RETIRED_SCALAR_DOUBLE].PAPI_event_name);
            sprintf(packed,
                "GFLOPS_(%%_max).packed = ((%s*2 + %s*4) / PAPI_TOT_CYC) / 4",
                event_list[SIMD_COMP_INST_RETIRED_PACKED_DOUBLE].PAPI_event_name,
                event_list[SIMD_COMP_INST_RETIRED_PACKED_SINGLE].PAPI_event_name);
            sprintf(scalar,
                "GFLOPS_(%%_max).scalar = ((%s + %s)/ PAPI_TOT_CYC) / 4",
                event_list[SIMD_COMP_INST_RETIRED_SCALAR_SINGLE].PAPI_event_name,
                event_list[SIMD_COMP_INST_RETIRED_SCALAR_DOUBLE].PAPI_event_name);
        }
    } else if (IS_EVENT_AVAILABLE(SIMD_FP_256_PACKED_SINGLE) &&
        IS_EVENT_AVAILABLE(SIMD_FP_256_PACKED_DOUBLE) &&
        IS_EVENT_AVAILABLE(FP_COMP_OPS_EXE_SSE_FP_PACKED_DOUBLE) &&
        IS_EVENT_AVAILABLE(FP_COMP_OPS_EXE_SSE_FP_SCALAR_SINGLE) &&
        IS_EVENT_AVAILABLE(FP_COMP_OPS_EXE_SSE_PACKED_SINGLE) &&
        IS_EVENT_AVAILABLE(FP_COMP_OPS_EXE_SSE_SCALAR_DOUBLE)) {

        if (0 == test_counter(FP_COMP_OPS_EXE_SSE_FP_SCALAR_SINGLE_SSE_SCALAR_DOUBLE)) {
            DOWNGRADE_EVENT(FP_COMP_OPS_EXE_SSE_FP_SCALAR);
            DOWNGRADE_EVENT(FP_COMP_OPS_EXE_SSE_SCALAR_DOUBLE);
            USE_EVENT(FP_COMP_OPS_EXE_SSE_FP_SCALAR_SINGLE_SSE_SCALAR_DOUBLE);
            USE_EVENT(SIMD_FP_256_PACKED_SINGLE);
            USE_EVENT(SIMD_FP_256_PACKED_DOUBLE);
            USE_EVENT(FP_COMP_OPS_EXE_SSE_FP_PACKED_DOUBLE);
            USE_EVENT(FP_COMP_OPS_EXE_SSE_PACKED_SINGLE);
            
            sprintf(overall,
                "GFLOPS_(%%_max).overall = ((%s*8 + (%s + %s)*4 + %s*2 + %s) / PAPI_TOT_CYC) / 8",
                event_list[SIMD_FP_256_PACKED_SINGLE].PAPI_event_name,
                event_list[SIMD_FP_256_PACKED_DOUBLE].PAPI_event_name,
                event_list[FP_COMP_OPS_EXE_SSE_PACKED_SINGLE].PAPI_event_name,
                event_list[FP_COMP_OPS_EXE_SSE_FP_PACKED_DOUBLE].PAPI_event_name,
                event_list[FP_COMP_OPS_EXE_SSE_FP_SCALAR_SINGLE_SSE_SCALAR_DOUBLE].PAPI_event_name);
            sprintf(packed,
                "GFLOPS_(%%_max).packed = ((%s*8 + (%s + %s)*4 + %s*2) / PAPI_TOT_CYC) / 8",
                event_list[SIMD_FP_256_PACKED_SINGLE].PAPI_event_name,
                event_list[SIMD_FP_256_PACKED_DOUBLE].PAPI_event_name,
                event_list[FP_COMP_OPS_EXE_SSE_PACKED_SINGLE].PAPI_event_name,
                event_list[FP_COMP_OPS_EXE_SSE_FP_PACKED_DOUBLE].PAPI_event_name);
            sprintf(scalar,
                "GFLOPS_(%%_max).scalar = (%s / PAPI_TOT_CYC) / 8",
                event_list[FP_COMP_OPS_EXE_SSE_FP_SCALAR_SINGLE_SSE_SCALAR_DOUBLE].PAPI_event_name);
        } else {
            USE_EVENT(FP_COMP_OPS_EXE_SSE_FP_SCALAR);
            USE_EVENT(FP_COMP_OPS_EXE_SSE_SCALAR_DOUBLE);
            USE_EVENT(SIMD_FP_256_PACKED_SINGLE);
            USE_EVENT(SIMD_FP_256_PACKED_DOUBLE);
            USE_EVENT(FP_COMP_OPS_EXE_SSE_FP_PACKED_DOUBLE);
            USE_EVENT(FP_COMP_OPS_EXE_SSE_PACKED_SINGLE);
            
            sprintf(overall,
                "GFLOPS_(%%_max).overall = ((%s*8 + (%s + %s)*4 + %s*2 + %s + %s) / PAPI_TOT_CYC) / 8",
                event_list[SIMD_FP_256_PACKED_SINGLE].PAPI_event_name,
                event_list[SIMD_FP_256_PACKED_DOUBLE].PAPI_event_name,
                event_list[FP_COMP_OPS_EXE_SSE_PACKED_SINGLE].PAPI_event_name,
                event_list[FP_COMP_OPS_EXE_SSE_FP_PACKED_DOUBLE].PAPI_event_name,
                event_list[FP_COMP_OPS_EXE_SSE_FP_SCALAR_SINGLE].PAPI_event_name,
                event_list[FP_COMP_OPS_EXE_SSE_SCALAR_DOUBLE].PAPI_event_name);
            sprintf(packed,
                "GFLOPS_(%%_max).packed = ((%s*8 + (%s + %s)*4 + %s*2) / PAPI_TOT_CYC) / 8",
                event_list[SIMD_FP_256_PACKED_SINGLE].PAPI_event_name,
                event_list[SIMD_FP_256_PACKED_DOUBLE].PAPI_event_name,
                event_list[FP_COMP_OPS_EXE_SSE_PACKED_SINGLE].PAPI_event_name,
                event_list[FP_COMP_OPS_EXE_SSE_FP_PACKED_DOUBLE].PAPI_event_name);
            sprintf(scalar,
                "GFLOPS_(%%_max).scalar = ((%s + %s) / PAPI_TOT_CYC) / 8",
                event_list[FP_COMP_OPS_EXE_SSE_FP_SCALAR_SINGLE].PAPI_event_name,
                event_list[FP_COMP_OPS_EXE_SSE_SCALAR_DOUBLE].PAPI_event_name);
        }
    } else {
        counter_err("GFLOPS");
    }
}

/* get_ratio_data_accesses */
void get_ratio_data_accesses(char *output) {
    char temp_str[LCPI_DEF_LENGTH];

    bzero(temp_str, LCPI_DEF_LENGTH);
    if (IS_EVENT_AVAILABLE(L1_DCA)) {
        USE_EVENT(L1_DCA);
        sprintf(temp_str, "%s", event_list[L1_DCA].PAPI_event_name);
    } else if (IS_EVENT_AVAILABLE(LD_INS)) {
        USE_EVENT(LD_INS);
        sprintf(temp_str, "%s", event_list[LD_INS].PAPI_event_name);
    } else {
        counter_err("ratio_data_accesses");
    }
    sprintf(output, "ratio.data_accesses = %s / PAPI_TOT_INS", temp_str);
}

/* get_ratio_floating_point */
void get_ratio_floating_point(char *output) {
    char temp_str[LCPI_DEF_LENGTH];

    bzero(temp_str, LCPI_DEF_LENGTH);
    if (IS_EVENT_AVAILABLE(FML_INS) && IS_EVENT_AVAILABLE(FDV_INS)) {
        USE_EVENT(FML_INS);
        USE_EVENT(FDV_INS);

        if (IS_EVENT_AVAILABLE(FAD_INS)) {
            USE_EVENT(FAD_INS);
            sprintf(temp_str, "(%s + %s + %s)",
                event_list[FML_INS].PAPI_event_name,
                event_list[FDV_INS].PAPI_event_name,
                event_list[FAD_INS].PAPI_event_name);
        } else {
            sprintf(temp_str, "(%s + %s)",
                event_list[FML_INS].PAPI_event_name,
                event_list[FDV_INS].PAPI_event_name);
        }
    } else if (IS_EVENT_AVAILABLE(FP_INS)) {
        USE_EVENT(FP_INS);
        sprintf(temp_str, "%s", event_list[FP_INS].PAPI_event_name);
    } else if (IS_EVENT_AVAILABLE(FP_COMP_OPS_EXE_SSE_FP_PACKED_DOUBLE) &&
        IS_EVENT_AVAILABLE(FP_COMP_OPS_EXE_SSE_FP_SCALAR_SINGLE) &&
        IS_EVENT_AVAILABLE(FP_COMP_OPS_EXE_SSE_PACKED_SINGLE) &&
        IS_EVENT_AVAILABLE(FP_COMP_OPS_EXE_SSE_SCALAR_DOUBLE)) {
        if (0 == test_counter(FP_COMP_OPS_EXE_SSE_FP_SCALAR_SINGLE_SSE_SCALAR_DOUBLE)) {
            DOWNGRADE_EVENT(FP_COMP_OPS_EXE_SSE_FP_SCALAR);
            DOWNGRADE_EVENT(FP_COMP_OPS_EXE_SSE_SCALAR_DOUBLE);
            USE_EVENT(FP_COMP_OPS_EXE_SSE_FP_SCALAR_SINGLE_SSE_SCALAR_DOUBLE);
            USE_EVENT(FP_COMP_OPS_EXE_SSE_FP_PACKED_DOUBLE);
            USE_EVENT(FP_COMP_OPS_EXE_SSE_PACKED_SINGLE);
            
            sprintf(temp_str, "%s + %s + %s",
                event_list[FP_COMP_OPS_EXE_SSE_PACKED_SINGLE].PAPI_event_name,
                event_list[FP_COMP_OPS_EXE_SSE_FP_PACKED_DOUBLE].PAPI_event_name,
                event_list[FP_COMP_OPS_EXE_SSE_FP_SCALAR_SINGLE_SSE_SCALAR_DOUBLE].PAPI_event_name);
        } else {
            USE_EVENT(FP_COMP_OPS_EXE_SSE_FP_SCALAR);
            USE_EVENT(FP_COMP_OPS_EXE_SSE_SCALAR_DOUBLE);
            USE_EVENT(FP_COMP_OPS_EXE_SSE_FP_PACKED_DOUBLE);
            USE_EVENT(FP_COMP_OPS_EXE_SSE_PACKED_SINGLE);

            sprintf(temp_str, "%s + %s + %s + %s",
                event_list[FP_COMP_OPS_EXE_SSE_PACKED_SINGLE].PAPI_event_name,
                event_list[FP_COMP_OPS_EXE_SSE_FP_PACKED_DOUBLE].PAPI_event_name,
                event_list[FP_COMP_OPS_EXE_SSE_FP_SCALAR_SINGLE].PAPI_event_name,
                event_list[FP_COMP_OPS_EXE_SSE_SCALAR_DOUBLE].PAPI_event_name);
        }
    } else {
        counter_err("ratio_floating_point");
    }
    sprintf(output, "ratio.floating_point = %s / PAPI_TOT_INS", temp_str);
}

/* get_overall */
void get_overall(char *output) {
    /* Check the basic ones first */
    if (!IS_EVENT_AVAILABLE(TOT_CYC)) {
        counter_err(event_list[TOT_CYC].PAPI_event_name);
    }
    USE_EVENT(TOT_CYC);

    if (!IS_EVENT_AVAILABLE(TOT_INS)) {
        counter_err(event_list[TOT_INS].PAPI_event_name);
    }
    USE_EVENT(TOT_INS);

    strcpy(output, "overall = PAPI_TOT_CYC / PAPI_TOT_INS");
}

/* counter_err */
void counter_err(char *lcpi) {
    fprintf(stderr, "Error: required counters not found for LCPI: %s\n", lcpi);
}

/* test_counter */
int test_counter(unsigned counter) {
    int ret;
    int event_set = PAPI_NULL;
    int papi_tot_ins_code;
    int event_code;

    if (PAPI_OK != (ret = PAPI_create_eventset(&event_set))) {
        fprintf(stderr, "Could not create PAPI event set\n");
        fprintf(stderr, "PAPI error message: %s.\n", PAPI_strerror(ret));
        return 1;
    }

    /* Add PAPI_TOT_INS as we will be measuring it in every run */
    PAPI_event_name_to_code("PAPI_TOT_INS", &papi_tot_ins_code);
    if (PAPI_OK != (ret = PAPI_add_event(event_set, papi_tot_ins_code))) {
        fprintf(stderr, "Could not add PAPI_TOT_INS to the event set\n");
        fprintf(stderr, "PAPI error message: %s.\n", PAPI_strerror(ret));
        return 1;
    }

    if ((PAPI_OK == PAPI_event_name_to_code(event_list[counter].PAPI_event_name,
        &event_code)) && (PAPI_OK == PAPI_add_event(event_set, event_code))) {
        /* Good! It can be added */
        return 0;
    }

    /* Fail... */
    return -1;
}

/* is_derived */
int is_derived(int event_code) {
    PAPI_event_info_t info;

    if (PAPI_OK != PAPI_get_event_info(event_code, &info)) {
        return -1;
    }
    return (0 != strlen(info.derived)) &&
        (0 != strcmp(info.derived, "NOT_DERIVED")) &&
        (0 != strcmp(info.derived, "DERIVED_CMPD"));
}

/* get_prime */
int get_prime(unsigned short sampling_category, unsigned int start) {
    unsigned short flag;
    unsigned int i, j, end;

    if (0 == start) {
        start = sampling_limits[sampling_category + 1];
    }
    end = sampling_limits[sampling_category];

    for (i = start + 1; i <= end; i++) {
        flag = 0;
        for (j = 2; j <= i / 2; j++) {
            if (0 == i%j) {
                flag = 1;
                break;
            }
        }
        if (0 == flag) {
            return i;
        }
    }
    return -1;
}

/* attach_sampling_rate */
int attach_sampling_rate(int categories[SAMPLING_CATEGORY_COUNT]) {
    int i, code, cat;
    
    for (i = 0; i < EVENT_COUNT; i++) {
        if ((PAPI_OK == PAPI_event_name_to_code(event_list[i].PAPI_event_name,
            &code)) && (PAPI_OK == PAPI_query_event(code))) {
            if (!is_derived(code)) {

                cat = event_list[i].sampling_category_id;
                event_list[i].use = EVENT_PRESENT;
                event_list[i].PAPI_event_code = code;

                if (0 >= (categories[cat] = get_prime(cat, categories[cat]))) {
                    fprintf(stderr, "Failed to generate a prime number\n");
                    return -1;
                }
                event_list[i].sampling_freq = categories[cat];
            }
        }
    }
}
