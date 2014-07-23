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

#ifndef TOOLS_MACPO_COMMON_ACTIONS_H_
#define TOOLS_MACPO_COMMON_ACTIONS_H_

// Bitmap for ACTION values.
// XXX: ACTION_NONE and ACTION_LAST are special actions!
// Define all other actions after ACTION_NONE and before ACTION_LAST!

const int16_t ACTION_NONE           = 0;
const int16_t ACTION_INSTRUMENT     = 1 <<  0;
const int16_t ACTION_ALIGNCHECK     = 1 <<  1;
const int16_t ACTION_GENTRACE       = 1 <<  2;
const int16_t ACTION_VECTORSTRIDES  = 1 <<  3;
const int16_t ACTION_TRIPCOUNT      = 1 <<  4;
const int16_t ACTION_BRANCHPATH     = 1 <<  5;
const int16_t ACTION_OVERLAPCHECK   = 1 <<  6;
const int16_t ACTION_STRIDECHECK    = 1 <<  7;
const int16_t ACTION_REUSEDISTANCE  = 1 <<  8;
const int16_t ACTION_LAST           = 1 <<  9;

#endif  // TOOLS_MACPO_COMMON_ACTIONS_H_
