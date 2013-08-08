/*
 * Copyright (c) 2013  University of Texas at Austin. All rights reserved.
 *
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
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
 * Author: Leonardo Fialho
 *
 * $HEADER$
 */

#ifndef PERFEXPERT_CONSTANTS_H_
#define PERFEXPERT_CONSTANTS_H_

#ifdef __cplusplus
extern "C" {
#endif
    
/**
 * Some contants here are used to return errors from functions. Those codes
 * should be defined using 'exponential values', e.g. 1, 2, 4, 8, etc. Thus,
 * it is possible to combine more than one error code in the return value.
 */

/** PERFEXPERT_UNDEFINED should be used when it is not an error */
#define PERFEXPERT_UNDEFINED -2
/** PERFEXPERT_FAILURE should be used when an error is -1 */
#define PERFEXPERT_FAILURE   -1
/** PERFEXPERT_SUCCESS should be used in case of success */
#define PERFEXPERT_SUCCESS    0
/** PERFEXPERT_ERROR should be used in case of general error */
#define PERFEXPERT_ERROR      1
/** PERFEXPERT_NO_REC should be used when we run out of recommendation */
#define PERFEXPERT_NO_REC     2
/** PERFEXPERT_NO_PAT should be used when no pattern matches */
#define PERFEXPERT_NO_PAT     3
/** PERFEXPERT_NO_PAT should be used when transformations can't be applied */
#define PERFEXPERT_NO_TRANS   4

#define PERFEXPERT_TRUE   1 /**< used to return boolean values */
#define PERFEXPERT_FALSE  0 /**< used to return boolean values */

/** Buffers size, will be used for:
 * - parsing INPUT file
 * - parsing metrics file
 * - storing SQL statements (including the 'functions')
 * - maybe something else
 */
#define BUFFER_SIZE       4096
#define MAX_FRAGMENT_DATA 1048576
#define PARAM_SIZE        128

/** Default values for some parameters */
#define RECOMMENDATION_DB           "recommendation.db"
#define PERFEXPERT_FRAGMENTS_DIR    "fragments"
#define PERFEXPERT_SOURCE_DIR       "source"
#define METRICS_TABLE               "metric"
#define METRICS_FILE                "recommender-metrics.txt"

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_CONSTANTS_H */
