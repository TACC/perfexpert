#
# Copyright (c) 2013  University of Texas at Austin. All rights reserved.
#
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#

bin_PROGRAMS = sniffer
sniffer_SOURCES = sniffer.c
sniffer_LDADD = /usr/local/lib/libpapi.a

install-exec-local:
	./sniffer
	$(MKDIR_P) $(sysconfdir)
	$(INSTALL) -m 644 lcpi.properties $(sysconfdir)
	echo "#!$(BASH_PROGRAM)" > perfexpert_run_exp.sh
	cat $(srcdir)/perfexpert_run_exp.sh_license >> perfexpert_run_exp.sh
	cat perfexpert_run_exp.sh_header >> perfexpert_run_exp.sh
	cat perfexpert_run_exp.sh_template >> perfexpert_run_exp.sh
	$(INSTALL) -m 755 perfexpert_run_exp.sh $(bindir)
	
install-exec-hook:
	cd $(bindir); $(LN_S) perfexpert_run_exp.sh perfexpert_run_exp

# EOF