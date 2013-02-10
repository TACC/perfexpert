/*
    Copyright (C) 2013 The University of Texas at Austin

    This file is part of PerfExpert.

    PerfExpert is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    PerfExpert is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with PerfExpert.  If not, see <http://www.gnu.org/licenses/>.

    Author: Ashay Rane
*/

#ifndef	SPECIFICS_X86_64_H
#define	SPECIFICS_X86_64_H

#define	WORD_1	"8"
#define	WORD_2	"16"
#define	WORD_3	"24"
#define	WORD_4	"32"
#define	WORD_5	"40"

#define	AX	"rax"
#define	BX	"rbx"
#define	CX	"rcx"
#define	DX	"rdx"

#define	SI	"rsi"
#define DI	"rdi"

#define BP	"rbp"
#define SP	"rsp"

#define MOV     "movq"
#define XOR     "xorq"
#define ADD     "addq"
#define SUB     "subq"
#define DEC     "decq"
#define PUSH    "pushq"
#define POP     "popq"
#define CMP	"cmpq"

#endif	/* SPECIFICS_X86_64_H */
