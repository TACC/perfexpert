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

lib_LTLIBRARIES = libperfexpert_module_timb.la
libperfexpert_module_timb_la_CPPFLAGS = -I$(srcdir)/../..
libperfexpert_module_timb_la_LDFLAGS = -module -lsqlite3 -lm \
	-version-info 1:0:0 -export-symbols $(srcdir)/timb_module.sym
libperfexpert_module_timb_la_SOURCES = timb_module.c \
	timb_output.c

# EOF
