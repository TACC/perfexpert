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

recommender_CPPFLAGS = -I$(srcdir)/../common
recommender_SOURCES = main.cpp recommender.c

# Adding support to Rose
if WITH_ROSE_SUPPORT
recommender_CPPFLAGS += -I$(WITH_ROSE_INCLUDE) -DHAVE_ROSE
recommender_LDADD = -lrose -ljvm
recommender_LDFLAGS = -L$(WITH_ROSE_LIBS) -L$(WITH_JVM_LIBS)
recommender_SOURCES += recommender_extract_fragment.cpp \
	recommenderTraversal.cpp
endif !WITH_ROSE_SUPPORT

# recommender_LDADD = -lrose -ljvm
# recommender_LDFLAGS = -L/opt/rose/lib -L/usr/lib/jvm/java-7-openjdk-amd64/jre/lib/amd64/server
# recommender_CPPFLAGS += -I/opt/rose/include

# Install the recommendation database
install-exec-local:
	$(MKDIR_P) $(localstatedir)
	$(MKDIR_P) $(sysconfdir)
	$(MKDIR_P) $(datadir)/$(PACKAGE_NAME)
	$(SQLITE3_PROGRAM) recommendation.db \
		< $(top_srcdir)/contrib/recommendation.db.txt
	$(INSTALL) -m 644 recommendation.db $(localstatedir)
	if [ -f $(localstatedir)/recommendation.db ]; then echo; \
		echo -ne "#################################\n#         !!!WARNING!!!"; \
		echo -ne "         #\n#                               #\n# Database "; \
		echo -e "file already exists, #\n#  press ENTER to overwrite it  #"; \
		echo "#################################"; read line; fi
	rm -rf recommendation.db
	$(INSTALL) -m 644 $(top_srcdir)/contrib/recommender-metrics.txt \
		$(sysconfdir)
	$(INSTALL) -m 644 $(top_srcdir)/contrib/recommendation.db.txt \
		$(datadir)/$(PACKAGE_NAME)
	$(INSTALL) -m 644 $(top_srcdir)/contrib/recommender-input_example.txt \
		$(datadir)/$(PACKAGE_NAME)

# EOF
