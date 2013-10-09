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

SUBDIRS = libmrt analyze

bin_PROGRAMS = minst
dist_bin_SCRIPTS = macpo.sh

minst_SOURCES = minst.cpp instrumentor.cpp main.cpp splitter.cpp scourer.cpp \
	expander.cpp stream_counter.cpp
minst_CXXFLAGS = -Wno-deprecated -I$(srcdir)/common
minst_LDFLAGS = -lrose -ljvm

# EOF
