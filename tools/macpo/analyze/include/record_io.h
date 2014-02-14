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

#ifndef RECORD_IO_H_
#define RECORD_IO_H_

#include <cstring>
#include <ctime>
#include <fcntl.h>
#include <vector>
#include <iostream>

#include "analysis_defs.h"
#include "generic_defs.h"
#include "macpo_record.h"

int print_trace_records(const global_data_t& global_data);
int read_file(const char* filename, global_data_t& global_data);

#endif  /* RECORD_IO_H_ */
