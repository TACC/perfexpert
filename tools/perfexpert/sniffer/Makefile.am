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

bin_PROGRAMS = sniffer
sniffer_SOURCES = sniffer.c
sniffer_CFLAGS = -O3

install-exec-local:
	./sniffer
	$(MKDIR_P) $(sysconfdir)
	$(INSTALL) -m 644 lcpi.properties $(sysconfdir)/
	echo "#!$(BASH_PROGRAM)" > perfexpert_run_exp.sh
	cat $(srcdir)/perfexpert_run_exp.sh_license >> perfexpert_run_exp.sh
	cat perfexpert_run_exp.sh_header >> perfexpert_run_exp.sh
	cat perfexpert_run_exp.sh_template >> perfexpert_run_exp.sh
	$(INSTALL) -m 755 perfexpert_run_exp.sh $(bindir)/

install-exec-hook:
	cd $(bindir); rm perfexpert_run_exp; $(LN_S) perfexpert_run_exp.sh perfexpert_run_exp

# EOF