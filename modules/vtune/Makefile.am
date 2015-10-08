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

lib_LTLIBRARIES = libperfexpert_module_vtune.la
libperfexpert_module_vtune_la_CPPFLAGS = -I$(srcdir)/../..
libperfexpert_module_vtune_la_LDFLAGS = -lsqlite3 -lrt \
	-module -version-info 1:0:0 -export-symbols $(srcdir)/vtune_module.sym
libperfexpert_module_vtune_la_SOURCES = vtune_module.c \
	vtune_event.c \
	vtune_options.c \
	vtune_database.c \
	vtune_tools.c

# Install vtune module database
install-exec-local:
	$(MKDIR_P) $(DESTDIR)$(sysconfdir)
	$(INSTALL) -m 644 $(top_srcdir)/modules/vtune/vtune_module.sql \
		$(DESTDIR)$(sysconfdir)/

# EOF
