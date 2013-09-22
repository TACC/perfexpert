/*
 * Copyright (c) 2013  University of Texas at Austin. All rights reserved.
 *
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
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
 * Author: Leonardo Fialho
 *
 * $HEADER$
 */

#ifndef PERFEXPERT_HPCTOOLKIT_H_
#define PERFEXPERT_HPCTOOLKIT_H_

#ifdef __cplusplus
extern "C" {
#endif

/* HPCToolkit stuff (binaries should be in the path) */
#define HPCSTRUCT "hpcstruct"
#define HPCRUN    "hpcrun"
#define HPCPROF   "hpcprof"

/* Function declarations */
int measurements_hpctoolkit(void);
int run_hpcstruct(void);
int run_hpcrun(void);
int run_hpcrun_knc(void);
int run_hpcprof(void);

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_HPCTOOLKIT_H_ */
