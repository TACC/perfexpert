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

#ifdef __cplusplus
extern "C" {
#endif

/* System standard headers */
#include <string.h>
#include <papi.h>

/* Modules headers */
#include "hpctoolkit.h"
#include "hpctoolkit_papi.h"
#include "hpctoolkit_types.h"

/* Tools headers */
#include "tools/perfexpert/perfexpert_types.h"

/* PerfExpert common headers */
#include "common/perfexpert_constants.h"
#include "common/perfexpert_hash.h"
#include "common/perfexpert_output.h"

/* papi_max_events */
int papi_max_events(void) {
    int papi_rc, event_set = PAPI_NULL, event_code, n = 0;
    hpctoolkit_event_t *e = NULL, *t = NULL;

    /* Find the maximum number of events per experiment */
    if (PAPI_OK != (papi_rc = PAPI_create_eventset(&event_set))) {
        OUTPUT(("%s [%s]", _ERROR("could not create PAPI event set"),
            PAPI_strerror(papi_rc)));
        return PERFEXPERT_FAILURE;
    }

    perfexpert_hash_iter_str(my_module_globals.events_by_name, e, t) {
        if (PAPI_OK != PAPI_event_name_to_code(e->name, &event_code)) {
            OUTPUT(("%s [%s]",
                _ERROR("event not available (name->code)"), e->name));
            return PERFEXPERT_FAILURE;
        }
        if (PAPI_OK != PAPI_add_event(event_set, event_code)) {
            return n;
        }
        n++;
    }
    return n;
}

/* papi_get_sampling_rate */
int papi_get_sampling_rate(const char *name) {
    int event_code, cat = 0, rate = 0;
    PAPI_event_info_t info;

    static sample_t sample[] = {
        {99999999, 10000000, "" },
        {9999999, 5000000, "FP_COMP_OPS_EXE:SSE_FP_PACKED_DOUBLE ARITH:FPU_DIV "
            "FP_COMP_OPS_EXE:SSE_FP_SCALAR_SINGLE:SSE_SCALAR_DOUBLE "
            "PAPI_TOT_INS PAPI_TOT_CYC L1I_CYCLES_STALLED "
            "INST_RETIRED:ANY_P "
            "PAPI_LD_INS PAPI_L1_DCA PAPI_L1_ICA ICACHE ARITH:CYCLES_DIV_BUSY "
            "SSEX_UOPS_RETIRED:SCALAR_DOUBLE ARITH FP_COMP_OPS_EXE:SSE_FP "
            "FP_COMP_OPS_EXE:SSE_FP_PACKED FP_COMP_OPS_EXE:SSE_FP_SCALAR "
            "FP_COMP_OPS_EXE:SSE_SINGLE_PRECISION RETIRED_SSE_OPERATIONS:ALL "
            "FP_COMP_OPS_EXE:SSE_DOUBLE_PRECISION RETIRED_SSE_OPS:ALL "
            "SSEX_UOPS_RETIRED:PACKED_DOUBLE SSEX_UOPS_RETIRED:PACKED_SINGLE "
            "SSEX_UOPS_RETIRED:SCALAR_SINGLE FP_COMP_OPS_EXE:X87 PAPI_FDV_INS "
            "SIMD_COMP_INST_RETIRED:PACKED_DOUBLE SIMD_FP_256:PACKED_SINGLE "
            "SIMD_COMP_INST_RETIRED:PACKED_SINGLE SIMD_FP_256:PACKED_DOUBLE "
            "SIMD_COMP_INST_RETIRED:SCALAR_DOUBLE PAPI_FAD_INS PAPI_FP_INS "
            "SIMD_COMP_INST_RETIRED:SCALAR_SINGLE PAPI_FML_INS "
            "FP_COMP_OPS_EXE:SSE_FP_SCALAR_SINGLE "
            "FP_COMP_OPS_EXE:SSE_PACKED_SINGLE "
            "FP_COMP_OPS_EXE:SSE_SCALAR_DOUBLE "
            "SIMD_FP_256:PACKED_SINGLE:PACKED_DOUBLE "
            "SSEX_UOPS_RETIRED:SCALAR_DOUBLE:SCALAR_SINGLE "
            "SIMD_COMP_INST_RETIRED:SCALAR_SINGLE:SCALAR_DOUBLE "
            "FP_COMP_OPS_EXE:SSE_DOUBLE_PRECISION:SSE_FP:SSE_FP_PACKED:"
            "SSE_FP_SCALAR:SSE_SINGLE_PRECISION:X87 CPU_CLK_UNHALTED:THREAD_P "
            "BR_INST_RETIRED:ALL_BRANCHES BR_MISP_RETIRED:ALL_BRANCHES" },
        {4999999, 500000, "PAPI_L2_DCA PAPI_L2_TCA PAPI_L2_ICA PAPI_L2_DCM "
            "PAPI_L2_ICM PAPI_L2_TCM PAPI_L3_TCM PAPI_L3_TCA PAPI_BR_INS "
            "L2_RQSTS:DEMAND_DATA_RD_HIT PAPI_BR_MSP MEM_UOP_RETIRED:ALL_LOADS "
            "L2_RQSTS:ALL_DEMAND_RD_HIT LAST_LEVEL_CACHE_REFERENCES "
            "LAST_LEVEL_CACHE_MISSES ICACHE:MISSES L2_RQSTS:CODE_RD_HIT "
            "MEM_UOP_RETIRED:ANY_LOADS L2_RQSTS:CODE_RD_MISS"},
        {499999, 100000, "DTLB_STORE_MISSES:CAUSES_A_WALK"
            "PAPI_TLB_DM DTLB_LOAD_MISSES:WALK_DURATION PAPI_TLB_IM "
            "ITLB_MISSES:WALK_DURATION DTLB_LOAD_MISSES:CAUSES_A_WALK " },
        {99999, 0, "" },
        {0, 0, "" }
    };

    if (PAPI_OK != PAPI_event_name_to_code((char *)name, &event_code)) {
        OUTPUT(("%s [%s]", _ERROR("event not available (name->code)"), name));
        return PERFEXPERT_FAILURE;
    }
    if (PAPI_OK != PAPI_query_event(event_code)) {
        OUTPUT(("%s [%s]", _ERROR("event not available (query code)"), name));
        return PERFEXPERT_FAILURE;
    }
    if (PAPI_OK != PAPI_get_event_info(event_code, &info)) {
        OUTPUT(("%s [%s]", _ERROR("event info not available"), name));
        return PERFEXPERT_FAILURE;
    }

    if ((0 == strlen(info.derived)) ||
        (0 == strcmp(info.derived, "NOT_DERIVED")) ||
        (0 == strcmp(info.derived, "DERIVED_CMPD"))) {

        while (0 != strcmp("", sample[cat].events)) {
            if (NULL != strstr(sample[cat].events, name)) {
                break;
            }
            cat++;
        }

        if (0 >= (rate = get_prime(sample[cat].last, sample[cat].end))) {
            OUTPUT(("%s [%s]", _ERROR("unable to get prime"), name));
            return PERFEXPERT_FAILURE;
        }
        sample[cat].last = rate;
        return rate;
    }
    return PERFEXPERT_FAILURE;
}

/* get_prime */
static int get_prime(int start, int end) {
    int flag, i, j;

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

#ifdef __cplusplus
}
#endif

// EOF
