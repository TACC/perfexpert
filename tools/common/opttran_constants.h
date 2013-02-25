/*
 * Copyright (c) 2013  University of Texas at Austin. All rights reserved.
 *
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * This file is part of OptTran and PerfExpert.
 *
 * OptTran as well PerfExpert are free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * OptTran and PerfExpert are distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with OptTran or PerfExpert. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Leonardo Fialho
 *
 * $HEADER$
 */

#ifndef OPTTRAN_CONSTANTS_H_
#define OPTTRAN_CONSTANTS_H_

#ifdef __cplusplus
extern "C" {
#endif
    
/**
 * Some contants here are used to return errors from functions. Those codes
 * should be defined using 'exponential values', e.g. 1, 2, 4, 8, etc. Thus,
 * it is possible to combine more than one error code in the return value.
 */

/** OPTTRAN_UNDEFINED should be used when it is not an error */
#define OPTTRAN_UNDEFINED -2
/** OPTTRAN_FAILURE should be used when an error is -1 */
#define OPTTRAN_FAILURE   -1
/** OPTTRAN_SUCCESS should be used in case of success */
#define OPTTRAN_SUCCESS    0
/** OPTTRAN_ERROR should be used in case of general error */
#define OPTTRAN_ERROR      1

#define OPTTRAN_TRUE   1 /**< used to return boolean values */
#define OPTTRAN_FALSE  0 /**< used to return boolean values */

#ifdef __cplusplus
}
#endif

#endif /* OPTTRAN_CONSTANTS_H */
