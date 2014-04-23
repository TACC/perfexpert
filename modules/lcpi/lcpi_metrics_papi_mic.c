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
#include "lcpi_metrics_papi_mic.h"

/* PerfExpert common headers */
#include "common/perfexpert_alloc.h"
#include "common/perfexpert_constants.h"
#include "common/perfexpert_hash.h"
#include "common/perfexpert_list.h"
#include "common/perfexpert_md5.h"
#include "common/perfexpert_output.h"

int metrics_generate_mic(void) {
    lcpi_metric_t *m = NULL, *temp = NULL;

    OUTPUT_VERBOSE((4, "%s", _YELLOW("Generating LCPI metrics")));

    if (PAPI_NOT_INITED == PAPI_is_initialized()) {
        if (PAPI_VER_CURRENT != PAPI_library_init(PAPI_VER_CURRENT)) {
            OUTPUT(("%s", _ERROR("initializing PAPI")));
            return PERFEXPERT_ERROR;
        }
    }

    /* Generate LCPI metrics (be sure the events are ordered) */
    if (PERFEXPERT_SUCCESS != generate_mic_ratio_floating_point()) {
        OUTPUT(("%s", _ERROR("generating MIC ratio floating point")));
        return PERFEXPERT_ERROR;
    }
    if (PERFEXPERT_SUCCESS != generate_mic_ratio_data_accesses()) {
        OUTPUT(("%s", _ERROR("generating MIC ratio data access")));
        return PERFEXPERT_ERROR;
    }
    if (PERFEXPERT_SUCCESS != generate_mic_overall()) {
        OUTPUT(("%s", _ERROR("generating MIC overall")));
        return PERFEXPERT_ERROR;
    }
    if (PERFEXPERT_SUCCESS != generate_mic_data_accesses()) {
        OUTPUT(("%s", _ERROR("generating MIC data access")));
        return PERFEXPERT_ERROR;
    }
    if (PERFEXPERT_SUCCESS != generate_mic_instruction_accesses()) {
        OUTPUT(("%s", _ERROR("generating MIC instruction access")));
        return PERFEXPERT_ERROR;
    }
    if (PERFEXPERT_SUCCESS != generate_mic_tlb_metrics()) {
        OUTPUT(("%s", _ERROR("generating MIC TLB metrics")));
        return PERFEXPERT_ERROR;
    }
    if (PERFEXPERT_SUCCESS != generate_mic_branch_metrics()) {
        OUTPUT(("%s", _ERROR("generating MIC branch metrics")));
        return PERFEXPERT_ERROR;
    }
    if (PERFEXPERT_SUCCESS != generate_mic_floating_point_instr()) {
        OUTPUT(("%s", _ERROR("generating MIC floating point instructions")));
        return PERFEXPERT_ERROR;
    }

    USE_EVENT(PAPI_TOT_INS);
    USE_EVENT(PAPI_LD_INS);
    USE_EVENT(PAPI_SR_INS);
    USE_EVENT(PAPI_BR_INS);
    USE_EVENT(PAPI_BR_MSP);
    USE_EVENT(PAPI_VEC_INS);
    USE_EVENT(PAPI_TOT_CYC);
    USE_EVENT(PAPI_L1_DCA);
    USE_EVENT(PAPI_L1_ICA);
    USE_EVENT(PAPI_L1_DCM);
    USE_EVENT(PAPI_L1_ICM);
    USE_EVENT(PAPI_L2_LDM);
    USE_EVENT(PAPI_TLB_DM);
    USE_EVENT(PAPI_TLB_IM);

    perfexpert_hash_iter_str(my_module_globals.metrics_by_name, m, temp) {
        OUTPUT_VERBOSE((7, "   %s=%s", _CYAN(m->name),
            evaluator_get_string(m->expression)));
    }

    OUTPUT_VERBOSE((4, "(%d) %s",
        perfexpert_hash_count_str(my_module_globals.metrics_by_name),
        _MAGENTA("LCPI metrics")));

    return PERFEXPERT_SUCCESS;
}

/* generate_mic_ratio_floating_point */
static int generate_mic_ratio_floating_point(void) {
    lcpi_metric_t *m;

    PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, m->name, (strlen("ratio.floating_point") + 1));
    strcpy(m->name, "ratio.floating_point");
    strcpy(m->name_md5, perfexpert_md5_string(m->name));
    m->value = 0.0;
    m->expression = evaluator_create("PAPI_VEC_INS / PAPI_TOT_INS");
    if (NULL == m->expression) {
        OUTPUT(("%s (%s)", _ERROR("invalid expression"), m->name));
        return PERFEXPERT_ERROR;
    }
    perfexpert_hash_add_str(my_module_globals.metrics_by_name, name_md5, m);

    return PERFEXPERT_SUCCESS;
}

/* generate_mic_ratio_data_accesses */
static int generate_mic_ratio_data_accesses(void) {
    lcpi_metric_t *m;

    PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, m->name, (strlen("ratio.data_accesses") + 1));
    strcpy(m->name, "ratio.data_accesses");
    strcpy(m->name_md5, perfexpert_md5_string(m->name));
    m->value = 0.0;
    m->expression = evaluator_create("PAPI_LD_INS / PAPI_TOT_INS");
    if (NULL == m->expression) {
        OUTPUT(("%s (%s)", _ERROR("invalid expression"), m->name));
        return PERFEXPERT_ERROR;
    }
    perfexpert_hash_add_str(my_module_globals.metrics_by_name, name_md5, m);

    return PERFEXPERT_SUCCESS;
}

/* generate_mic_overall */
static int generate_mic_overall(void) {
    lcpi_metric_t *m;

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

/* generate_mic_data_accesses */
static int generate_mic_data_accesses(void) {
    lcpi_metric_t *m;

    /* Data accesses: overall */
    PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, m->name,
        (strlen("data_accesses.overall") + 1));
    strcpy(m->name, "data_accesses.overall");
    strcpy(m->name_md5, perfexpert_md5_string(m->name));
    m->value = 0.0;
    m->expression = evaluator_create("((PAPI_LD_INS * L1_dlat) + ((PAPI_L1_DCM "
        "+ PAPI_L2_LDM) * mem_lat)) / PAPI_TOT_INS");
    if (NULL == m->expression) {
        OUTPUT(("%s (%s)", _ERROR("invalid expression"), m->name));
        return PERFEXPERT_ERROR;
    }
    perfexpert_hash_add_str(my_module_globals.metrics_by_name, name_md5, m);

    /* Data accesses: L1d_hits */
    PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, m->name,
        (strlen("data_accesses.L1_cache_hits") + 1));
    strcpy(m->name, "data_accesses.L1_cache_hits");
    strcpy(m->name_md5, perfexpert_md5_string(m->name));
    m->value = 0.0;
    m->expression = evaluator_create("(PAPI_LD_INS * L1_dlat) / PAPI_TOT_INS");
    if (NULL == m->expression) {
        OUTPUT(("%s (%s)", _ERROR("invalid expression"), m->name));
        return PERFEXPERT_ERROR;
    }
    perfexpert_hash_add_str(my_module_globals.metrics_by_name, name_md5, m);

    /* Data accesses: LLC_misses */
    PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, m->name,
        (strlen("data_accesses.LLC_misses_(memory)") + 1));
    strcpy(m->name, "data_accesses.LLC_misses_(memory)");
    strcpy(m->name_md5, perfexpert_md5_string(m->name));
    m->value = 0.0;
    m->expression = evaluator_create("((PAPI_L1_DCM + PAPI_L2_LDM) * mem_lat) /"
        " PAPI_TOT_INS");
    if (NULL == m->expression) {
        OUTPUT(("%s (%s)", _ERROR("invalid expression"), m->name));
        return PERFEXPERT_ERROR;
    }
    perfexpert_hash_add_str(my_module_globals.metrics_by_name, name_md5, m);

    return PERFEXPERT_SUCCESS;
}

/* generate_mic_instruction_accesses */
static int generate_mic_instruction_accesses(void) {
    lcpi_metric_t *m;

    /* Instruction accesses: overall */
    PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, m->name,
        (strlen("instruction_accesses.overall") + 1));
    strcpy(m->name, "instruction_accesses.overall");
    strcpy(m->name_md5, perfexpert_md5_string(m->name));
    m->value = 0.0;
    m->expression = evaluator_create("((PAPI_L1_ICA * L1_ilat) + (PAPI_L1_ICM "
        "* mem_lat)) / PAPI_TOT_INS");
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
    m->expression = evaluator_create("(PAPI_L1_ICA * L1_ilat) / PAPI_TOT_INS");
    if (NULL == m->expression) {
        OUTPUT(("%s (%s)", _ERROR("invalid expression"), m->name));
        return PERFEXPERT_ERROR;
    }
    perfexpert_hash_add_str(my_module_globals.metrics_by_name, name_md5, m);

    /* Instruction accesses: L1 misses */
    PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, m->name,
        (strlen("instruction_accesses.L2_misses") + 1));
    strcpy(m->name, "instruction_accesses.L2_misses");
    strcpy(m->name_md5, perfexpert_md5_string(m->name));
    m->value = 0.0;
    m->expression = evaluator_create("(PAPI_L1_ICM * mem_lat) / PAPI_TOT_INS");
    if (NULL == m->expression) {
        OUTPUT(("%s (%s)", _ERROR("invalid expression"), m->name));
        return PERFEXPERT_ERROR;
    }
    perfexpert_hash_add_str(my_module_globals.metrics_by_name, name_md5, m);

    return PERFEXPERT_SUCCESS;
}

/* generate_mic_tlb_metrics */
static int generate_mic_tlb_metrics(void) {
    lcpi_metric_t *m;

    /* Data TLB */
    PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, m->name, (strlen("data_TLB.overall") + 1));
    strcpy(m->name, "data_TLB.overall");
    strcpy(m->name_md5, perfexpert_md5_string(m->name));
    m->value = 0.0;
    m->expression = evaluator_create("PAPI_TLB_DM * TLB_lat / PAPI_TOT_INS");
    if (NULL == m->expression) {
        OUTPUT(("%s (%s)", _ERROR("invalid expression"), m->name));
        return PERFEXPERT_ERROR;
    }
    perfexpert_hash_add_str(my_module_globals.metrics_by_name, name_md5, m);

    /* Instruction TLB */
    PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, m->name, (strlen("instruction_TLB.overall") + 1));
    strcpy(m->name, "instruction_TLB.overall");
    strcpy(m->name_md5, perfexpert_md5_string(m->name));
    m->value = 0.0;
    m->expression = evaluator_create("PAPI_TLB_IM * TLB_lat / PAPI_TOT_INS");
    if (NULL == m->expression) {
        OUTPUT(("%s (%s)", _ERROR("invalid expression"), m->name));
        return PERFEXPERT_ERROR;
    }
    perfexpert_hash_add_str(my_module_globals.metrics_by_name, name_md5, m);

    return PERFEXPERT_SUCCESS;
}

/* generate_mic_branch_metrics */
static int generate_mic_branch_metrics(void) {
    lcpi_metric_t *m;

    /* Overall */
    PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, m->name,
        (strlen("branch_instructions.overall") + 1));
    strcpy(m->name, "branch_instructions.overall");
    strcpy(m->name_md5, perfexpert_md5_string(m->name));
    m->value = 0.0;
    m->expression = evaluator_create("((PAPI_BR_INS * BR_lat) + (PAPI_BR_MSP * "
        "BR_miss_lat)) / PAPI_TOT_INS");
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

/* generate_mic_floating_point_instr */
static int generate_mic_floating_point_instr(void) {
    lcpi_metric_t *m;

    /* Floating-point instructions: overall */
    PERFEXPERT_ALLOC(lcpi_metric_t, m, sizeof(lcpi_metric_t));
    PERFEXPERT_ALLOC(char, m->name, (strlen("FP_instructions.overall") + 1));
    strcpy(m->name, "FP_instructions.overall");
    strcpy(m->name_md5, perfexpert_md5_string(m->name));
    m->value = 0.0;
    m->expression = evaluator_create(
        "(PAPI_VEC_INS * FP_slow_lat) / PAPI_TOT_INS");
    if (NULL == m->expression) {
        OUTPUT(("%s (%s)", _ERROR("invalid expression"), m->name));
        return PERFEXPERT_ERROR;
    }
    perfexpert_hash_add_str(my_module_globals.metrics_by_name, name_md5, m);

    return PERFEXPERT_SUCCESS;
}

// EOF
