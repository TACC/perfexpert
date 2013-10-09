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

bin_PROGRAMS = perfexpert_analyzer

perfexpert_analyzer_CPPFLAGS = -I$(srcdir)/../common
perfexpert_analyzer_CFLAGS = -O3
perfexpert_analyzer_LDFLAGS = -lxml2 -lmatheval
perfexpert_analyzer_SOURCES = analyzer_main.c \
	analyzer_hpctoolkit.c \
	analyzer_lcpi.c \
	analyzer_logic.c \
	analyzer_machine.c \
	analyzer_options.c \
	analyzer_output.c \
	analyzer_profile.c \
	analyzer_sort.c \
	analyzer_vtune.c

# EOF
