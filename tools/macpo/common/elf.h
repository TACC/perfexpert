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

#ifndef TOOLS_MACPO_COMMON_ELF_H_
#define TOOLS_MACPO_COMMON_ELF_H_

#include <dlfcn.h>

std::string get_function_name(void* function_address) {
    Dl_info dl_info;
    std::string function_name = "unknown";
    if (dladdr(function_address, &dl_info) != 0 && dl_info.dli_sname != NULL) {
        function_name = std::string(dl_info.dli_sname);
    }

    return function_name;
}

#endif  // TOOLS_MACPO_COMMON_ELF_H_
