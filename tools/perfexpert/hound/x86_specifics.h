/*
 * Copyright (C) 2013 The University of Texas at Austin
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
 * Author: Ashay Rane
 */

#ifndef    SPECIFICS_X86_H
#define    SPECIFICS_X86_H

#define    WORD_1    "4"
#define    WORD_2    "8"
#define    WORD_3    "12"
#define    WORD_4    "16"
#define    WORD_5    "20"

#define    AX    "eax"
#define    BX    "ebx"
#define    CX    "ecx"
#define    DX    "edx"

#define    SI    "esi"
#define DI    "edi"

#define BP    "ebp"
#define SP    "esp"

#define MOV     "movl"
#define XOR     "xorl"
#define ADD     "addl"
#define SUB     "subl"
#define DEC     "decl"
#define PUSH    "pushl"
#define POP     "popl"
#define    CMP    "cmpl"

#endif    /* SPECIFICS_X86_H */
