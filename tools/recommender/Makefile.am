#
# Copyright (c) 2013  University of Texas at Austin. All rights reserved.
#
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#

AM_CFLAGS = -I$(srcdir)/../common

bin_PROGRAMS = recommender teste

recommender_SOURCES = recommender.c
# recommender_LDFLAGS = -lsqlite3

teste_SOURCES = teste.c
# teste_LDFLAGS =  -L/opt/pips/lib -lpipslibs -lnewgenlibs
# teste_CFLAGS = -I/opt/pips/include -L/opt/pips/lib

# Install the recommendation database
install-exec-local:
	$(MKDIR_P) $(localstatedir)
	$(MKDIR_P) $(sysconfdir)
	$(SQLITE3_PROGRAM) $(localstatedir)/recommendation.db < $(top_srcdir)/contrib/recommendation.db.txt
	$(INSTALL) -m 644 $(top_srcdir)/contrib/recommender-metrics.txt $(sysconfdir)

# EOF