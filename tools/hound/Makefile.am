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

bin_PROGRAMS = hound
hound_SOURCES = hound.c

# Overwrite CFLAGS from 'configure'
if ENABLE_DEBUG
    AM_CFLAGS = -fPIC -DDEBUG_PRINT -g
else
    AM_CFLAGS = -fPIC
endif

install-exec-local:
	./hound > machine.conf
	$(MKDIR_P) $(sysconfdir)
	$(INSTALL) -m 644 machine.conf $(sysconfdir)/
	hwloc-info --output-format xml > memory.conf
	$(INSTALL) -m 644 memory.conf $(sysconfdir)/

# EOF
