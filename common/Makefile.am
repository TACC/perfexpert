#
# Copyright (c) 2011-2017  University of Texas at Austin. All rights reserved.
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

lib_LTLIBRARIES = libperfexpert_common.la
libperfexpert_common_la_CPPFLAGS = -I$(srcdir)/..
libperfexpert_common_la_LDFLAGS = -lltdl -version-info 1:0:0
libperfexpert_common_la_SOURCES = perfexpert_alloc.c \
	perfexpert_backup.c \
	perfexpert_base64.c \
	perfexpert_cpuinfo.c \
	perfexpert_database.c \
	perfexpert_fork.c \
	perfexpert_list.c \
	perfexpert_md5.c \
	perfexpert_log.c \
	perfexpert_string.c \
	perfexpert_time.c \
	perfexpert_util.c

# Install architecture database
install-exec-local:
	$(MKDIR_P) $(DESTDIR)$(sysconfdir)
	$(INSTALL) -m 644 $(top_srcdir)/common/perfexpert_architecture.sql \
		$(DESTDIR)$(sysconfdir)/

# EOF
