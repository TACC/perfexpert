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

AUTOMAKE_OPTIONS = subdir-objects

lib_LTLIBRARIES = libset.la 

libset_la_SOURCES = ../analyze/cache_info.cpp set_cache_conflict_lib.cpp

libset_la_CXXFLAGS = -I. -Wno-deprecated -I$(srcdir)/../../../ -fopenmp
#libcache_la_CXXFLAGS = -I. -Wno-deprecated -I$(srcdir)/../../../ -fopenmp

include_HEADERS = set_cache_conflict_lib.h
