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
#include <papi.h>
#include <matheval.h>

/* Tools headers */
#include "tools/perfexpert/perfexpert_types.h"

/* Modules headers */
#include "lcpi.h"
#include "lcpi_types.h"
#include "lcpi_metrics.h"

/* PerfExpert common headers */
#include "common/perfexpert_alloc.h"
#include "common/perfexpert_constants.h"
#include "common/perfexpert_hash.h"
#include "common/perfexpert_list.h"
#include "common/perfexpert_md5.h"
#include "common/perfexpert_output.h"

int metrics_generate(void) {
    lcpi_metric_t *m = NULL, *temp = NULL;

    OUTPUT_VERBOSE((4, "%s", _YELLOW("Generating LCPI metrics")));

    if (PAPI_VER_CURRENT != PAPI_library_init(PAPI_VER_CURRENT)) {
        OUTPUT(("%s", _ERROR("initializing PAPI")));
        return PERFEXPERT_ERROR;
    }

    /* Generate LCPI metrics (be sure the events are ordered) */
    if (PERFEXPERT_SUCCESS != generate_ratio_floating_point()) {
        return PERFEXPERT_ERROR;
    }
    if (PERFEXPERT_SUCCESS != generate_ratio_data_accesses()) {
        return PERFEXPERT_ERROR;
    }
    if (PERFEXPERT_SUCCESS != generate_gflops()) {
        return PERFEXPERT_ERROR;
    }
    if (PERFEXPERT_SUCCESS != generate_overall()) {
        return PERFEXPERT_ERROR;
    }
    if (PERFEXPERT_SUCCESS != generate_data_accesses()) {
        return PERFEXPERT_ERROR;
    }
    if (PERFEXPERT_SUCCESS != generate_instruction_accesses()) {
        return PERFEXPERT_ERROR;
    }
    if (PERFEXPERT_SUCCESS != generate_tlb_metrics()) {
        return PERFEXPERT_ERROR;
    }
    if (PERFEXPERT_SUCCESS != generate_branch_metrics()) {
        return PERFEXPERT_ERROR;
    }
    if (PERFEXPERT_SUCCESS != generate_floating_point_instr()) {
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
    lcpi_metric_t *m;
    char a[MAX_LCPI];

    bzero(a, MAX_LCPI);
    if (event_avail("PAPI_FML_INS") && event_avail("PAPI_FDV_INS")) {
        USE_EVENT("PAPI_FML_INS");
        USE_EVENT("PAPI_FDV_INS");

        if (event_avail("PAPI_FAD_INS")) {
            USE_EVENT("PAPI_FAD_INS");
            strcpy(a,
                "(PAPI_FML_INS + PAPI_FDV_INS + PAPI_FAD_INS) / PAPI_TOT_INS");
        } else {
            strcpy(a, "(PAPI_FML_INS + PAPI_FDV_INS) / PAPI_TOT_INS");
        }
    } else if (event_avail("FP_COMP_OPS_EXE:SSE_FP_PACKED_DOUBLE") &&
        event_avail("FP_COMP_OPS_EXE:SSE_FP_SCALAR_SINGLE") &&
        event_avail("FP_COMP_OPS_EXE:SSE_PACKED_SINGLE") &&
        event_avail("FP_COMP_OPS_EXE:SSE_SCALAR_DOUBLE")) {

        USE_EVENT("FP_COMP_OPS_EXE:SSE_FP_PACKED_DOUBLE");
        USE_EVENT("FP_COMP_OPS_EXE:SSE_FP_SCALAR_SINGLE");
        USE_EVENT("FP_COMP_OPS_EXE:SSE_PACKED_SINGLE");
        USE_EVENT("FP_COMP_OPS_EXE:SSE_SCALAR_DOUBLE");

        strcpy(a, "(FP_COMP_OPS_EXE_SSE_PACKED_SINGLE + "
            "FP_COMP_OPS_EXE_SSE_FP_PACKED_DOUBLE + "
            "FP_COMP_OPS_EXE_SSE_FP_SCALAR_SINGLE + "
            "FP_COMP_OPS_EXE_SSE_SCALAR_DOUBLE) / PAPI_TOT_INS");
    } else if (event_avail("PAPI_FP_INS")) {
        USE_EVENT("PAPI_FP_INS");
        strcpy(a, "PAPI_FP_INS / PAPI_TOT_INS");
    } else {
        OUTPUT(("%s", "unable to calculate 'ratio.floating_point'"));
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
    lcpi_metric_t *m;
    char a[MAX_LCPI];

    bzero(a, MAX_LCPI);
    if (event_avail("PAPI_L1_DCA")) {
        USE_EVENT("PAPI_L1_DCA");
        strcpy(a, "PAPI_L1_DCA / PAPI_TOT_INS");
    } else if (event_avail("PAPI_LD_INS")) {
        USE_EVENT("PAPI_LD_INS");
        strcpy(a, "PAPI_LD_INS / PAPI_TOT_INS");
    } else {
        OUTPUT(("%s", "unable to calculate 'ratio.data_accesses'"));
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
    lcpi_metric_t *m;
    char overall[MAX_LCPI], scalar[MAX_LCPI], packed[MAX_LCPI];

    bzero(overall, MAX_LCPI);
    bzero(scalar, MAX_LCPI);
    bzero(packed, MAX_LCPI);
    if (event_avail("RETIRED_SSE_OPERATIONS:ALL")) {
        USE_EVENT("RETIRED_SSE_OPERATIONS:ALL");

        strcpy(overall, "(RETIRED_SSE_OPERATIONS_ALL / PAPI_TOT_CYC) / 4");
        strcpy(packed, "");
        strcpy(scalar, "");
    } else if (event_avail("RETIRED_SSE_OPS:ALL")) {
        USE_EVENT("RETIRED_SSE_OPS:ALL");

        strcpy(overall, "(RETIRED_SSE_OPS_ALL / PAPI_TOT_CYC) / 4");
        strcpy(packed, "");
        strcpy(scalar, "");
    } else if (event_avail("SSEX_UOPS_RETIRED:PACKED_DOUBLE") &&
        event_avail("SSEX_UOPS_RETIRED:PACKED_SINGLE") &&
        event_avail("SSEX_UOPS_RETIRED:SCALAR_DOUBLE") &&
        event_avail("SSEX_UOPS_RETIRED:SCALAR_SINGLE")) {

        USE_EVENT("SSEX_UOPS_RETIRED:PACKED_DOUBLE");
        USE_EVENT("SSEX_UOPS_RETIRED:PACKED_SINGLE");
        USE_EVENT("SSEX_UOPS_RETIRED:SCALAR_DOUBLE");
        USE_EVENT("SSEX_UOPS_RETIRED:SCALAR_SINGLE");

        strcpy(overall, "((SSEX_UOPS_RETIRED_PACKED_DOUBLE * 2 + "
            "SSEX_UOPS_RETIRED_PACKED_SINGLE * 4 + "
            "SSEX_UOPS_RETIRED_SCALAR_SINGLE + SSEX_UOPS_RETIRED_SCALAR_DOUBLE)"
            " / PAPI_TOT_CYC) / 4");
        strcpy(packed, "((SSEX_UOPS_RETIRED_PACKED_DOUBLE * 2 + "
            "SSEX_UOPS_RETIRED_PACKED_SINGLE * 4) / PAPI_TOT_CYC) / 4");
        strcpy(scalar, "(SSEX_UOPS_RETIRED_SCALAR_DOUBLE + "
            "SSEX_UOPS_RETIRED_SCALAR_SINGLE / PAPI_TOT_CYC) / 4");
    } else if (event_avail("SIMD_COMP_INST_RETIRED:PACKED_DOUBLE") &&
        event_avail("SIMD_COMP_INST_RETIRED:PACKED_SINGLE") &&
        event_avail("SIMD_COMP_INST_RETIRED:SCALAR_DOUBLE") &&
        event_avail("SIMD_COMP_INST_RETIRED:SCALAR_SINGLE")) {

        USE_EVENT("SIMD_COMP_INST_RETIRED:PACKED_DOUBLE");
        USE_EVENT("SIMD_COMP_INST_RETIRED:PACKED_SINGLE");
        USE_EVENT("SIMD_COMP_INST_RETIRED:SCALAR_DOUBLE");
        USE_EVENT("SIMD_COMP_INST_RETIRED:SCALAR_SINGLE");

        strcpy(overall, "((SIMD_COMP_INST_RETIRED_PACKED_DOUBLE * 2 + "
            "SIMD_COMP_INST_RETIRED_PACKED_SINGLE * 4 + "
            "SIMD_COMP_INST_RETIRED_SCALAR_SINGLE + "
            "SIMD_COMP_INST_RETIRED_SCALAR_DOUBLE) / PAPI_TOT_CYC) / 4");
        strcpy(packed, "((SIMD_COMP_INST_RETIRED_PACKED_DOUBLE * 2 + "
            "SIMD_COMP_INST_RETIRED_PACKED_SINGLE * 4) / PAPI_TOT_CYC) / 4");
        strcpy(scalar, "((SIMD_COMP_INST_RETIRED_SCALAR_SINGLE + "
            "SIMD_COMP_INST_RETIRED_SCALAR_DOUBLE)/ PAPI_TOT_CYC) / 4");
    } else if (event_avail("SIMD_FP_256:PACKED_SINGLE") &&
        event_avail("SIMD_FP_256:PACKED_DOUBLE") &&
        event_avail("FP_COMP_OPS_EXE:SSE_FP_PACKED_DOUBLE") &&
        event_avail("FP_COMP_OPS_EXE:SSE_FP_SCALAR_SINGLE") &&
        event_avail("FP_COMP_OPS_EXE:SSE_PACKED_SINGLE") &&
        event_avail("FP_COMP_OPS_EXE:SSE_SCALAR_DOUBLE")) {

        USE_EVENT("SIMD_FP_256:PACKED_SINGLE");
        USE_EVENT("SIMD_FP_256:PACKED_DOUBLE");
        USE_EVENT("FP_COMP_OPS_EXE:SSE_FP_PACKED_DOUBLE");
        USE_EVENT("FP_COMP_OPS_EXE:SSE_FP_SCALAR_SINGLE");
        USE_EVENT("FP_COMP_OPS_EXE:SSE_PACKED_SINGLE");
        USE_EVENT("FP_COMP_OPS_EXE:SSE_SCALAR_DOUBLE");

        strcpy(overall, "((SIMD_FP_256_PACKED_SINGLE * 8 + "
            "(SIMD_FP_256_PACKED_DOUBLE + FP_COMP_OPS_EXE_SSE_PACKED_SINGLE) "
            "* 4 + FP_COMP_OPS_EXE_SSE_FP_PACKED_DOUBLE * 2 + "
            "FP_COMP_OPS_EXE_SSE_FP_SCALAR_SINGLE + "
            "FP_COMP_OPS_EXE_SSE_SCALAR_DOUBLE) / PAPI_TOT_CYC) / 8");
        strcpy(packed, "((SIMD_FP_256_PACKED_SINGLE * 8 + ("
            "SIMD_FP_256_PACKED_DOUBLE + FP_COMP_OPS_EXE_SSE_PACKED_SINGLE) * 4"
            " + FP_COMP_OPS_EXE_SSE_FP_PACKED_DOUBLE * 2) / PAPI_TOT_CYC) / 8");
        strcpy(scalar, "((FP_COMP_OPS_EXE_SSE_FP_SCALAR_SINGLE + "
            "FP_COMP_OPS_EXE_SSE_SCALAR_DOUBLE) / PAPI_TOT_CYC) / 8");
    } else {
        OUTPUT(("%s", "unable to calculate 'GFLOPS_(%%_max)'"));
        return PERFEXPERT_ERROR;
    }

    /* GFLOPS: overall */
    PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, m->name, (strlen("GFLOPS_(%_max).overall") + 1));
    strcpy(m->name, "GFLOPS_(%_max).overall");
    strcpy(m->name_md5, perfexpert_md5_string(m->name));
    m->value = 0.0;
    m->expression = evaluator_create(overall);
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
    m->expression = evaluator_create(scalar);
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
    m->expression = evaluator_create(packed);
    if (NULL == m->expression) {
        OUTPUT(("%s (%s)", _ERROR("invalid expression"), m->name));
        return PERFEXPERT_ERROR;
    }
    perfexpert_hash_add_str(my_module_globals.metrics_by_name, name_md5, m);

    return PERFEXPERT_SUCCESS;
}

/* generate_overall */
static int generate_overall(void) {
    lcpi_metric_t *m;

    /* Check the basic ones first */
    if ((!event_avail("PAPI_TOT_CYC")) || (!event_avail("PAPI_TOT_INS"))) {
        OUTPUT(("%s", "unable to calculate 'overall'"));
        return PERFEXPERT_ERROR;
    }
    USE_EVENT("PAPI_TOT_CYC");
    USE_EVENT("PAPI_TOT_INS");

    PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, m->name, (strlen("overall") + 1));
    strcpy(m->name, "overall");
    strcpy(m->name_md5, perfexpert_md5_string(m->name));
    m->value = 0.0;
    m->expression = evaluator_create("PAPI_TOT_CYC / PAPI_TOT_INS");
    if (NULL == m->expression) {
        OUTPUT(("%s (%s)", _ERROR("invalid expression"), m->name));
        return PERFEXPERT_ERROR;
    }
    perfexpert_hash_add_str(my_module_globals.metrics_by_name, name_md5, m);

    return PERFEXPERT_SUCCESS;
}

/* generate_data_accesses */
static int generate_data_accesses(void) {
    lcpi_metric_t *m;
    char overall[MAX_LCPI], L1d_hits[MAX_LCPI], L2d_hits[MAX_LCPI],
        L3d_hits[MAX_LCPI], LLC_misses[MAX_LCPI], a[MAX_LCPI], b[MAX_LCPI],
        c[MAX_LCPI];

    bzero(a, MAX_LCPI);
    if (event_avail("PAPI_L1_DCA")) {
        USE_EVENT("PAPI_L1_DCA");
        strcpy(a, "PAPI_L1_DCA");
    } else if (event_avail("PAPI_LD_INS")) {
        USE_EVENT("PAPI_LD_INS");
        strcpy(a, "PAPI_LD_INS");
    }
    bzero(L1d_hits, MAX_LCPI);
    sprintf(L1d_hits, "(%s * L1_dlat) / PAPI_TOT_INS", a);

    bzero(b, MAX_LCPI);
    if (event_avail("L2_RQSTS:DEMAND_DATA_RD_HIT")) {
        USE_EVENT("L2_RQSTS:DEMAND_DATA_RD_HIT");
        strcpy(b, "L2_RQSTS_DEMAND_DATA_RD_HIT");
    } else if (event_avail("PAPI_L2_DCA")) {
        USE_EVENT("PAPI_L2_DCA");
        strcpy(b, "PAPI_L2_DCA");
    } else if (event_avail("PAPI_L2_TCA") && event_avail("PAPI_L2_ICA")) {
        USE_EVENT("PAPI_L2_TCA");
        USE_EVENT("PAPI_L2_ICA");
        strcpy(b, "(PAPI_L2_TCA - PAPI_L2_ICA)");
    }
    bzero(L2d_hits, MAX_LCPI);
    sprintf(L2d_hits, "(%s * L2_lat) / PAPI_TOT_INS", b);

    bzero(c, MAX_LCPI);
    bzero(L3d_hits, MAX_LCPI);
    if (event_avail("PAPI_L3_TCA") && event_avail("PAPI_L3_TCM")) {
        USE_EVENT("PAPI_L3_TCA");
        USE_EVENT("PAPI_L3_TCM");
        strcpy(c, "(PAPI_L3_TCA - PAPI_L3_TCM)");
        sprintf(L3d_hits, "(%s * L3_lat) / PAPI_TOT_INS", c);
    }

    bzero(overall, MAX_LCPI);
    bzero(LLC_misses, MAX_LCPI);
    if (event_avail("PAPI_L3_TCA") && event_avail("PAPI_L3_TCM")) {
        USE_EVENT("PAPI_L3_TCA");
        USE_EVENT("PAPI_L3_TCM");
        strcpy(LLC_misses, "(PAPI_L3_TCM * mem_lat) / PAPI_TOT_INS");
        sprintf(overall, "(%s * L1_dlat + %s * L2_lat + %s * L3_lat + "
            "PAPI_L3_TCM * mem_lat) / PAPI_TOT_INS", a, b, c);
    } else if (event_avail("PAPI_L2_DCM")) {
        USE_EVENT("PAPI_L2_DCM");
        sprintf(LLC_misses, "(%s * mem_lat) / PAPI_TOT_INS", b);
        sprintf(overall, "(%s * L1_dlat + %s * L2_lat + PAPI_L2_DCM * mem_lat) "
            "/ PAPI_TOT_INS", a, b);
    } else if (event_avail("PAPI_L2_TCM") && event_avail("PAPI_L2_ICM")) {
        USE_EVENT("PAPI_L2_TCM");
        USE_EVENT("PAPI_L2_ICM");
        sprintf(LLC_misses, "(%s * mem_lat) / PAPI_TOT_INS", b);
        sprintf(overall, "(%s * L1_dlat + %s * L2_lat + "
            "(PAPI_L2_TCM - PAPI_L2_ICM) * mem_lat) / PAPI_TOT_INS", a, b);
    }

    /* L1 and L2 are mandatory, L3 is optional */
    if ((0 == strlen(a)) || (0 == strlen(b))) {
        OUTPUT(("%s", "unable to calculate 'data_accesses'"));
        return PERFEXPERT_ERROR;
    }

    /* Data accesses: overall */
    PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, m->name,
        (strlen("data_accesses.overall") + 1));
    strcpy(m->name, "data_accesses.overall");
    strcpy(m->name_md5, perfexpert_md5_string(m->name));
    m->value = 0.0;
    m->expression = evaluator_create(overall);
    if (NULL == m->expression) {
        OUTPUT(("%s (%s)", _ERROR("invalid expression"), m->name));
        return PERFEXPERT_ERROR;
    }
    perfexpert_hash_add_str(my_module_globals.metrics_by_name, name_md5, m);

    /* Data accesses: L1d_hits */
    PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, m->name,
        (strlen("data_accesses.L1d_hits") + 1));
    strcpy(m->name, "data_accesses.L1d_hits");
    strcpy(m->name_md5, perfexpert_md5_string(m->name));
    m->value = 0.0;
    m->expression = evaluator_create(L1d_hits);
    if (NULL == m->expression) {
        OUTPUT(("%s (%s)", _ERROR("invalid expression"), m->name));
        return PERFEXPERT_ERROR;
    }
    perfexpert_hash_add_str(my_module_globals.metrics_by_name, name_md5, m);

    /* Data accesses: L2d_hits */
    PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, m->name,
        (strlen("data_accesses.L2d_hits") + 1));
    strcpy(m->name, "data_accesses.L2d_hits");
    strcpy(m->name_md5, perfexpert_md5_string(m->name));
    m->value = 0.0;
    m->expression = evaluator_create(L2d_hits);
    if (NULL == m->expression) {
        OUTPUT(("%s (%s)", _ERROR("invalid expression"), m->name));
        return PERFEXPERT_ERROR;
    }
    perfexpert_hash_add_str(my_module_globals.metrics_by_name, name_md5, m);

    /* Data accesses: L3d_hits */
    if (0 < strlen(c)) {
        PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
        PERFEXPERT_ALLOC(char, m->name,
            (strlen("data_accesses.L3d_hits") + 1));
        strcpy(m->name, "data_accesses.L3d_hits");
        strcpy(m->name_md5, perfexpert_md5_string(m->name));
        m->value = 0.0;
        m->expression = evaluator_create(L3d_hits);
        if (NULL == m->expression) {
            OUTPUT(("%s (%s)", _ERROR("invalid expression"), m->name));
            return PERFEXPERT_ERROR;
        }
        perfexpert_hash_add_str(my_module_globals.metrics_by_name, name_md5, m);
    }

    /* Data accesses: LLC_misses */
    PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, m->name,
        (strlen("data_accesses.LLC_misses") + 1));
    strcpy(m->name, "data_accesses.LLC_misses");
    strcpy(m->name_md5, perfexpert_md5_string(m->name));
    m->value = 0.0;
    m->expression = evaluator_create(LLC_misses);
    if (NULL == m->expression) {
        OUTPUT(("%s (%s)", _ERROR("invalid expression"), m->name));
        return PERFEXPERT_ERROR;
    }
    perfexpert_hash_add_str(my_module_globals.metrics_by_name, name_md5, m);

    return PERFEXPERT_SUCCESS;
}

/* generate_instruction_accesses */
static int generate_instruction_accesses(void) {
    lcpi_metric_t *m;
    char overall[MAX_LCPI], L1i_hits[MAX_LCPI], L2i_hits[MAX_LCPI],
        L2i_misses[MAX_LCPI], a[MAX_LCPI], b[MAX_LCPI], c[MAX_LCPI];

    bzero(a, MAX_LCPI);
    if (event_avail("PAPI_L1_ICA")) {
        USE_EVENT("PAPI_L1_ICA");
        strcpy(a, "PAPI_L1_ICA");
    } else if (event_avail("ICACHE")) {
        USE_EVENT("ICACHE");
        strcpy(a, "ICACHE");
    }

    bzero(b, MAX_LCPI);
    if (event_avail("PAPI_L2_ICA")) {
        USE_EVENT("PAPI_L2_ICA");
        strcpy(b, "PAPI_L2_ICA");
    } else if (event_avail("PAPI_L2_TCA") && event_avail("PAPI_L2_DCA")) {
        USE_EVENT("PAPI_L2_TCA");
        USE_EVENT("PAPI_L2_DCA");
        strcpy(b, "(PAPI_L2_TCA - PAPI_L2_DCA)");
    }

    bzero(c, MAX_LCPI);
    if (event_avail("PAPI_L2_ICM")) {
        USE_EVENT("PAPI_L2_ICM");
        strcpy(c, "PAPI_L2_ICM");
    } else if (event_avail("PAPI_L2_TCM") && event_avail("PAPI_L2_DCM")) {
        USE_EVENT("PAPI_L2_TCM");
        USE_EVENT("PAPI_L2_DCM");
        strcpy(c, "(PAPI_L2_TCM - PAPI_L2_DCM)");
    }

    if ((0 == strlen(a)) || (0 == strlen(b)) || (0 == strlen(c))) {
        OUTPUT(("%s", "unable to calculate 'instruction_accesses'"));
        return PERFEXPERT_ERROR;
    }

    bzero(overall, MAX_LCPI);
    bzero(L1i_hits, MAX_LCPI);
    bzero(L2i_hits, MAX_LCPI);
    bzero(L2i_misses, MAX_LCPI);
    sprintf(overall,
        "(%s * L1_ilat + %s * L2_lat + %s * mem_lat) / PAPI_TOT_INS", a, b, c);
    sprintf(L1i_hits, "(%s * L1_ilat) / PAPI_TOT_INS", a);
    sprintf(L2i_hits, "(%s * L2_lat) / PAPI_TOT_INS", b);
    sprintf(L2i_misses, "(%s * mem_lat) / PAPI_TOT_INS", c);

    /* Instruction accesses: overall */
    PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, m->name,
        (strlen("instruction_accesses.overall") + 1));
    strcpy(m->name, "instruction_accesses.overall");
    strcpy(m->name_md5, perfexpert_md5_string(m->name));
    m->value = 0.0;
    m->expression = evaluator_create(overall);
    if (NULL == m->expression) {
        OUTPUT(("%s (%s)", _ERROR("invalid expression"), m->name));
        return PERFEXPERT_ERROR;
    }
    perfexpert_hash_add_str(my_module_globals.metrics_by_name, name_md5, m);

    /* Instruction accesses: L1 hits */
    PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, m->name,
        (strlen("instruction_accesses.L1i_hits") + 1));
    strcpy(m->name, "instruction_accesses.L1i_hits");
    strcpy(m->name_md5, perfexpert_md5_string(m->name));
    m->value = 0.0;
    m->expression = evaluator_create(L1i_hits);
    if (NULL == m->expression) {
        OUTPUT(("%s (%s)", _ERROR("invalid expression"), m->name));
        return PERFEXPERT_ERROR;
    }
    perfexpert_hash_add_str(my_module_globals.metrics_by_name, name_md5, m);

    /* Instruction accesses: L2 hits */
    PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, m->name,
        (strlen("instruction_accesses.L2i_hits") + 1));
    strcpy(m->name, "instruction_accesses.L2i_hits");
    strcpy(m->name_md5, perfexpert_md5_string(m->name));
    m->value = 0.0;
    m->expression = evaluator_create(L2i_hits);
    if (NULL == m->expression) {
        OUTPUT(("%s (%s)", _ERROR("invalid expression"), m->name));
        return PERFEXPERT_ERROR;
    }
    perfexpert_hash_add_str(my_module_globals.metrics_by_name, name_md5, m);

    /* Instruction accesses: L2 misses */
    PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, m->name,
        (strlen("instruction_accesses.L2i_misses") + 1));
    strcpy(m->name, "instruction_accesses.L2i_misses");
    strcpy(m->name_md5, perfexpert_md5_string(m->name));
    m->value = 0.0;
    m->expression = evaluator_create(L2i_misses);
    if (NULL == m->expression) {
        OUTPUT(("%s (%s)", _ERROR("invalid expression"), m->name));
        return PERFEXPERT_ERROR;
    }
    perfexpert_hash_add_str(my_module_globals.metrics_by_name, name_md5, m);

    return PERFEXPERT_SUCCESS;
}

/* generate_tlb_metrics */
static int generate_tlb_metrics(void) {
    lcpi_metric_t *m;
    char a[MAX_LCPI];

    /* Data TLB */
    if (event_avail("DTLB_LOAD_MISSES:WALK_DURATION")) {
        USE_EVENT("DTLB_LOAD_MISSES:WALK_DURATION");
        strcpy(a, "DTLB_LOAD_MISSES_WALK_DURATION / PAPI_TOT_INS");
    } else if (event_avail("PAPI_TLB_DM")) {
        USE_EVENT("PAPI_TLB_DM");
        strcpy(a, "PAPI_TLB_DM * TLB_lat / PAPI_TOT_INS");
    } else if (event_avail("DTLB_LOAD_MISSES:CAUSES_A_WALK") &&
               event_avail("DTLB_STORE_MISSES:CAUSES_A_WALK")) {
        USE_EVENT("DTLB_LOAD_MISSES:CAUSES_A_WALK");
        USE_EVENT("DTLB_STORE_MISSES:CAUSES_A_WALK");
        strcpy(a, "(DTLB_LOAD_MISSES_CAUSES_A_WALK + "
            "DTLB_STORE_MISSES_CAUSES_A_WALK) * TLB_lat / PAPI_TOT_INS");
    } else {
        OUTPUT(("%s", "unable to calculate 'data_TLB.overall'"));
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

    /* Instruction TLB */
    if (event_avail("ITLB_MISSES:WALK_DURATION")) {
        USE_EVENT("ITLB_MISSES:WALK_DURATION");
        strcpy(a, "ITLB_MISSES_WALK_DURATION / PAPI_TOT_INS");
    } else if (event_avail("PAPI_TLB_IM")) {
        USE_EVENT("PAPI_TLB_IM");
        str_cpy(a, "PAPI_TLB_IM * TLB_lat / PAPI_TOT_INS");
    } else {
        OUTPUT(("%s", "unable to calculate 'instruction_TLB.overall'"));
        return PERFEXPERT_ERROR;
    }

    PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, m->name, (strlen("instruction_TLB.overall") + 1));
    strcpy(m->name, "instruction_TLB.overall");
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

/* generate_branch_metrics */
static int generate_branch_metrics(void) {
    lcpi_metric_t *m;

    if (event_avail("PAPI_BR_INS") && event_avail("PAPI_BR_MSP")) {
        USE_EVENT("PAPI_BR_INS");
        USE_EVENT("PAPI_BR_MSP");
    } else {
        OUTPUT(("%s", "unable to calculate 'branch_instructions'"));
        return PERFEXPERT_ERROR;
    }

    /* Overall */
    PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, m->name,
        (strlen("branch_instructions.overall") + 1));
    strcpy(m->name, "branch_instructions.overall");
    strcpy(m->name_md5, perfexpert_md5_string(m->name));
    m->value = 0.0;
    m->expression = evaluator_create("(PAPI_BR_INS * BR_lat + PAPI_BR_MSP "
        "* BR_miss_lat) / PAPI_TOT_INS");
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
    m->expression = evaluator_create("PAPI_BR_INS * BR_lat / PAPI_TOT_INS");
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
    m->expression = evaluator_create(
        "PAPI_BR_MSP * BR_miss_lat / PAPI_TOT_INS");
    if (NULL == m->expression) {
        OUTPUT(("%s (%s)", _ERROR("invalid expression"), m->name));
        return PERFEXPERT_ERROR;
    }
    perfexpert_hash_add_str(my_module_globals.metrics_by_name, name_md5, m);

    return PERFEXPERT_SUCCESS;
}

/* generate_floating_point_instr */
static int generate_floating_point_instr(void) {
    lcpi_metric_t *m;
    char overall[MAX_LCPI], fast[MAX_LCPI], slow[MAX_LCPI], a[MAX_LCPI],
        b[MAX_LCPI];

    bzero(a, MAX_LCPI);
    bzero(b, MAX_LCPI);
    if (event_avail("PAPI_FML_INS")) {
        USE_EVENT("PAPI_FML_INS");
        if (event_avail("PAPI_FAD_INS")) {
            USE_EVENT("PAPI_FAD_INS");
            strcpy(a, "((PAPI_FML_INS + PAPI_FAD_INS) * FP_lat)");
        } else {
            strcpy(a, "(PAPI_FML_INS * FP_lat)");
        }
    } else if (event_avail("FP_COMP_OPS_EXE:SSE_DOUBLE_PRECISION") &&
        event_avail("FP_COMP_OPS_EXE:SSE_FP") &&
        event_avail("FP_COMP_OPS_EXE:SSE_FP_PACKED") &&
        event_avail("FP_COMP_OPS_EXE:SSE_FP_SCALAR") &&
        event_avail("FP_COMP_OPS_EXE:SSE_SINGLE_PRECISION") &&
        event_avail("FP_COMP_OPS_EXE:X87")) {

        USE_EVENT("FP_COMP_OPS_EXE:SSE_DOUBLE_PRECISION");
        USE_EVENT("FP_COMP_OPS_EXE:SSE_FP");
        USE_EVENT("FP_COMP_OPS_EXE:SSE_FP_PACKED");
        USE_EVENT("FP_COMP_OPS_EXE:SSE_FP_SCALAR");
        USE_EVENT("FP_COMP_OPS_EXE:SSE_SINGLE_PRECISION");
        USE_EVENT("FP_COMP_OPS_EXE:X87");

        strcpy(a,
            "((FP_COMP_OPS_EXE_SSE_DOUBLE_PRECISION + FP_COMP_OPS_EXE_SSE_FP + "
            "FP_COMP_OPS_EXE_SSE_FP_PACKED + FP_COMP_OPS_EXE_SSE_FP_SCALAR + "
            "FP_COMP_OPS_EXE_SSE_SINGLE_PRECISION + FP_COMP_OPS_EXE_X87) * "
            "FP_lat)");
    } else if (event_avail("FP_COMP_OPS_EXE:SSE_FP_PACKED_DOUBLE") &&
        event_avail("FP_COMP_OPS_EXE:SSE_FP_SCALAR_SINGLE") &&
        event_avail("FP_COMP_OPS_EXE:SSE_PACKED_SINGLE") &&
        event_avail("FP_COMP_OPS_EXE:SSE_SCALAR_DOUBLE")) {

        USE_EVENT("FP_COMP_OPS_EXE:SSE_FP_PACKED_DOUBLE");
        USE_EVENT("FP_COMP_OPS_EXE:SSE_FP_SCALAR_SINGLE");
        USE_EVENT("FP_COMP_OPS_EXE:SSE_PACKED_SINGLE");
        USE_EVENT("FP_COMP_OPS_EXE:SSE_SCALAR_DOUBLE");

        strcpy(a, "((FP_COMP_OPS_EXE_SSE_FP_PACKED_DOUBLE + "
            "FP_COMP_OPS_EXE_SSE_FP_SCALAR_SINGLE + "
            "FP_COMP_OPS_EXE_SSE_PACKED_SINGLE + "
            "FP_COMP_OPS_EXE_SSE_SCALAR_DOUBLE) * FP_lat)");
    } else if (event_avail("ARITH")) {
        USE_EVENT("ARITH");
        strcpy(a, "ARITH * FP_lat");
    } else {
        OUTPUT(("%s", "unable to calculate 'fast_FP_instr'"));
        return PERFEXPERT_ERROR;
    }

    if (event_avail("PAPI_FDV_INS")) {
        USE_EVENT("PAPI_FDV_INS");
        strcpy(b, "(PAPI_FDV_INS * FP_slow_lat)");
    } else if (event_avail("ARITH:CYCLES_DIV_BUSY")) {
        USE_EVENT("ARITH:CYCLES_DIV_BUSY");
        strcpy(b, "ARITH_CYCLES_DIV_BUSY");
    } else {
        OUTPUT(("%s", "unable to calculate 'slow_FP_instr'"));
        return PERFEXPERT_ERROR;
    }

    bzero(overall, MAX_LCPI);
    bzero(fast, MAX_LCPI);
    bzero(slow, MAX_LCPI);
    sprintf(overall, "(%s + %s) / PAPI_TOT_INS", a, b);
    sprintf(fast, "%s / PAPI_TOT_INS", a);
    sprintf(slow, "%s / PAPI_TOT_INS", b);

    /* Floating-point instructions: overall */
    PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, m->name,
        (strlen("point_instr.overall") + 1));
    strcpy(m->name, "point_instr.overall");
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
        (strlen("point_instr.fast_FP_instr") + 1));
    strcpy(m->name, "point_instr.fast_FP_instr");
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
        (strlen("point_instr.slow_FP_instr") + 1));
    strcpy(m->name, "point_instr.slow_FP_instr");
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

/* event_avail */
static int event_avail(const char *event) {
    int event_code;

    if (PAPI_OK != PAPI_event_name_to_code((char *)event, &event_code)) {
        return PERFEXPERT_FALSE;
    }

    if (PAPI_OK != PAPI_query_event(event_code)) {
        return PERFEXPERT_FALSE;
    }

    return PERFEXPERT_TRUE;
}

// EOF
