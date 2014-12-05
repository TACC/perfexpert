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
#------------------------------------------------------------------------------
# Requires autoconf 2.63 at least
AC_PREREQ(2.63)

# Define the package name, version, author, etc.
AC_INIT([cache_sim], [0.1],
    [Use PerfExpert mailing list: perfexpert@lists.tacc.utexas.edu],
    [cache_sim-0.1.tar.gz], [http://www.tacc.com.br/perfexpert/])
AC_COPYRIGHT([University of Texas at Austin])
AC_REVISION([$Revision: 0.1 $])
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_MACRO_DIR([m4])

#------------------------------------------------------------------------------
# Generate output files
#
AC_CONFIG_FILES([Makefile])

# Initialize Automake
AM_INIT_AUTOMAKE([foreign -Wall -Werror])

# Ok Automake 1.12, shut up!
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

# Requirements
AM_PROG_CC_C_O
AC_PROG_INSTALL
LT_INIT

# Check for libm
AC_CHECK_LIB([m],[log2])

#------------------------------------------------------------------------------
# Debug
#
AC_ARG_ENABLE([debug], [AS_HELP_STRING([--enable-debug], [enable debugging])],
    [ENABLE_DEBUG=$enableval], [ENABLE_DEBUG=no]
)
AC_MSG_CHECKING([whether to enable debug])
AS_IF([test x"$ENABLE_DEBUG" = x"yes"], [
    AC_MSG_RESULT([yes])
    AC_DEFINE(DEBUG)
], [
    AC_MSG_RESULT([no])
])

#------------------------------------------------------------------------------
# Headers: generic
#
AC_CHECK_HEADER([math.h],         [], [AC_MSG_ERROR([math.h not found])])
AC_CHECK_HEADER([inttypes.h],     [], [AC_MSG_ERROR([inttypes.h not found])])
AC_CHECK_HEADER([stdint.h],       [], [AC_MSG_ERROR([stdint.h not found])])
AC_CHECK_HEADER([stdio.h],        [], [AC_MSG_ERROR([stdio.h not found])])
AC_CHECK_HEADER([stdlib.h],       [], [AC_MSG_ERROR([stdlib.h not found])])
AC_CHECK_HEADER([string.h],       [], [AC_MSG_ERROR([string.h not found])])

#------------------------------------------------------------------------------
# Done!
#
AC_OUTPUT

# EOF
