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

CLEANFILES = y.tab.c y.tab.h lex.yy.c

y.tab.c y.tab.h: c_loop2.y; $(YACC) -d $(srcdir)/c_loop2.y
lex.yy.c: c_loop2.l; $(LEX) $(srcdir)/c_loop2.l

bin_PROGRAMS = c_loop2

c_loop2_SOURCES = y.tab.c lex.yy.c
c_loop2_CFLAGS = -O3

# EOF
