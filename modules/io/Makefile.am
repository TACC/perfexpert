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

lib_LTLIBRARIES = libperfexpert_module_io_wrapper.la
libperfexpert_module_io_wrapper_la_CFLAGS = -I$(srcdir)/../..
libperfexpert_module_io_wrapper_la_CPFLAGS = -I$(srcdir)/../..
libperfexpert_module_io_wrapper_la_LDFLAGS = -module -lm -lunwind -ldl -lutil\
	-version-info 1:0:0
libperfexpert_module_io_wrapper_la_SOURCES = io_wrapper.c


lib_LTLIBRARIES += libperfexpert_module_io.la
libperfexpert_module_io_la_CPPFLAGS = -I$(srcdir)/../..
libperfexpert_module_io_la_LDFLAGS = -module -lsqlite3 -lm \
	-version-info 1:0:0 -export-symbols $(srcdir)/io_module.sym
libperfexpert_module_io_la_SOURCES = io_module.c \
	io_output.c io_options.c


#EOF
