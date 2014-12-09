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

lib_LTLIBRARIES = libcache_sim.la

libcache_sim_la_CPPFLAGS =
libcache_sim_la_LDFLAGS = -lm -version-info 1:0:0 \
	-export-symbols $(srcdir)/cache_sim.sym
libcache_sim_la_SOURCES = cache_sim.c \
	cache_sim_conflict.c \
	cache_sim_reuse.c \
	cache_sim_prefetcher.c \
	cache_sim_symbol.c \
	cache_policy_lru.c

# EOF
