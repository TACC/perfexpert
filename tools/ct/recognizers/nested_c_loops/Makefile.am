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

CLEANFILES = y.tab.c y.tab.h lex.yy.c

y.tab.c y.tab.h: nested_c_loops.y; $(YACC) -d $(srcdir)/nested_c_loops.y
lex.yy.c: nested_c_loops.l; $(LEX) $(srcdir)/nested_c_loops.l

bin_PROGRAMS = pr_nested_c_loops

pr_nested_c_loops_SOURCES = y.tab.c lex.yy.c
pr_nested_c_loops_CFLAGS = -O3

# EOF
