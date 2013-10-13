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

#ifndef SPECIFICS_X86_64_H_
#define SPECIFICS_X86_64_H_

#define WORD_1 "8"
#define WORD_2 "16"
#define WORD_3 "24"
#define WORD_4 "32"
#define WORD_5 "40"

#define AX "rax"
#define BX "rbx"
#define CX "rcx"
#define DX "rdx"

#define SI "rsi"
#define DI "rdi"

#define BP "rbp"
#define SP "rsp"

#define MOV  "movq"
#define XOR  "xorq"
#define ADD  "addq"
#define SUB  "subq"
#define DEC  "decq"
#define PUSH "pushq"
#define POP  "popq"
#define CMP  "cmpq"

#endif /* SPECIFICS_X86_64_H */
