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

SUBDIRS = $(ENABLE_HPCTOOLKIT)
SUBDIRS += $(ENABLE_VTUNE)
SUBDIRS += $(ENABLE_LCPI)
SUBDIRS += $(ENABLE_READELF)
SUBDIRS += $(ENABLE_MACPO)
SUBDIRS += $(ENABLE_TIMB)
SUBDIRS += $(ENABLE_MAKE)
SUBDIRS += $(ENABLE_GCC)
SUBDIRS += $(ENABLE_GPP)
SUBDIRS += $(ENABLE_ICC)
SUBDIRS += $(ENABLE_SQLRULES)
SUBDIRS += $(ENABLE_MACVEC)
SUBDIRS += $(ENABLE_LUSTRE)

lib_LTLIBRARIES = libperfexpert_module_base.la
libperfexpert_module_base_la_SOURCES = perfexpert_module_base.c
libperfexpert_module_base_la_CPPFLAGS = -I$(srcdir)/.. -fopenmp
libperfexpert_module_base_la_LDFLAGS = -lltdl -version-info 1:0:0 \
	-export-symbols $(srcdir)/perfexpert_module_base.sym -fopenmp

# EOF
