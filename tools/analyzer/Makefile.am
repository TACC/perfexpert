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

# SUBDIRS = argp hpcdata analyzer

bin_PROGRAMS = perfexpert_analyzer

perfexpert_analyzer_SOURCES = analyzer_main.c \
	analyzer_hpctoolkit.c \
	analyzer_lcpi.c \
	analyzer_logic.c \
	analyzer_machine.c \
	analyzer_options.c \
	analyzer_output.c \
	analyzer_profile.c

# TODO: I don't like libxml2 include here. Move it to configure :/
perfexpert_analyzer_CPPFLAGS = -I$(srcdir)/../common -I/usr/include/libxml2/

perfexpert_analyzer_CFLAGS = -O3

# TODO: I don't like libxml2 and matheval here. Move it to configure :/
perfexpert_analyzer_LDFLAGS = -lxml2 -lmatheval

# EOF
