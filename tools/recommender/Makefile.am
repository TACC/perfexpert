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

perfexpert_recommender_CPPFLAGS = -I$(srcdir)/../..
perfexpert_recommender_LDFLAGS = -lsqlite3 -lm
perfexpert_recommender_SOURCES = recommender_main.c \
	recommender_options.c \
	recommender_logic.c

# Install the recommendation database
install-exec-local:
	$(MKDIR_P) $(DESTDIR)$(sysconfdir)
	$(INSTALL) -m 644 $(top_srcdir)/tools/recommender/recommender.sql \
		$(DESTDIR)$(sysconfdir)/

# EOF
