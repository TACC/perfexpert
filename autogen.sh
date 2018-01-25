#!/bin/sh
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

# Run autoreconf
mkdir -p m4
mkdir -p lib/cache_sim/m4
mkdir -p config
aclocal -I m4 --install
autoreconf -ivf --warnings=all,no-obsolete,no-override -I config
# autoreconf --install

# EOF
