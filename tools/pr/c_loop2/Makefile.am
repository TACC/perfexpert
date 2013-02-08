#
# Copyright (c) 2013  University of Texas at Austin. All rights reserved.
#
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#

CLEANFILES = y.tab.c y.tab.h lex.yy.c

y.tab.c y.tab.h: c_loop2.y; $(YACC) -d $(srcdir)/c_loop2.y
lex.yy.c: c_loop2.l; $(LEX) $(srcdir)/c_loop2.l

bin_PROGRAMS = c_loop2

c_loop2_SOURCES = y.tab.c lex.yy.c

# EOF
