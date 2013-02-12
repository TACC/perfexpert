#
# Copyright (c) 2013  University of Texas at Austin. All rights reserved.
#
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#

SUBDIRS = $(ENABLE_PERFEXPERT) $(ENABLE_RECOMMENDER) $(ENABLE_PR)
SUBDIRS += $(ENABLE_CT) $(ENABLE_CR)

install-data-local:
	$(MKDIR_P) $(datadir)/$(PACKAGE_NAME)
	$(INSTALL) -m 644 $(top_srcdir)/contrib/codes/perfexpert-experiment.xml \
		$(datadir)/codes/$(PACKAGE_NAME)
	$(INSTALL) -m 644 $(top_srcdir)/contrib/codes/pr-c_loop1_example_DO.c \
		$(datadir)/codes/$(PACKAGE_NAME)
	$(INSTALL) -m 644 $(top_srcdir)/contrib/codes/pr-c_loop1_example_FOR.c \
		$(datadir)/codes/$(PACKAGE_NAME)
	$(INSTALL) -m 644 $(top_srcdir)/contrib/codes/pr-c_loop1_example_WHILE.c \
		$(datadir)/codes/$(PACKAGE_NAME)
	$(INSTALL) -m 644 $(top_srcdir)/contrib/codes/pr-c_loop2_example_DO.c \
		$(datadir)/codes/$(PACKAGE_NAME)
	$(INSTALL) -m 644 $(top_srcdir)/contrib/codes/pr-c_loop2_example_FOR.c \
		$(datadir)/codes/$(PACKAGE_NAME)
	$(INSTALL) -m 644 $(top_srcdir)/contrib/codes/pr-c_loop2_example_WHILE.c \
		$(datadir)/codes/$(PACKAGE_NAME)
	$(INSTALL) -m 644 $(top_srcdir)/contrib/pr-input.txt \
		$(datadir)/$(PACKAGE_NAME)
	$(INSTALL) -m 644 $(top_srcdir)/contrib/recommendation.db.txt \
		$(datadir)/$(PACKAGE_NAME)
	$(INSTALL) -m 644 $(top_srcdir)/contrib/recommender-input_example.txt \
		$(datadir)/$(PACKAGE_NAME)

# EOF