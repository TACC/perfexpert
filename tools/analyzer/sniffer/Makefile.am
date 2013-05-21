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

bin_PROGRAMS = sniffer
sniffer_SOURCES = sniffer.c
sniffer_CFLAGS = -O3

install-exec-local:
	./sniffer
	$(MKDIR_P) $(sysconfdir)
	$(INSTALL) -m 644 lcpi.properties $(sysconfdir)/
	echo "#!$(BASH_PROGRAM)" > analyzer_run_exp.sh
	cat $(srcdir)/analyzer_run_exp.sh_license >> analyzer_run_exp.sh
	cat analyzer_run_exp.sh_header >> analyzer_run_exp.sh
	cat analyzer_run_exp.sh_template >> analyzer_run_exp.sh
	$(INSTALL) -m 755 analyzer_run_exp.sh $(bindir)/

install-exec-hook:
	cd $(bindir); rm analyzer_run_exp; $(LN_S) analyzer_run_exp.sh analyzer_run_exp

# EOF