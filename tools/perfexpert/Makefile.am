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

bin_PROGRAMS = perfexpert

perfexpert_CFLAGS = -O3
perfexpert_LDFLAGS = -lrt -lsqlite3
perfexpert_LDADD = ../../modules/libperfexpert_module_base.la
perfexpert_CPPFLAGS = -I$(srcdir)/../..
perfexpert_SOURCES = perfexpert_main.c \
	perfexpert_compile.c \
	perfexpert_options.c \
	perfexpert_recommendation.c \
	perfexpert_transformation.c

dist_bin_SCRIPTS = perfexpert_setup_db.sh

install-exec-local:
	$(INSTALL) -m 755 .libs/perfexpert $(DESTDIR)$(bindir)/perfexpert_run_exp

# EOF
