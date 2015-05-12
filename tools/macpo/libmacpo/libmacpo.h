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

#ifndef TOOLS_MACPO_LIBMACPO_LIBMACPO_H_
#define TOOLS_MACPO_LIBMACPO_LIBMACPO_H_

#include <stdint.h>
#include "actions.h"

typedef struct __tag_code_location_t {
    uint16_t action;
    const char* name;
    uint32_t line_number;

    struct __tag_code_location_t* next;
} code_location_t;

typedef struct {
    uint8_t no_compile_flag;
    uint8_t disable_sampling_flag;
    uint8_t profiling_flag;
    uint8_t dynamic_inst_flag;

    const char* base_compiler;
    const char* backup_filename;

    code_location_t* location;
} macpo_options_t;

extern "C" {
uint8_t get_no_compile_flag(const macpo_options_t* macpo_options);
void set_no_compile_flag(macpo_options_t* macpo_options, uint8_t flag);
void set_dynamic_instrumentation_flag(macpo_options_t *macpo_options,
        uint8_t flag);

uint8_t get_disable_sampling_flag(const macpo_options_t* macpo_options);
void set_disable_sampling_flag(macpo_options_t* macpo_options, uint8_t flag);

uint8_t get_profiling_flag(const macpo_options_t* macpo_options);
void set_profiling_flag(macpo_options_t* macpo_options, uint8_t flag);

const char* get_base_compiler(const macpo_options_t* macpo_options);
void set_base_compiler(macpo_options_t* macpo_options,
        const char* compiler_path);

const char* get_backup_filename(const macpo_options_t* macpo_options);
void set_backup_filename(macpo_options_t* macpo_options, const char* filename);

uint8_t instrument_function(macpo_options_t* macpo_options, uint16_t action,
        const char* name);
uint8_t instrument_block(macpo_options_t* macpo_options, uint16_t action,
        const char* name, uint32_t line_number);

uint8_t destroy_options(macpo_options_t* macpo_options);

uint8_t instrument(macpo_options_t* macpo_options, const char* args[],
        uint32_t arg_count);
}   // extern "C"

#endif  // TOOLS_MACPO_LIBMACPO_LIBMACPO_H_
