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

lib_LTLIBRARIES = libperfexpert_module_sqlrules.la
libperfexpert_module_sqlrules_la_CPPFLAGS = -I$(srcdir)/../..
libperfexpert_module_sqlrules_la_LDFLAGS = -module -lsqlite3 -lm \
	-version-info 1:0:0 -export-symbols $(srcdir)/sqlrules_module.sym
libperfexpert_module_sqlrules_la_SOURCES = sqlrules_module.c \
	sqlrules_logic.c \
	sqlrules_options.c

# Install the recommendation database
install-exec-local:
	$(MKDIR_P) $(DESTDIR)$(sysconfdir)
	$(INSTALL) -m 644 $(top_srcdir)/modules/sqlrules/sqlrules_module.sql \
		$(DESTDIR)$(sysconfdir)/

# EOF
