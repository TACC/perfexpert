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

bin_PROGRAMS = opttran_ci

opttran_ci_SOURCES = main.c rose_functions.cpp
opttran_ci_CPPFLAGS = -I$(srcdir)/../common
opttran_ci_CFLAGS = -O3

install-data-local:
	$(MKDIR_P) $(datadir)/$(PACKAGE_NAME)
	$(INSTALL) -m 644 $(top_srcdir)/contrib/opttran_ci-input_example.txt \
		$(datadir)/$(PACKAGE_NAME)/

# EOF
