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

y.tab.c y.tab.h: c_loop1.y; $(YACC) -d c_loop1.y
lex.yy.c: c_loop1.l; $(LEX) c_loop1.l

bin_PROGRAMS = c_loop1

c_loop1_SOURCES = y.tab.c lex.yy.c

# EOF
