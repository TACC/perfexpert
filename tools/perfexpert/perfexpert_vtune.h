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

#ifndef PERFEXPERT_VTUNE_H_
#define PERFEXPERT_VTUNE_H_

#ifdef __cplusplus
extern "C" {
#endif

#define VTUNE_EXPERIMENT_FILE     "experiment_vtune.conf"
#define VTUNE_MIC_EXPERIMENT_FILE "experiment_vtune_mic.conf"

/* HPCToolkit stuff (binaries should be in the path) */
#define VTUNE_AMPLIFIER "amplxe-cl"

/* Function declarations */
int measurements_vtune(void);

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_VTUNE_H_ */
