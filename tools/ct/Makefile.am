#
# Copyright (c) 2013  University of Texas at Austin. All rights reserved.
#
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# This file is part of PerfExpert.
#
# PerfExpert is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# PerfExpert is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
# A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with PerfExpert. If not, see <http://www.gnu.org/licenses/>.
#
# Author: Leonardo Fialho
#
# $HEADER$
#

bin_PROGRAMS = perfexpert_ct

perfexpert_ct_SOURCES = ct_real_main.c \
	ct_input.c \
	ct_logic.c \
	ct_main.cpp \
	ct_options.c \
	ct_pattern.c \
	ct_recommendation.c \
	ct_rose.cpp \
	ct_transformation.c \
	ct_traversal.cpp

perfexpert_ct_CPPFLAGS = -I$(srcdir)/../common

perfexpert_ct_LDFLAGS = -lsqlite3 -lrose -ljvm

perfexpert_ct_CFLAGS = -O3

SUBDIRS = recognizers transformers

# EOF
