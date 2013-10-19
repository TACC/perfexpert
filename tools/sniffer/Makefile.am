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

bin_PROGRAMS = sniffer
sniffer_SOURCES = sniffer.c
sniffer_CFLAGS = -O3
sniffer_LDFLAGS = -lpapi

install-exec-local:
	./sniffer
	$(MKDIR_P) $(sysconfdir)
	$(INSTALL) -m 644 lcpi.conf $(DESTDIR)$(sysconfdir)/
	$(INSTALL) -m 644 experiment.conf \
		$(DESTDIR)$(sysconfdir)/experiment_hpctoolkit.conf
	$(INSTALL) -m 644 experiment.conf \
		$(DESTDIR)$(sysconfdir)/experiment_vtune.conf

# EOF