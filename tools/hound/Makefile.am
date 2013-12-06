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

bin_PROGRAMS = hound
hound_SOURCES = hound.c

# Overwrite CFLAGS from 'configure'
if ENABLE_DEBUG
    AM_CFLAGS = -fPIC -DDEBUG_PRINT -g
else
    AM_CFLAGS = -fPIC
endif

install-exec-local:
	./hound > hound.sql
	$(MKDIR_P) $(DESTDIR)$(sysconfdir)
	$(INSTALL) -m 644 hound.sql $(DESTDIR)$(sysconfdir)/

# EOF
