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
libperfexpert_module_hpctoolkit_la_SOURCES = hpctoolkit.c
libperfexpert_module_hpctoolkit_la_CPPFLAGS = -I$(srcdir)/../../tools -I$(srcdir)/.. -I$(abs_srcdir)/../../tools/common
libperfexpert_module_hpctoolkit_la_LDFLAGS = -lxml2 -module -version-info 1:0:0 -export-symbols $(srcdir)/../perfexpert_module.sym

# EOF
