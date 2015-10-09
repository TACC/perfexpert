#
# Copyright (c) 2011-2015  University of Texas at Austin. All rights reserved.
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

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = lib common modules tools

# .PHONY: check

# check:
#	cd $(top_builddir)/tests/
#	./run_tests.pl

# Set database version
install-exec-local:
	echo 1.29 > perfexpert.db.version
	test -d $(DESTDIR)$(sysconfdir) || mkdir $(DESTDIR)$(sysconfdir)
	$(INSTALL) -m 644 perfexpert.db.version $(DESTDIR)$(sysconfdir)/

# EOF
