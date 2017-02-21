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
# Authors: Antonio Gomez-Iglesias
#
# $HEADER$
#

lib_LTLIBRARIES = libperfexpert_module_lustre_wrapper.la
libperfexpert_module_lustre_wrapper_la_CFLAGS = -I$(srcdir)/../..
libperfexpert_module_lustre_wrapper_la_CPFLAGS = -I$(srcdir)/../..
libperfexpert_module_lustre_wrapper_la_LDFLAGS = -module -lm -lunwind -ldl \
	-version-info 1:0:0
libperfexpert_module_lustre_wrapper_la_SOURCES = lustre_wrapper.c


lib_LTLIBRARIES += libperfexpert_module_lustre.la
libperfexpert_module_lustre_la_CPPFLAGS = -I$(srcdir)/../..
libperfexpert_module_lustre_la_LDFLAGS = -module -lsqlite3 -lm \
	-version-info 1:0:0 -export-symbols $(srcdir)/lustre_module.sym
libperfexpert_module_lustre_la_SOURCES = lustre_module.c \
	lustre_output.c


#EOF
