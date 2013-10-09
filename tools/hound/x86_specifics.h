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

#ifndef SPECIFICS_X86_H_
#define SPECIFICS_X86_H_

#define WORD_1 "4"
#define WORD_2 "8"
#define WORD_3 "12"
#define WORD_4 "16"
#define WORD_5 "20"

#define AX "eax"
#define BX "ebx"
#define CX "ecx"
#define DX "edx"

#define SI "esi"
#define DI "edi"

#define BP "ebp"
#define SP "esp"

#define MOV  "movl"
#define XOR  "xorl"
#define ADD  "addl"
#define SUB  "subl"
#define DEC  "decl"
#define PUSH "pushl"
#define POP  "popl"
#define CMP  "cmpl"

#endif /* SPECIFICS_X86_H */
