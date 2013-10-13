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

bin_PROGRAMS = perfexpert_recommender

perfexpert_recommender_CPPFLAGS = -I$(srcdir)/../common
perfexpert_recommender_LDFLAGS = -lsqlite3
perfexpert_recommender_SOURCES = recommender_main.c \
	recommender_input.c \
	recommender_log.c \
	recommender_logic.c \
	recommender_options.c \
	recommender_output.c

# Install the recommendation database
install-exec-local:
	echo "" > $(top_builddir)/warnings_recommender
	$(MKDIR_P) $(localstatedir)
	$(MKDIR_P) $(sysconfdir)
	$(MKDIR_P) $(datadir)/$(PACKAGE_NAME)
	$(SQLITE3_PROGRAM) perfexpert.db < $(top_srcdir)/contrib/recommender.db.txt
	if [ -f $(localstatedir)/perfexpert.db ]; then echo \
		"############################# WARNING #############################" \
		>> $(top_builddir)/warnings_recommender; echo \
		"Your recommendation database was moved to perfexpert.db.old" \
		>> $(top_builddir)/warnings_recommender; echo \
		"############################# WARNING #############################" \
		>> $(top_builddir)/warnings_recommender; fi
	$(INSTALL) -b -m 644 perfexpert.db $(localstatedir)/
	rm perfexpert.db
	echo 1.23 > .perfexpert.db.version
	$(INSTALL) -m 644 .perfexpert.db.version $(localstatedir)/
	$(INSTALL) -m 644 $(top_srcdir)/contrib/recommender_metrics.txt \
		$(sysconfdir)/
	$(INSTALL) -m 644 $(top_srcdir)/contrib/recommender.db.txt \
		$(datadir)/$(PACKAGE_NAME)/

# EOF
