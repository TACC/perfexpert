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

bin_PROGRAMS = perfexpert_ct

perfexpert_ct_CPPFLAGS = -I$(srcdir)/../..
perfexpert_ct_LDFLAGS = -lsqlite3 -lrose -ljvm
perfexpert_ct_CFLAGS = -O3
perfexpert_ct_SOURCES = ct_real_main.c \
	ct_database.c \
	ct_input.c \
	ct_main.cpp \
	ct_options.c \
	ct_rose.cpp \
	ct_tools.c \
	ct_traversal.cpp

SUBDIRS = recognizers transformers

# EOF
