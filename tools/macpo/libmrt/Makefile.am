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

lib_LIBRARIES = libmrt.a

# FIXME: This is ugly! Move histogram and AVL code to separate subdirectories.
libmrt_a_SOURCES = mrt.cpp ../analyze/avl_tree.cpp ../analyze/histogram.cpp
libmrt_a_CXXFLAGS = -I$(srcdir) -I$(srcdir)/../include \
    -I$(srcdir)/../analyze/include

include_HEADERS = mrt.h ../include/macpo_record.h


# EOF
