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

SUBDIRS = $(ENABLE_PERFEXPERT) $(ENABLE_RECOMMENDER) $(ENABLE_PR)
SUBDIRS += $(ENABLE_CT) $(ENABLE_CR)

install-data-local:
	$(MKDIR_P) $(datadir)/$(PACKAGE_NAME)/codes
	$(INSTALL) -m 644 $(top_srcdir)/contrib/codes/mm_naive.c \
		$(datadir)/$(PACKAGE_NAME)/codes
	if (test x"$(WITH_OPTTRAN_SUPPORT)" = x"yes"); then \
		$(INSTALL) -m 644 $(top_srcdir)/contrib/opttran.sh $(bindir); \
	fi
	
install-data-hook:
	if (test x"$(WITH_OPTTRAN_SUPPORT)" = x"yes"); then \
		cd $(bindir); rm opttran.sh; $(LN_S) opttran.sh opttran; \
	fi

# EOF
