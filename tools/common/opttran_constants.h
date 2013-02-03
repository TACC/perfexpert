/*
 * Copyright (c) 2013  University of Texas at Austin. All rights reserved.
 *
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

#ifndef OPTTRAN_CONSTANTS_H
#define OPTTRAN_CONSTANTS_H

/**
 * Some contants here are used to return errors from functions. Those codes
 * should be defined using 'exponential values', e.g. 1, 2, 4, 8, etc. Thus,
 * it is possible to combine more than one error code in the return value.
 */

/** OPTTRAN_FAILURE should be used when an error is -1 */
#define OPTTRAN_FAILURE  -1
/** OPTTRAN_SUCCESS should be used in case of success */
#define OPTTRAN_SUCCESS  0
/** OPTTRAN_ERROR should be used in case of general error */
#define OPTTRAN_ERROR    1

#define OPTTRAN_TRUE   1 /**< used to return boolean values */
#define OPTTRAN_FALSE  0 /**< used to return boolean values */

#endif /* OPTTRAN_CONSTANTS_H */
