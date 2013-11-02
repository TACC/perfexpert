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

lib_LTLIBRARIES = libperfexpert_module_hpctoolkit.la
libperfexpert_module_hpctoolkit_la_CPPFLAGS = -I$(srcdir)/../..
libperfexpert_module_hpctoolkit_la_LDFLAGS = -lxml2 -lpapi -lsqlite3 -module \
	-version-info 1:0:0 -export-symbols $(srcdir)/hpctoolkit_module.sym
libperfexpert_module_hpctoolkit_la_SOURCES = hpctoolkit_module.c \
	hpctoolkit_database.c \
	hpctoolkit_parser.c \
	hpctoolkit_profile.c \
	hpctoolkit_tools.c

# Install hpctoolkit module database
install-exec-local:
	$(MKDIR_P) $(DESTDIR)$(sysconfdir)
	$(INSTALL) -m 644 $(top_srcdir)/modules/hpctoolkit/hpctoolkit_module.sql \
		$(DESTDIR)$(sysconfdir)/

# EOF
