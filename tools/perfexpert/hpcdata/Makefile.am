#
# Copyright (c) 2013  University of Texas at Austin. All rights reserved.
#
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#

EXTRA_DIST = hpcdata.jar

# The following commands should use TAB, not SPACES
all:
	$(ANT_PROGRAM) -f build.xml

clean-local:
	rm hpcdata.jar
	rm -rf edu

install-exec-local: all
	$(MKDIR_P) $(libdir)
	$(INSTALL) -m 644 hpcdata.jar $(libdir)

# EOF