#
# Copyright (c) 2013  University of Texas at Austin. All rights reserved.
#
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# This file is part of PerfExpert.
#
# PerfExpert is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# PerfExpert is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
# A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with PerfExpert. If not, see <http://www.gnu.org/licenses/>.
#
# Author: Leonardo Fialho
#
# $HEADER$
#

bin_PROGRAMS = recommender

recommender_SOURCES = main.c
recommender_CPPFLAGS = -I$(srcdir)/../common
recommender_LDFLAGS = -lsqlite3

# Install the recommendation database
install-exec-local:
	echo "" > $(top_builddir)/warnings_recommender
	$(MKDIR_P) $(localstatedir)
	$(MKDIR_P) $(sysconfdir)
	$(MKDIR_P) $(datadir)/$(PACKAGE_NAME)
	$(SQLITE3_PROGRAM) perfexpert.db \
		< $(top_srcdir)/contrib/recommender.db.txt
	if [ -f $(localstatedir)/perfexpert.db ]; then echo \
		"############################# WARNING #############################" \
		>> $(top_builddir)/warnings_recommender; echo \
		"Your recommendation database was moved to perfexpert.db.old" \
		>> $(top_builddir)/warnings_recommender; echo \
		"############################# WARNING #############################" \
		>> $(top_builddir)/warnings_recommender; fi
	$(INSTALL) -b -m 644 perfexpert.db $(localstatedir)/
	rm perfexpert.db
	echo 1.1 > .perfexpert.db.version
	$(INSTALL) -m 644 .perfexpert.db.version $(localstatedir)/
	$(INSTALL) -m 644 $(top_srcdir)/contrib/recommender-metrics.txt \
		$(sysconfdir)/
	$(INSTALL) -m 644 $(top_srcdir)/contrib/recommender.db.txt \
		$(datadir)/$(PACKAGE_NAME)/

# EOF
