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

# TODO: replace ROSE include/libdir by some 'configure' parameter (the same for Java)
recommender_CPPFLAGS = -I$(srcdir)/../common -I/opt/rose/include
recommender_LDADD = -lrose -ljvm 
recommender_LDFLAGS = -L/opt/rose/lib -L/usr/lib/jvm/java-7-openjdk-amd64/jre/lib/amd64/server
recommender_SOURCES = main.cpp \
	recommender.c \
	recommender_extract_fragment.cpp \
	recommenderTraversal.cpp

# Install the recommendation database
install-exec-local:
	$(MKDIR_P) $(localstatedir)
	$(MKDIR_P) $(sysconfdir)
	$(MKDIR_P) $(datadir)/$(PACKAGE_NAME)
	$(SQLITE3_PROGRAM) recommendation.db \
		< $(top_srcdir)/contrib/recommendation.db.txt
	$(INSTALL) -m 644 recommendation.db $(localstatedir)
		if [ -f $(localstatedir)/recommendation.db ]; then echo \
		!!!WARNING!!! Database file already exists, press ENTER \
		to overwrite it; read line; fi
	rm -rf recommendation.db
	$(INSTALL) -m 644 $(top_srcdir)/contrib/recommender-metrics.txt \
		$(sysconfdir)
	$(INSTALL) -m 644 $(top_srcdir)/contrib/recommendation.db.txt \
		$(datadir)/$(PACKAGE_NAME)
	$(INSTALL) -m 644 $(top_srcdir)/contrib/recommender-input_example.txt \
		$(datadir)/$(PACKAGE_NAME)

# EOF
