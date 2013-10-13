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

CLEANFILES = y.tab.c y.tab.h lex.yy.c

y.tab.c y.tab.h: nested_c_loops.y; $(YACC) -d $(srcdir)/nested_c_loops.y
lex.yy.c: nested_c_loops.l; $(LEX) $(srcdir)/nested_c_loops.l

bin_PROGRAMS = nested_c_loops

nested_c_loops_SOURCES = y.tab.c lex.yy.c
nested_c_loops_CFLAGS = -O3

# EOF
