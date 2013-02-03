#
# Copyright (c) 2013  University of Texas at Austin. All rights reserved.
#
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#

y.tab.c: c_loop2.y; $(YACC) -d c_loop2.y
lex.yy.c: c_loop2.l; $(LEX) c_loop2.l

bin_PROGRAMS = c_loop2

c_loop2_SOURCES = y.tab.c lex.yy.c

# EOF
