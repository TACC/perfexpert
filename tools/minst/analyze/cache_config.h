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
 
#ifndef CACHE_CONFIG_H
#define CACHE_CONFIG_H

#define	L1_SIZE	32768
#define	L2_SIZE	262144
#define	L3_SIZE	8388608

// Codes for later expansion
#define	L1	1
#define	L2	2
#define	L3	3
#define LOCAL_MEM	4
#define REMOTE_MEM	5

#define	L1_LATENCY	3
#define	L2_LATENCY	9
#define	L3_LATENCY	30
#define	LOCAL_MEM_LATENCY	100	// Not the worst-case latency... (?)
#define	REMOTE_MEM_LATENCY	200	// Not the worst-case latency... (?)

#define	L1_LINES	1024
#define	L2_LINES	8192
#define L3_LINES	32768

#define	L1_SETS		512
#define	L2_SETS		512
#define	L3_SETS		1024

#define	L1_ASSOC	2
#define	L2_ASSOC	16
#define	L3_ASSOC	32

#define	NUM_L1		16
#define	NUM_L2		8
#define	NUM_L3		4

#define	LINE_SIZE	64

#endif /* CACHE_CONFIG_H */
