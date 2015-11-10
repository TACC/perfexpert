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

bin_PROGRAMS = macpo-analyze

macpo_analyze_SOURCES = main.cpp record_io.cpp record_analysis.cpp        \
    cache_info.cpp histogram.cpp stride_analysis.cpp latency_analysis.cpp \
    vector_stride_analysis.cpp argp_custom.cpp associative_cache.cpp      \
    set_cache_conflict_analysis.cpp
macpo_analyze_CXXFLAGS = -I$(srcdir)/include -I$(srcdir)/../common -I$(srcdir)/../libmrt -I$(srcdir)/../../.. -fopenmp -O0 -g
macpo_analyze_LDFLAGS = -fopenmp -lgmp -lgsl -lgslcblas -lhwloc -O0 -g
