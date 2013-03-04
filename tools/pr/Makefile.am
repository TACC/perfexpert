#
# Copyright (c) 2013  University of Texas at Austin. All rights reserved.
#
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# This file is part of OptTran and PerfExpert.
#
# OptTran as well PerfExpert are free software: you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation, either version 3 of the License,
# or (at your option) any later version.
#
# OptTran and PerfExpert are distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
# General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with OptTran or PerfExpert. If not, see <http://www.gnu.org/licenses/>.
#
# Author: Leonardo Fialho
#
# $HEADER$
#

bin_PROGRAMS = opttran_pr

opttran_pr_SOURCES = main.c
opttran_pr_CPPFLAGS = -I$(srcdir)/../common
opttran_pr_CFLAGS = -O3

SUBDIRS = c_loop1 c_loop2

install-data-local:
	$(MKDIR_P) $(datadir)/$(PACKAGE_NAME)/codes
	$(INSTALL) -m 644 $(top_srcdir)/contrib/opttran_pr-input_example.txt \
		$(datadir)/$(PACKAGE_NAME)/
	$(INSTALL) -m 644 $(top_srcdir)/contrib/codes/pr-c_loop1_example_DO.c \
		$(datadir)/$(PACKAGE_NAME)/codes/
	$(INSTALL) -m 644 $(top_srcdir)/contrib/codes/pr-c_loop1_example_FOR.c \
		$(datadir)/$(PACKAGE_NAME)/codes/
	$(INSTALL) -m 644 $(top_srcdir)/contrib/codes/pr-c_loop1_example_WHILE.c \
		$(datadir)/$(PACKAGE_NAME)/codes/
	$(INSTALL) -m 644 $(top_srcdir)/contrib/codes/pr-c_loop2_example_DO.c \
		$(datadir)/$(PACKAGE_NAME)/codes/
	$(INSTALL) -m 644 $(top_srcdir)/contrib/codes/pr-c_loop2_example_FOR.c \
		$(datadir)/$(PACKAGE_NAME)/codes/
	$(INSTALL) -m 644 $(top_srcdir)/contrib/codes/pr-c_loop2_example_WHILE.c \
		$(datadir)/$(PACKAGE_NAME)/codes/

# EOF