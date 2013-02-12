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
	$(INSTALL) -m 644 $(top_srcdir)/contrib/perfexpert-experiment.xml $(datadir)
	$(INSTALL) -m 644 $(top_srcdir)/contrib/pr-c_loop1_example_DO.c $(datadir)
	$(INSTALL) -m 644 $(top_srcdir)/contrib/pr-c_loop1_example_FOR.c $(datadir)
	$(INSTALL) -m 644 $(top_srcdir)/contrib/pr-c_loop1_example_WHILE.c $(datadir)
	$(INSTALL) -m 644 $(top_srcdir)/contrib/pr-c_loop2_example_DO.c $(datadir)
	$(INSTALL) -m 644 $(top_srcdir)/contrib/pr-c_loop2_example_FOR.c $(datadir)
	$(INSTALL) -m 644 $(top_srcdir)/contrib/pr-c_loop2_example_WHILE.c $(datadir)
	$(INSTALL) -m 644 $(top_srcdir)/contrib/pr-input.txt $(datadir)
	$(INSTALL) -m 644 $(top_srcdir)/contrib/recommendation.db.txt $(datadir)
	$(INSTALL) -m 644 $(top_srcdir)/contrib/recommender-input_example.txt $(datadir)

# EOF