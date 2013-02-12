/*
 * Copyright (C) 2013 The University of Texas at Austin
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
 * Author: Ashay Rane
 */

#include <papi.h>
#include <stdio.h>
#include <string.h>

#include "typeDefinitions.h"
#include "eventDefinitions.h"

#define LCPI_DEF_LENGTH 600

void counter_err(char* lcpi) {
    fprintf(stderr, "Error: required counters not found for LCPI: %s\n", lcpi);
}

int testCounter(unsigned ctr) {
    int event_set = PAPI_NULL;
    int papi_tot_ins_code;
    int event_code;

    if (PAPI_OK != PAPI_create_eventset(&event_set)) {
        fprintf(stderr, "Could not create PAPI event set for determining which counters can be counted simultaneously\n");
        return 1;
    }

    /* Add PAPI_TOT_INS as we will be measuring it in every run */
    PAPI_event_name_to_code("PAPI_TOT_INS", &papi_tot_ins_code);
    if (PAPI_OK != PAPI_add_event(event_set, papi_tot_ins_code)) {
        fprintf(stderr,
                "Could not add PAPI_TOT_INS to the event set, cannot continue\n");
        return 1;
    }

    if ((PAPI_OK == PAPI_event_name_to_code(event_list[ctr].PAPI_event_name,
                                            &event_code)) &&
        (PAPI_OK == PAPI_add_event(event_set, event_code))) {
        /* Good! It can be added */
        return 0;
    }

    /* Fail... */
    return -1;
}

/* From PAPI source */
int is_derived(int event_code) {
    PAPI_event_info_t info;

    if (PAPI_OK != PAPI_get_event_info(event_code, &info)) {
        return -1;
    }
    return (0 != strlen(info.derived)) &&
           (0 != strcmp(info.derived, "NOT_DERIVED")) &&
           (0 != strcmp(info.derived, "DERIVED_CMPD"));
}

int generate_prime(unsigned short sampling_category, unsigned int start) {
    unsigned short flag;
    unsigned int i, j, end;

    if (0 == start) {
        start = sampling_limits[sampling_category + 1];
    }
    end = sampling_limits[sampling_category];

    for(i = start + 1; i <= end; i++) {
        flag = 0;
        for(j = 2; j <= i / 2; j++) {
            if(0 == i%j) {
                flag=1;
                break;
            }
        }
        if (0 == flag) {
            return i;
        }
    }
    return -1;
}

int attach_sampling_frequencies(int saved_sampling_freq[SAMPLING_CATEGORY_COUNT]) {
    int i, code, cat;
    
    for (i = 0; i < EVENT_COUNT; i++) {
        if ((PAPI_OK == PAPI_event_name_to_code(event_list[i].PAPI_event_name,
                                                &code)) &&
            (PAPI_OK == PAPI_query_event(code))) {
            if (!is_derived(code)) {
                // Mark as available and associate sampling frequency
                // printf ("%s is available\n", event_list[i].PAPI_event_name);

                cat = event_list[i].sampling_category_id;
                event_list[i].use = EVENT_PRESENT, event_list[i].PAPI_event_code = code;
                if (0 >= (saved_sampling_freq[cat] = generate_prime(cat, saved_sampling_freq[cat]))) {
                    fprintf(stderr,
                            "Failed to generate a prime number within given range\n");
                    return -1;
                }
                event_list[i].sampling_freq = saved_sampling_freq[cat];
            }
        }
    }
}

int main(int argc, char* argv []) {
    FILE* fp;
    int i;
    int saved_sampling_freq[SAMPLING_CATEGORY_COUNT] = { 0 };
    int event_code;
    int exp_count = 0;
    int papi_tot_ins_code;
    short addCount, remaining;
    char num[LCPI_DEF_LENGTH];
    char ratio_floating_point[LCPI_DEF_LENGTH];
    char ratio_data_accesses[LCPI_DEF_LENGTH];
    char gflops_overall[LCPI_DEF_LENGTH] = { 0 };
    char gflops_packed[LCPI_DEF_LENGTH] = { 0 };
    char gflops_scalar[LCPI_DEF_LENGTH] = { 0 };
    char overall[LCPI_DEF_LENGTH];
    char lcpi_L2_DCA[LCPI_DEF_LENGTH];
    char lcpi_L2_DCM[LCPI_DEF_LENGTH];
    char data_accesses_overall[LCPI_DEF_LENGTH];
    char data_accesses_L1d_hits[LCPI_DEF_LENGTH];
    char data_accesses_L2d_hits[LCPI_DEF_LENGTH];
    char data_accesses_L2d_misses[LCPI_DEF_LENGTH];
    char lcpi_L2_ICA[LCPI_DEF_LENGTH];
    char lcpi_L2_ICM[LCPI_DEF_LENGTH];
    char instruction_accesses_overall[LCPI_DEF_LENGTH];
    char instruction_accesses_L1i_hits[LCPI_DEF_LENGTH];
    char l1i_hits[LCPI_DEF_LENGTH];
    char instruction_accesses_L2i_hits[LCPI_DEF_LENGTH];
    char instruction_accesses_L2i_misses[LCPI_DEF_LENGTH];
    char data_TLB_overall[LCPI_DEF_LENGTH];
    char instruction_TLB_overall[LCPI_DEF_LENGTH];
    char branch_instructions_overall[LCPI_DEF_LENGTH];
    char branch_instructions_correctly_predicted[LCPI_DEF_LENGTH];
    char branch_instructions_mispredicted[LCPI_DEF_LENGTH];
    char lcpi_slow[LCPI_DEF_LENGTH];
    char lcpi_fast[LCPI_DEF_LENGTH];
    char floating_point_instr_overall [LCPI_DEF_LENGTH];
    char floating_point_instr_fast_FP_instr[LCPI_DEF_LENGTH];
    char floating_point_instr_slow_FP_instr[LCPI_DEF_LENGTH];

    if (PAPI_VER_CURRENT != PAPI_library_init(PAPI_VER_CURRENT)) {
        fprintf(stderr, "Failed initializing PAPI\n");
        return 1;
    }

    attach_sampling_frequencies(saved_sampling_freq);

    /* Check the basic ones first, so that we don't have to repeat their checks
     * every time we use them
     */
    if (!IS_EVENT_AVAILABLE(TOT_CYC)) {
        counter_err(event_list[TOT_CYC].PAPI_event_name);
    }
    USE_EVENT(TOT_CYC);

    if (!IS_EVENT_AVAILABLE(TOT_INS)) {
        counter_err(event_list[TOT_INS].PAPI_event_name);
    }
    USE_EVENT(TOT_INS);

    /* ratio.floating_point */
    bzero(num, LCPI_DEF_LENGTH);
    if (IS_EVENT_AVAILABLE(FML_INS) && IS_EVENT_AVAILABLE(FDV_INS)) {
        USE_EVENT(FML_INS);
        USE_EVENT(FDV_INS);
        
        if (IS_EVENT_AVAILABLE(FAD_INS)) {
            USE_EVENT(FAD_INS);
            sprintf(num, "(%s + %s + %s)",
                    event_list[FML_INS].PAPI_event_name,
                    event_list[FDV_INS].PAPI_event_name,
                    event_list[FAD_INS].PAPI_event_name);
        } else {
            sprintf(num, "(%s + %s)",
                    event_list[FML_INS].PAPI_event_name,
                    event_list[FDV_INS].PAPI_event_name);
        }
    } else if (IS_EVENT_AVAILABLE(FP_INS)) {
        USE_EVENT(FP_INS);
        sprintf(num, "%s", event_list[FP_INS].PAPI_event_name);
    } else if (IS_EVENT_AVAILABLE(FP_COMP_OPS_EXE_SSE_FP_PACKED_DOUBLE) &&
               IS_EVENT_AVAILABLE(FP_COMP_OPS_EXE_SSE_FP_SCALAR_SINGLE) &&
               IS_EVENT_AVAILABLE(FP_COMP_OPS_EXE_SSE_PACKED_SINGLE) &&
               IS_EVENT_AVAILABLE(FP_COMP_OPS_EXE_SSE_SCALAR_DOUBLE)) {
        if (0 == testCounter(FP_COMP_OPS_EXE_SSE_FP_SCALAR_SINGLE_SSE_SCALAR_DOUBLE)) {
            DOWNGRADE_EVENT(FP_COMP_OPS_EXE_SSE_FP_SCALAR);
            DOWNGRADE_EVENT(FP_COMP_OPS_EXE_SSE_SCALAR_DOUBLE);
            USE_EVENT(FP_COMP_OPS_EXE_SSE_FP_SCALAR_SINGLE_SSE_SCALAR_DOUBLE);
            USE_EVENT(FP_COMP_OPS_EXE_SSE_FP_PACKED_DOUBLE);
            USE_EVENT(FP_COMP_OPS_EXE_SSE_PACKED_SINGLE);
            
            sprintf(num, "%s + %s + %s",
                    event_list[FP_COMP_OPS_EXE_SSE_PACKED_SINGLE].PAPI_event_name,
                    event_list[FP_COMP_OPS_EXE_SSE_FP_PACKED_DOUBLE].PAPI_event_name,
                    event_list[FP_COMP_OPS_EXE_SSE_FP_SCALAR_SINGLE_SSE_SCALAR_DOUBLE].PAPI_event_name);
        } else {
            USE_EVENT(FP_COMP_OPS_EXE_SSE_FP_SCALAR);
            USE_EVENT(FP_COMP_OPS_EXE_SSE_SCALAR_DOUBLE);
            USE_EVENT(FP_COMP_OPS_EXE_SSE_FP_PACKED_DOUBLE);
            USE_EVENT(FP_COMP_OPS_EXE_SSE_PACKED_SINGLE);
            
            sprintf(num, "%s + %s + %s + %s",
                    event_list[FP_COMP_OPS_EXE_SSE_PACKED_SINGLE].PAPI_event_name,
                    event_list[FP_COMP_OPS_EXE_SSE_FP_PACKED_DOUBLE].PAPI_event_name,
                    event_list[FP_COMP_OPS_EXE_SSE_FP_SCALAR_SINGLE].PAPI_event_name,
                    event_list[FP_COMP_OPS_EXE_SSE_SCALAR_DOUBLE].PAPI_event_name);
        }
        
    } else {
        counter_err("ratio.floating_point");
    }
    sprintf(ratio_floating_point, "ratio.floating_point = %s / PAPI_TOT_INS\n",
            num);

    /* ratio.data_accesses */
    bzero(num, LCPI_DEF_LENGTH);
    if (IS_EVENT_AVAILABLE(L1_DCA)) {
        USE_EVENT(L1_DCA);
        sprintf(num, "%s", event_list[L1_DCA].PAPI_event_name);
    } else if (IS_EVENT_AVAILABLE(LD_INS)) {
        USE_EVENT(LD_INS);
        sprintf(num, "%s", event_list[LD_INS].PAPI_event_name);
    } else {
        counter_err("data_accesses");
    }
    sprintf(ratio_data_accesses, "ratio.data_accesses = %s / PAPI_TOT_INS\n\n",
            num);

    /* GFLOPS */
    if (IS_EVENT_AVAILABLE(RETIRED_SSE_OPERATIONS_ALL)) {
        USE_EVENT(RETIRED_SSE_OPERATIONS_ALL);
        
        sprintf(gflops_overall, "percent.GFLOPS_(%%_max).overall = (%s / PAPI_TOT_CYC) / 4\n",
                event_list[RETIRED_SSE_OPERATIONS_ALL].PAPI_event_name);
        
        strcpy(gflops_packed, "");
        strcpy(gflops_scalar, "\n");
    } else if (IS_EVENT_AVAILABLE(RETIRED_SSE_OPS_ALL)) {
        USE_EVENT(RETIRED_SSE_OPS_ALL);

        sprintf(gflops_overall, "percent.GFLOPS_(%%_max).overall = (%s / PAPI_TOT_CYC) / 4\n",
                event_list[RETIRED_SSE_OPS_ALL].PAPI_event_name);
        
        strcpy(gflops_packed, "");
        strcpy(gflops_scalar, "\n");
    } else if (IS_EVENT_AVAILABLE(SSEX_UOPS_RETIRED_PACKED_DOUBLE) &&
               IS_EVENT_AVAILABLE(SSEX_UOPS_RETIRED_PACKED_SINGLE) &&
               IS_EVENT_AVAILABLE(SSEX_UOPS_RETIRED_SCALAR_DOUBLE) &&
               IS_EVENT_AVAILABLE(SSEX_UOPS_RETIRED_SCALAR_SINGLE)) {
        // Do a dummy test to see if the single big counter can be added or do we have to break it down into multiple parts
        if (0 == testCounter(SSEX_UOPS_RETIRED_SCALAR_DOUBLE_SCALAR_SINGLE)) {
            USE_EVENT(SSEX_UOPS_RETIRED_PACKED_DOUBLE);
            USE_EVENT(SSEX_UOPS_RETIRED_PACKED_SINGLE);
            USE_EVENT(SSEX_UOPS_RETIRED_SCALAR_DOUBLE_SCALAR_SINGLE);
            DOWNGRADE_EVENT(SSEX_UOPS_RETIRED_SCALAR_DOUBLE);
            DOWNGRADE_EVENT(SSEX_UOPS_RETIRED_SCALAR_SINGLE);
            
            sprintf(gflops_overall, "percent.GFLOPS_(%%_max).overall = ((%s*2 + %s*4 + %s) / PAPI_TOT_CYC) / 4\n",
                    event_list[SSEX_UOPS_RETIRED_PACKED_DOUBLE].PAPI_event_name,
                    event_list[SSEX_UOPS_RETIRED_PACKED_SINGLE].PAPI_event_name,
                    event_list[SSEX_UOPS_RETIRED_SCALAR_DOUBLE_SCALAR_SINGLE].PAPI_event_name);
            
            sprintf(gflops_packed,  "percent.GFLOPS_(%%_max).packed = ((%s*2 + %s*4) / PAPI_TOT_CYC) / 4\n",
                    event_list[SSEX_UOPS_RETIRED_PACKED_DOUBLE].PAPI_event_name,
                    event_list[SSEX_UOPS_RETIRED_PACKED_SINGLE].PAPI_event_name);
            
            sprintf(gflops_scalar,  "percent.GFLOPS_(%%_max).scalar = (%s / PAPI_TOT_CYC) / 4\n\n",
                    event_list[SSEX_UOPS_RETIRED_SCALAR_DOUBLE_SCALAR_SINGLE].PAPI_event_name);
        } else {
            // Asume that we can add separately. If this is false, we will catch it at the time of generating LCPIs
            // Use the available counters
            USE_EVENT(SSEX_UOPS_RETIRED_PACKED_DOUBLE);
            USE_EVENT(SSEX_UOPS_RETIRED_PACKED_SINGLE);
            USE_EVENT(SSEX_UOPS_RETIRED_SCALAR_DOUBLE);
            USE_EVENT(SSEX_UOPS_RETIRED_SCALAR_SINGLE);
            
            sprintf(gflops_overall, "percent.GFLOPS_(%%_max).overall = ((%s*2 + %s*4 + %s + %s) / PAPI_TOT_CYC) / 4\n",
                    event_list[SSEX_UOPS_RETIRED_PACKED_DOUBLE].PAPI_event_name,
                    event_list[SSEX_UOPS_RETIRED_PACKED_SINGLE].PAPI_event_name,
                    event_list[SSEX_UOPS_RETIRED_SCALAR_SINGLE].PAPI_event_name,
                    event_list[SSEX_UOPS_RETIRED_SCALAR_DOUBLE].PAPI_event_name);
            
            sprintf(gflops_packed,  "percent.GFLOPS_(%%_max).packed = ((%s*2 + %s*4) / PAPI_TOT_CYC) / 4\n",
                    event_list[SSEX_UOPS_RETIRED_PACKED_DOUBLE].PAPI_event_name,
                    event_list[SSEX_UOPS_RETIRED_PACKED_SINGLE].PAPI_event_name);
            
            sprintf(gflops_scalar,  "percent.GFLOPS_(%%_max).scalar = (%s + %s / PAPI_TOT_CYC) / 4\n\n",
                    event_list[SSEX_UOPS_RETIRED_SCALAR_DOUBLE].PAPI_event_name,
                    event_list[SSEX_UOPS_RETIRED_SCALAR_SINGLE].PAPI_event_name);
        }
    } else if (IS_EVENT_AVAILABLE(SIMD_COMP_INST_RETIRED_PACKED_DOUBLE) &&
               IS_EVENT_AVAILABLE(SIMD_COMP_INST_RETIRED_PACKED_SINGLE) &&
               IS_EVENT_AVAILABLE(SIMD_COMP_INST_RETIRED_SCALAR_DOUBLE) &&
               IS_EVENT_AVAILABLE(SIMD_COMP_INST_RETIRED_SCALAR_SINGLE)) {
        // Do a dummy test to see if the single big counter can be added or do we have to break it down into multiple parts
        if (0 == testCounter(SIMD_COMP_INST_RETIRED_SCALAR_SINGLE_SCALAR_DOUBLE)) {
            USE_EVENT(SIMD_COMP_INST_RETIRED_PACKED_DOUBLE);
            USE_EVENT(SIMD_COMP_INST_RETIRED_PACKED_SINGLE);
            DOWNGRADE_EVENT(SIMD_COMP_INST_RETIRED_SCALAR_DOUBLE);
            DOWNGRADE_EVENT(SIMD_COMP_INST_RETIRED_SCALAR_SINGLE);
            USE_EVENT(SIMD_COMP_INST_RETIRED_SCALAR_SINGLE_SCALAR_DOUBLE);
            
            sprintf(gflops_overall, "percent.GFLOPS_(%%_max).overall = ((%s*2 + %s*4 + %s) / PAPI_TOT_CYC) / 4\n",
                    event_list[SIMD_COMP_INST_RETIRED_PACKED_DOUBLE].PAPI_event_name,
                    event_list[SIMD_COMP_INST_RETIRED_PACKED_SINGLE].PAPI_event_name,
                    event_list[SIMD_COMP_INST_RETIRED_SCALAR_SINGLE_SCALAR_DOUBLE].PAPI_event_name);
            
            sprintf(gflops_packed,  "percent.GFLOPS_(%%_max).packed = ((%s*2 + %s*4) / PAPI_TOT_CYC) / 4\n",
                    event_list[SIMD_COMP_INST_RETIRED_PACKED_DOUBLE].PAPI_event_name,
                    event_list[SIMD_COMP_INST_RETIRED_PACKED_SINGLE].PAPI_event_name);
            
            sprintf(gflops_scalar,  "percent.GFLOPS_(%%_max).scalar = (%s / PAPI_TOT_CYC) / 4\n",
                    event_list[SIMD_COMP_INST_RETIRED_SCALAR_SINGLE_SCALAR_DOUBLE].PAPI_event_name);
        } else {
            USE_EVENT(SIMD_COMP_INST_RETIRED_PACKED_DOUBLE);
            USE_EVENT(SIMD_COMP_INST_RETIRED_PACKED_SINGLE);
            USE_EVENT(SIMD_COMP_INST_RETIRED_SCALAR_DOUBLE);
            USE_EVENT(SIMD_COMP_INST_RETIRED_SCALAR_SINGLE);
            
            sprintf(gflops_overall, "percent.GFLOPS_(%%_max).overall = ((%s*2 + %s*4 + %s + %s) / PAPI_TOT_CYC) / 4\n",
                    event_list[SIMD_COMP_INST_RETIRED_PACKED_DOUBLE].PAPI_event_name,
                    event_list[SIMD_COMP_INST_RETIRED_PACKED_SINGLE].PAPI_event_name,
                    event_list[SIMD_COMP_INST_RETIRED_SCALAR_SINGLE].PAPI_event_name,
                    event_list[SIMD_COMP_INST_RETIRED_SCALAR_DOUBLE].PAPI_event_name);
            
            sprintf(gflops_packed,  "percent.GFLOPS_(%%_max).packed = ((%s*2 + %s*4) / PAPI_TOT_CYC) / 4\n",
                    event_list[SIMD_COMP_INST_RETIRED_PACKED_DOUBLE].PAPI_event_name,
                    event_list[SIMD_COMP_INST_RETIRED_PACKED_SINGLE].PAPI_event_name);
            
            sprintf(gflops_scalar,  "percent.GFLOPS_(%%_max).scalar = ((%s + %s)/ PAPI_TOT_CYC) / 4\n",
                    event_list[SIMD_COMP_INST_RETIRED_SCALAR_SINGLE].PAPI_event_name,
                    event_list[SIMD_COMP_INST_RETIRED_SCALAR_DOUBLE].PAPI_event_name);
        }
    } else if (IS_EVENT_AVAILABLE(SIMD_FP_256_PACKED_SINGLE) &&
               IS_EVENT_AVAILABLE(SIMD_FP_256_PACKED_DOUBLE) &&
               IS_EVENT_AVAILABLE(FP_COMP_OPS_EXE_SSE_FP_PACKED_DOUBLE) &&
               IS_EVENT_AVAILABLE(FP_COMP_OPS_EXE_SSE_FP_SCALAR_SINGLE) &&
               IS_EVENT_AVAILABLE(FP_COMP_OPS_EXE_SSE_PACKED_SINGLE) &&
               IS_EVENT_AVAILABLE(FP_COMP_OPS_EXE_SSE_SCALAR_DOUBLE)) {
        if (0 == testCounter(FP_COMP_OPS_EXE_SSE_FP_SCALAR_SINGLE_SSE_SCALAR_DOUBLE)) {
            DOWNGRADE_EVENT(FP_COMP_OPS_EXE_SSE_FP_SCALAR);
            DOWNGRADE_EVENT(FP_COMP_OPS_EXE_SSE_SCALAR_DOUBLE);
            USE_EVENT(FP_COMP_OPS_EXE_SSE_FP_SCALAR_SINGLE_SSE_SCALAR_DOUBLE);
            USE_EVENT(SIMD_FP_256_PACKED_SINGLE);
            USE_EVENT(SIMD_FP_256_PACKED_DOUBLE);
            USE_EVENT(FP_COMP_OPS_EXE_SSE_FP_PACKED_DOUBLE);
            USE_EVENT(FP_COMP_OPS_EXE_SSE_PACKED_SINGLE);
            
            sprintf(gflops_overall, "percent.GFLOPS_(%%_max).overall = ((%s*8 + (%s + %s)*4 + %s*2 + %s) / PAPI_TOT_CYC) / 8\n",
                    event_list[SIMD_FP_256_PACKED_SINGLE].PAPI_event_name,
                    event_list[SIMD_FP_256_PACKED_DOUBLE].PAPI_event_name,
                    event_list[FP_COMP_OPS_EXE_SSE_PACKED_SINGLE].PAPI_event_name,
                    event_list[FP_COMP_OPS_EXE_SSE_FP_PACKED_DOUBLE].PAPI_event_name,
                    event_list[FP_COMP_OPS_EXE_SSE_FP_SCALAR_SINGLE_SSE_SCALAR_DOUBLE].PAPI_event_name);
            
            sprintf(gflops_packed, "percent.GFLOPS_(%%_max).packed = ((%s*8 + (%s + %s)*4 + %s*2) / PAPI_TOT_CYC) / 8\n",
                    event_list[SIMD_FP_256_PACKED_SINGLE].PAPI_event_name,
                    event_list[SIMD_FP_256_PACKED_DOUBLE].PAPI_event_name,
                    event_list[FP_COMP_OPS_EXE_SSE_PACKED_SINGLE].PAPI_event_name,
                    event_list[FP_COMP_OPS_EXE_SSE_FP_PACKED_DOUBLE].PAPI_event_name);
            
            sprintf(gflops_scalar, "percent.GFLOPS_(%%_max).scalar = (%s / PAPI_TOT_CYC) / 8\n",
                    event_list[FP_COMP_OPS_EXE_SSE_FP_SCALAR_SINGLE_SSE_SCALAR_DOUBLE].PAPI_event_name);
        } else {
            USE_EVENT(FP_COMP_OPS_EXE_SSE_FP_SCALAR);
            USE_EVENT(FP_COMP_OPS_EXE_SSE_SCALAR_DOUBLE);
            USE_EVENT(SIMD_FP_256_PACKED_SINGLE);
            USE_EVENT(SIMD_FP_256_PACKED_DOUBLE);
            USE_EVENT(FP_COMP_OPS_EXE_SSE_FP_PACKED_DOUBLE);
            USE_EVENT(FP_COMP_OPS_EXE_SSE_PACKED_SINGLE);
            
            sprintf(gflops_overall, "percent.GFLOPS_(%%_max).overall = ((%s*8 + (%s + %s)*4 + %s*2 + %s + %s) / PAPI_TOT_CYC) / 8\n",
                    event_list[SIMD_FP_256_PACKED_SINGLE].PAPI_event_name,
                    event_list[SIMD_FP_256_PACKED_DOUBLE].PAPI_event_name,
                    event_list[FP_COMP_OPS_EXE_SSE_PACKED_SINGLE].PAPI_event_name,
                    event_list[FP_COMP_OPS_EXE_SSE_FP_PACKED_DOUBLE].PAPI_event_name,
                    event_list[FP_COMP_OPS_EXE_SSE_FP_SCALAR_SINGLE].PAPI_event_name,
                    event_list[FP_COMP_OPS_EXE_SSE_SCALAR_DOUBLE].PAPI_event_name);
            
            sprintf(gflops_packed, "percent.GFLOPS_(%%_max).packed = ((%s*8 + (%s + %s)*4 + %s*2) / PAPI_TOT_CYC) / 8\n",
                    event_list[SIMD_FP_256_PACKED_SINGLE].PAPI_event_name,
                    event_list[SIMD_FP_256_PACKED_DOUBLE].PAPI_event_name,
                    event_list[FP_COMP_OPS_EXE_SSE_PACKED_SINGLE].PAPI_event_name,
                    event_list[FP_COMP_OPS_EXE_SSE_FP_PACKED_DOUBLE].PAPI_event_name);
            
            sprintf(gflops_scalar, "percent.GFLOPS_(%%_max).scalar = ((%s + %s) / PAPI_TOT_CYC) / 8\n",
                    event_list[FP_COMP_OPS_EXE_SSE_FP_SCALAR_SINGLE].PAPI_event_name,
                    event_list[FP_COMP_OPS_EXE_SSE_SCALAR_DOUBLE].PAPI_event_name);
        }
    } else {
        counter_err("GFLOPS");
    }
    
    /* overall */
    strcpy(overall, "overall = PAPI_TOT_CYC / PAPI_TOT_INS\n\n");

    /* data_accesses */
    if (IS_EVENT_AVAILABLE(L2_DCA)) {
        USE_EVENT(L2_DCA);
        strcpy(lcpi_L2_DCA, "PAPI_L2_DCA");
    } else if (IS_EVENT_AVAILABLE(L2_TCA) && IS_EVENT_AVAILABLE(L2_ICA)) {
        USE_EVENT(L2_TCA);
        USE_EVENT(L2_ICA);
        strcpy(lcpi_L2_DCA, "(PAPI_L2_TCA - PAPI_L2_ICA)");
    } else {
        counter_err("PAPI_L2_DCA");
    }
    
    if (IS_EVENT_AVAILABLE(L2_DCM)) {
        USE_EVENT(L2_DCM);
        strcpy(lcpi_L2_DCM, "PAPI_L2_DCM");
    } else if (IS_EVENT_AVAILABLE(L2_TCM) && IS_EVENT_AVAILABLE(L2_ICM)) {
        USE_EVENT(L2_TCM);
        USE_EVENT(L2_ICM);
        strcpy(lcpi_L2_DCM, "(PAPI_L2_TCM - PAPI_L2_ICM)");
    } else {
        counter_err("PAPI_L2_DCM");
    }
    
    if (IS_EVENT_AVAILABLE(L1_DCA)) {
        USE_EVENT(L1_DCA);
        sprintf(data_accesses_overall,
                "data_accesses.overall = (PAPI_L1_DCA * L1_dlat + %s * L2_lat + %s * mem_lat) / PAPI_TOT_INS\n",
                lcpi_L2_DCA, lcpi_L2_DCM);
        strcpy(data_accesses_L1d_hits,
               "data_accesses.L1d_hits = (PAPI_L1_DCA * L1_dlat) / PAPI_TOT_INS\n");
    } else if (IS_EVENT_AVAILABLE(LD_INS)) {
        USE_EVENT(LD_INS);
        sprintf(data_accesses_overall,
                "data_accesses.overall = (PAPI_LD_INS * L1_dlat + %s * L2_lat + %s * mem_lat) / PAPI_TOT_INS\n",
                lcpi_L2_DCA, lcpi_L2_DCM);
        strcpy(data_accesses_L1d_hits,
               "data_accesses.L1d_hits = (PAPI_LD_INS * L1_dlat) / PAPI_TOT_INS\n");
    } else {
        counter_err("data_accesses_L1d_hits");
    }
    
    sprintf(data_accesses_L2d_hits, "data_accesses.L2d_hits = (%s * L2_lat) / PAPI_TOT_INS\n", lcpi_L2_DCA);

    sprintf(data_accesses_L2d_misses, "data_accesses.L2d_misses = (%s * mem_lat) / PAPI_TOT_INS\n\n", lcpi_L2_DCM);

    /* instruction_accesses */
    if (IS_EVENT_AVAILABLE(L1_ICA)) {
        USE_EVENT(L1_ICA);
        sprintf(l1i_hits, "%s", event_list[L1_ICA].PAPI_event_name);
    } else if (IS_EVENT_AVAILABLE(ICACHE)) {
        USE_EVENT(ICACHE);
        sprintf(l1i_hits, "%s", event_list[ICACHE].PAPI_event_name);
    } else {
        counter_err("instruction_accesses.L1i_hits");
    }
    sprintf(instruction_accesses_L1i_hits,
            "instruction_accesses.L1i_hits = (%s * L1_ilat) / PAPI_TOT_INS\n",
            l1i_hits);

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
    sprintf(instruction_accesses_overall,
            "instruction_accesses.overall = (%s * L1_ilat + %s * L2_lat + %s * mem_lat) / PAPI_TOT_INS\n",
            l1i_hits, lcpi_L2_ICA, lcpi_L2_ICM);
    
    sprintf(instruction_accesses_L2i_hits,
            "instruction_accesses.L2i_hits = %s * L2_lat / PAPI_TOT_INS\n",
            lcpi_L2_ICA);
    
    sprintf(instruction_accesses_L2i_misses,
            "instruction_accesses.L2i_misses = %s * mem_lat / PAPI_TOT_INS\n\n",
            lcpi_L2_ICM);

    /* data_TLB */
    if (IS_EVENT_AVAILABLE(TLB_DM)) {
        USE_EVENT(TLB_DM);
        strcpy(data_TLB_overall,
               "data_TLB.overall = PAPI_TLB_DM * TLB_lat / PAPI_TOT_INS\n");
    } else if (IS_EVENT_AVAILABLE(DTLB_LOAD_MISSES_CAUSES_A_WALK) &&
               IS_EVENT_AVAILABLE(DTLB_STORE_MISSES_CAUSES_A_WALK)) {
        USE_EVENT(DTLB_LOAD_MISSES_CAUSES_A_WALK);
        USE_EVENT(DTLB_STORE_MISSES_CAUSES_A_WALK);
        strcpy(data_TLB_overall,
               "data_TLB.overall = (DTLB_LOAD_MISSES:CAUSES_A_WALK + DTLB_STORE_MISSES:CAUSES_A_WALK) * TLB_lat / PAPI_TOT_INS\n");
    } else {
        counter_err("PAPI_TLB_DM");
    }
    
    /* instruction_TLB */
    if (IS_EVENT_AVAILABLE(TLB_IM)) {
        USE_EVENT(TLB_IM);
        strcpy(instruction_TLB_overall,
               "instruction_TLB.overall = PAPI_TLB_IM * TLB_lat / PAPI_TOT_INS\n\n");
    } else {
        counter_err("PAPI_TLB_IM");
    }
    
    /* branch_instructions */

    if (IS_EVENT_AVAILABLE(BR_INS) && IS_EVENT_AVAILABLE(BR_MSP)) {
        USE_EVENT(BR_INS);
        USE_EVENT(BR_MSP);
        strcpy(branch_instructions_overall,
               "branch_instructions.overall = (PAPI_BR_INS * BR_lat + PAPI_BR_MSP * BR_miss_lat) / PAPI_TOT_INS\n");
        strcpy(branch_instructions_correctly_predicted,
               "branch_instructions.correctly_predicted = PAPI_BR_INS * BR_lat / PAPI_TOT_INS\n");
        strcpy(branch_instructions_mispredicted,
               "branch_instructions.mispredicted = PAPI_BR_MSP * BR_miss_lat / PAPI_TOT_INS\n\n");
    } else {
        counter_err("branch_instructions");
    }
    
    /* floating-point_instr */
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
        if (0 == testCounter(FP_COMP_OPS_EXE_SSE_DOUBLE_PRECISION_SSE_FP_SSE_FP_PACKED_SSE_FP_SCALAR_SSE_SINGLE_PRECISION_X87)) {
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
        if (0 == testCounter(FP_COMP_OPS_EXE_SSE_FP_SCALAR_SINGLE_SSE_SCALAR_DOUBLE)) {
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
    
    sprintf(floating_point_instr_overall, "floating-point_instr.overall = (%s + %s) / PAPI_TOT_INS\n", lcpi_fast, lcpi_slow);

    sprintf(floating_point_instr_fast_FP_instr, "floating-point_instr.fast_FP_instr = %s / PAPI_TOT_INS\n", lcpi_fast);

    sprintf(floating_point_instr_slow_FP_instr, "floating-point_instr.slow_FP_instr = %s / PAPI_TOT_INS\n", lcpi_slow);

    PAPI_event_name_to_code("PAPI_TOT_INS", &papi_tot_ins_code);

    fp = fopen("perfexpert_run_exp.sh_header", "w");
    if (NULL == fp) {
        fprintf(stderr,
                "Could not open file perfexpert_run_exp.sh_header for writing\n");
        return 1;
    }

    do {
        addCount = 0;
        remaining = 0;
        int j;
        int event_set = PAPI_NULL;
        
        if (PAPI_OK != PAPI_create_eventset(&event_set)) {
            fprintf(stderr, "Could not create PAPI event set for determining which counters can be counted simultaneously\n");
            return 1;
        }
        
        /* Add PAPI_TOT_INS as we will be measuring it in every run */
        if (PAPI_OK != PAPI_add_event(event_set, papi_tot_ins_code)) {
            fprintf(stderr, "Could not add PAPI_TOT_INS to the event set, cannot continue...\n");
            return 1;
        }
        
        for (j = 0; j < EVENT_COUNT; j++) {
            // Don't need to specially add PAPI_TOT_INS, we are measuring that anyway
            if (PAPI_TOT_INS == j) {
                continue;
            }
            
            if (IS_EVENT_USED(j) && TOT_INS != j) {
                PAPI_event_name_to_code(event_list[j].PAPI_event_name,
                                        &event_code);
                if (PAPI_OK == PAPI_add_event(event_set, event_code)) {
                    ADD_EVENT(j);
                    
                    if (0 == remaining) { // New line
                        fprintf(fp, "experiment[%d]=\"", exp_count);
                    }
                    fprintf(fp, "--event %s:%d ", event_list[j].PAPI_event_name,
                            event_list[j].sampling_freq);
                    addCount++;
                }
                remaining++;
            }
            fprintf(fp, "\"\n");
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
        if ((0 > addCount) && (0 < remaining)) {
            fprintf(fp, "--event PAPI_TOT_INS:%d\\\"\\n",
                    event_list[TOT_INS].sampling_freq);
        }
    } while (0 < remaining);

    fclose(fp);

    fp = fopen("lcpi.properties", "w");
    if (NULL == fp) {
        fprintf(stderr, "Could not open file lcpi.properties for writing\n");
        return 1;
    }

    fprintf(fp, "version = 1.0\n\n# LCPI config generated using sniffer\n");
    fprintf(fp, "%s", ratio_floating_point);
    fprintf(fp, "%s", ratio_data_accesses);
    fprintf(fp, "%s", gflops_overall);
    fprintf(fp, "%s", gflops_packed);
    fprintf(fp, "%s", gflops_scalar);
    fprintf(fp, "%s", overall);
    fprintf(fp, "%s", data_accesses_overall);
    fprintf(fp, "%s", data_accesses_L1d_hits);
    fprintf(fp, "%s", data_accesses_L2d_hits);
    fprintf(fp, "%s", data_accesses_L2d_misses);
    fprintf(fp, "%s", instruction_accesses_overall);
    fprintf(fp, "%s", instruction_accesses_L1i_hits);
    fprintf(fp, "%s", instruction_accesses_L2i_hits);
    fprintf(fp, "%s", instruction_accesses_L2i_misses);
    fprintf(fp, "%s", data_TLB_overall);
    fprintf(fp, "%s", instruction_TLB_overall);
    fprintf(fp, "%s", branch_instructions_overall);
    fprintf(fp, "%s", branch_instructions_correctly_predicted);
    fprintf(fp, "%s", branch_instructions_mispredicted);
    fprintf(fp, "%s", floating_point_instr_overall);
    fprintf(fp, "%s", floating_point_instr_fast_FP_instr);
    fprintf(fp, "%s", floating_point_instr_slow_FP_instr);

    fclose (fp);

    fprintf(stderr, "Generated LCPI and experiment files\n");
    return 0;
}
