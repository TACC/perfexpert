#
# Copyright (c) 2011-2016  University of Texas at Austin. All rights reserved.
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
# Authors: Antonio Gomez-Iglesias, Leonardo Fialho and Ashay Rane
#
# $HEADER$
#

lib_LTLIBRARIES = libperfexpert_module_macvec.la
libperfexpert_module_macvec_la_CPPFLAGS = -I$(srcdir)/../..
libperfexpert_module_macvec_la_LDFLAGS = -lm -lsqlite3 -module \
	-version-info 1:0:0 -export-symbols $(srcdir)/macvec_module.sym
libperfexpert_module_macvec_la_SOURCES = macvec_module.c macvec_counters.c \
    macvec_database.c macvec_options.c macvec_parse.c

install-exec-local:
	$(MKDIR_P) $(DESTDIR)$(sysconfdir)
	$(INSTALL) -m 644 $(top_srcdir)/modules/macvec/macvec_module.sql \
		$(DESTDIR)$(sysconfdir)/
# EOF
