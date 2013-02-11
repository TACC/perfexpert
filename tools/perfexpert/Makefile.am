#
# Copyright (c) 2013  University of Texas at Austin. All rights reserved.
#
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#

SUBDIRS = argp hpcdata perfexpert $(ENABLE_HOUND) $(ENABLE_SNIFFER)

dist_bin_SCRIPTS = hpcdata.sh perfexpert.sh $(ENABLE_PERFEXPERT_RUN_EXP)

# The following commands should use TAB, not SPACES
install-exec-local: install-dist_binSCRIPTS
	$(MKDIR_P) $(libdir)
	$(INSTALL) -m 644 $(top_srcdir)/external/log4j-1.2.16.jar $(libdir)
	$(INSTALL) -m 644 $(top_srcdir)/external/log4j.properties $(libdir)
	$(MKDIR_P) $(bindir)

# EOF