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

#ifndef PERFEXPERT_MODULE_HPCTOOLKIT_PARSER_H_
#define PERFEXPERT_MODULE_HPCTOOLKIT_PARSER_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __XML_PARSER_H__
#include <libxml/parser.h>
#endif

/* Modules headers */
#include "hpctoolkit_types.h"

/* PerfExpert common headers */
#include "common/perfexpert_list.h"

/* Function declarations */
static int parse_profile(xmlDocPtr document, xmlNodePtr node,
    perfexpert_list_t *profiles, hpctoolkit_profile_t *profile,
    hpctoolkit_callpath_t *parent, int loopdepth);

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_MODULE_HPCTOOLKIT_PARSER_H_ */
