#
# Copyright (c) 2013  University of Texas at Austin. All rights reserved.
#
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# This file is part of OptTran and PerfExpert.
#
# OptTran as well PerfExpert are free software: you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation, either version 3 of the License,
# or (at your option) any later version.
#
# OptTran and PerfExpert are distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
# General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with OptTran or PerfExpert. If not, see <http://www.gnu.org/licenses/>.
#
# Author: Leonardo Fialho
#
# $HEADER$
#

bin_PROGRAMS = recommender

recommender_SOURCES = main.cpp recommender.c
recommender_CPPFLAGS = -I$(srcdir)/../common
recommender_CFLAGS = -O3

# Adding support to Rose
if WITH_ROSE_SUPPORT
recommender_CPPFLAGS += -DHAVE_ROSE
recommender_SOURCES += recommender_extract_fragment.cpp \
	recommenderTraversal.cpp
endif


# Install the recommendation database
install-exec-local:
	if [ -f $(top_builddir)/warnings_recommender ]; then \
		rm $(top_builddir)/warnings_recommender; fi
	$(MKDIR_P) $(localstatedir)
	$(MKDIR_P) $(sysconfdir)
	$(MKDIR_P) $(datadir)/$(PACKAGE_NAME)
	$(SQLITE3_PROGRAM) recommendation.db \
		< $(top_srcdir)/contrib/recommendation.db.txt
	if [ -f $(localstatedir)/recommendation.db ]; then echo \
		"############################# WARNING #############################" \
		>> $(top_builddir)/warnings_recommender; echo \
		"Your recommendation database was moved to recommendation.db.old" \
		>> $(top_builddir)/warnings_recommender; echo \
		"############################# WARNING #############################" \
		>> $(top_builddir)/warnings_recommender; fi
	$(INSTALL) -b -m 644 recommendation.db $(localstatedir)
	rm recommendation.db
	$(INSTALL) -m 644 $(top_srcdir)/contrib/recommender-metrics.txt \
		$(sysconfdir)
	$(INSTALL) -m 644 $(top_srcdir)/contrib/recommendation.db.txt \
		$(datadir)/$(PACKAGE_NAME)
	$(INSTALL) -m 644 $(top_srcdir)/contrib/recommender-input_example.txt \
		$(datadir)/$(PACKAGE_NAME)

# EOF
