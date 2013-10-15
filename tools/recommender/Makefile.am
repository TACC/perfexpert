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
	$(MKDIR_P) $(sysconfdir)
	echo 1.23 > perfexpert.db.version
	$(INSTALL) -m 644 perfexpert.db.version $(sysconfdir)/
	$(INSTALL) -m 644 $(top_srcdir)/tools/recommender/perfexpert.db.sql \
		$(sysconfdir)/
	$(INSTALL) -m 644 $(top_srcdir)/tools/recommender/recommender_metrics.txt \
		$(sysconfdir)/

# EOF
