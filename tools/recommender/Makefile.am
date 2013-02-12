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

bin_PROGRAMS = recommender
recommender_SOURCES = recommender.c
recommender_LDFLAGS = -lsqlite3

# Install the recommendation database
install-exec-local:
	$(MKDIR_P) $(localstatedir)
	$(MKDIR_P) $(sysconfdir)
	$(SQLITE3_PROGRAM) $(localstatedir)/recommendation.db < $(top_srcdir)/contrib/recommendation.db.txt
	$(INSTALL) -m 644 $(top_srcdir)/contrib/recommender-metrics.txt $(sysconfdir)

# EOF