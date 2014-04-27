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

SUBDIRS = libmrt analyze tests

bin_PROGRAMS = macpo
dist_bin_SCRIPTS = macpo.sh

macpo_SOURCES = argparse.cpp minst.cpp instrumentor.cpp main.cpp streams.cpp \
                aligncheck.cpp ir_methods.cpp loop_traversal.cpp tracer.cpp \
                generic_vars.cpp vector_strides.cpp analysis_profile.cpp \
                tripcount.cpp branchpath.cpp pntr_overlap.cpp stride_check.cpp \
                traversal.cpp reuse_dist.cpp
macpo_CXXFLAGS = -Wno-deprecated -I$(srcdir)/include
macpo_LDFLAGS = -lrose -ljvm

# EOF
