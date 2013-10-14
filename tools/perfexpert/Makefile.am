#
# Copyright (c) 2011-2013  University of Texas at Austin. All rights reserved.
#
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# This file is part of PerfExpert.
#
# PerfExpert is free software: you can redistribute it and/or modify it under
# the terms of the The University of Texas at Austin Research License
# 
# PerfExpert is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
# A PARTICULAR PURPOSE.
# 
# Authors: Leonardo Fialho and Ashay Rane
#
# $HEADER$
#

bin_PROGRAMS = perfexpert

perfexpert_CFLAGS = -O3
perfexpert_LDFLAGS = -lrt
perfexpert_CPPFLAGS = -I$(srcdir)/../common
perfexpert_SOURCES = perfexpert_main.c \
	perfexpert_analysis.c \
	perfexpert_compile.c \
	perfexpert_hpctoolkit.c \
	perfexpert_measurements.c \
	perfexpert_options.c \
	perfexpert_recommendation.c \
	perfexpert_transformation.c \
	perfexpert_vtune.c

dist_bin_SCRIPTS = perfexpert_get_compiler_info.pl

# EOF
