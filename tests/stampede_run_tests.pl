#!/usr/bin/perl
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

# Dedicated to Ashay, who loves Perl. 

use Term::ANSIColor;
use warnings;
use strict;

my $SRCDIR=".";
$ENV{PERFEXPERT_SRCDIR}=$SRCDIR;
$ENV{PERFEXPERT_BIN}="./tools/perfexpert";

foreach my $test (`ls $SRCDIR/tests`) {
    chomp($test);
    if ("run_tests.pl.in" ne $test) {
        printf("%s $test\n", colored("Testing", 'blue'));
        if (-x "$SRCDIR/tests/$test/run") {
            system("$SRCDIR/tests/$test/run");
            if ($? eq 0) {
                printf("%s\n\n", colored("Test PASSED!", 'green'));
            } else {
                printf("%s\n\n", colored("Test FAILED!", 'red'));
            }
        } else {
            printf("%s\n\n", colored("Test SKIPED!", 'magenta'));
        }
    }
}  

# EOF
