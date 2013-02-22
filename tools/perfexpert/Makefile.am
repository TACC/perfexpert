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

# perfexpert_run_exp will be installed by sniffer

SUBDIRS = argp hpcdata perfexpert $(ENABLE_HOUND) $(ENABLE_SNIFFER)

dist_bin_SCRIPTS = hpcdata.sh perfexpert.sh

# The following commands should use TAB, not SPACES
install-exec-local: install-dist_binSCRIPTS
	$(MKDIR_P) $(libdir)
	$(MKDIR_P) $(datadir)/$(PACKAGE_NAME)
	$(INSTALL) -m 644 $(top_srcdir)/external/log4j-1.2.16.jar $(libdir)
	$(INSTALL) -m 644 $(top_srcdir)/external/log4j.properties $(libdir)
	$(INSTALL) -m 644 $(top_srcdir)/contrib/perfexpert-experiment.xml \
		$(datadir)/$(PACKAGE_NAME)
	$(INSTALL) -m 644 $(top_srcdir)/contrib/opt-sug-intel.pdb $(localstatedir)

install-exec-hook:
	cd $(bindir); rm hpcdata; $(LN_S) hpcdata.sh hpcdata
	cd $(bindir); rm perfexpert; $(LN_S) perfexpert.sh perfexpert

# EOF