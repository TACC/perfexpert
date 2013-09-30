#
# Copyright (c) 2013  University of Texas at Austin. All rights reserved.
#
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# This file is part of PerfExpert.
#
# PerfExpert is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# PerfExpert is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
# A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with PerfExpert. If not, see <http://www.gnu.org/licenses/>.
#
# Author: Leonardo Fialho and Ashay Rane
#
# $HEADER$
#------------------------------------------------------------------------------

# Requires autoconf 2.67 at least
AC_PREREQ(2.63)

# Define the package name, version, author, etc.
AC_INIT([perfexpert], [4.1], [Leonardo Fialho: fialho@utexas.edu],
    [perfexpert.tar.gz], [http://www.tacc.com.br/perfexpert/])
AC_COPYRIGHT([University of Texas at Austin])
AC_REVISION([$Revision: 4.1 $])
AC_CONFIG_AUX_DIR(config)

#------------------------------------------------------------------------------
# Generate output files and global config.h and install_dirs.h files
#
AC_CONFIG_FILES([Makefile tools/Makefile])
AC_CONFIG_SRCDIR([tools/common/config.h.in])
AC_CONFIG_HEADERS([tools/common/config.h])
AC_CONFIG_FILES([tools/common/install_dirs.h])
AC_CONFIG_FILES([tools/hound/Makefile])
AC_CONFIG_FILES([tools/sniffer/Makefile])
AC_CONFIG_FILES([tools/analyzer/Makefile])
AC_CONFIG_FILES([tools/perfexpert/Makefile])
AC_CONFIG_FILES([tools/perfexpert/perfexpert_get_info.pl])

# Initialize Automake
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
# Ok Automake 1.12, shut up!
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

# Just to have a cleaner output during 'make'
# AM_SILENT_RULES([yes])

# To save hound from -O2 on CFLAGS. This overwrites any user-defined CFLAGS :-/
CFLAGS="-g"
CXXFLAGS="-g"

# Requirements
AC_PROG_CPP
AM_PROG_CC_C_O
AC_C_VOLATILE
AC_C_INLINE
AC_PROG_CXX
AC_PROG_CXXCPP
AC_PROG_CXX_C_O
AC_PROG_MKDIR_P
AC_PROG_LN_S
AC_PROG_INSTALL
AC_PROG_RANLIB
AC_TYPE_SIZE_T
AC_FUNC_MALLOC

#------------------------------------------------------------------------------
# Check for the operation system to include specific defines (if needed!)
#
AC_CANONICAL_HOST
AC_MSG_CHECKING([for the OS])
AS_CASE([${host}],
    [*-*-darwin*], [
        AC_MSG_RESULT([you are a happy Mac user, I suppose...])
        AC_MSG_ERROR([...but we do not support OSX])],
    [*-*-freebsd*], [
        AC_MSG_RESULT([hum... Are you happy with FreeBSD?])
        AC_MSG_ERROR([...but we do not support FreeBSD])],
    [*-*-linux*], [
        AC_DEFINE([OS_LINUX], [1], [The host OS is Linux])
        AC_MSG_RESULT([one more geek using Linux])],
    [*-*-netbsd*], [
        AC_MSG_RESULT([Really? There is at least one NetBSD user in the world!])
        AC_MSG_ERROR([...but we do not support NetBSD])],
    [*-*-solaris2*], [
        AC_MSG_RESULT([We understand, you're stucked on Solaris, right?])
        AC_MSG_ERROR([...but we do not support Solaris])],
    [*-*-mingw* | *-*-cygwin*], [
        AC_MSG_RESULT([I will pray for you, Windows user :-/])
        AC_MSG_ERROR([...but we do not support M$ Windows])],
    [AC_MSG_RESULT([Unsupported operating system: ${host}])
    AC_MSG_ERROR([unsupported plataform])
])

#------------------------------------------------------------------------------
# Debug
#
AC_ARG_ENABLE([debug], [AS_HELP_STRING([--enable-debug], [enable debugging])],
    [ENABLE_DEBUG=$enableval], [ENABLE_DEBUG=no]
)
AC_MSG_CHECKING([whether to enable debug])
AS_IF([test x"$ENABLE_DEBUG" = x"yes"], [
    AC_MSG_RESULT([yes])
    AC_DEFINE(ENABLE_DEBUG, [1], [Enable/Disable debug])
], [
    AC_MSG_RESULT([no])
    AC_DEFINE(ENABLE_DEBUG, [0], [Enable/Disable debug])
])
AM_CONDITIONAL([ENABLE_DEBUG], [test x"$ENABLE_DEBUG" = x"yes"])

#------------------------------------------------------------------------------
# Headers: generic
#
# perfexpert, analyzer, recommender, hound, and sniffer
AC_CHECK_HEADER([argp.h],      [], [AC_MSG_ERROR([argp.h not found])])
AC_CHECK_HEADER([errno.h],     [], [AC_MSG_ERROR([errno.h not found])])
AC_CHECK_HEADER([fcntl.h],     [], [AC_MSG_ERROR([fcntl.h not found])])
AC_CHECK_HEADER([float.h],     [], [AC_MSG_ERROR([float.h not found])])
AC_CHECK_HEADER([getopt.h],    [], [AC_MSG_ERROR([getopt.h not found])])
AC_CHECK_HEADER([inttypes.h],  [], [AC_MSG_ERROR([inttypes.h not found])])
AC_CHECK_HEADER([math.h],      [], [AC_MSG_ERROR([math.h not found])])
AC_CHECK_HEADER([sched.h],     [], [AC_MSG_ERROR([sched.h not found])])
AC_CHECK_HEADER([stdarg.h],    [], [AC_MSG_ERROR([stdarg.h not found])])
AC_CHECK_HEADER([stddef.h],    [], [AC_MSG_ERROR([stddef.h not found])])
AC_CHECK_HEADER([stdio.h],     [], [AC_MSG_ERROR([stdio.h not found])])
AC_CHECK_HEADER([stdlib.h],    [], [AC_MSG_ERROR([stdlib.h not found])])
AC_CHECK_HEADER([string.h],    [], [AC_MSG_ERROR([string.h not found])])
AC_CHECK_HEADER([strings.h],   [], [AC_MSG_ERROR([strings.h not found])])
AC_CHECK_HEADER([sys/mman.h],  [], [AC_MSG_ERROR([sys/mman.h not found])])
AC_CHECK_HEADER([sys/stat.h],  [], [AC_MSG_ERROR([sys/stat.h not found])])
AC_CHECK_HEADER([sys/types.h], [], [AC_MSG_ERROR([sys/types.h not found])])
AC_CHECK_HEADER([time.h],      [], [AC_MSG_ERROR([time.h not found])])
AC_CHECK_HEADER([unistd.h],    [], [AC_MSG_ERROR([unistd.h not found])])

#------------------------------------------------------------------------------
# Binaries: Bash shell, PERL, HPCToolkit programs, and SQLite
#
AC_PATH_TOOL([BASH_PROGRAM],      [bash])
AC_PATH_TOOL([PERL_PROGRAM],      [perl])
AC_PATH_TOOL([HPCSTRUCT_PROGRAM], [hpcstruct])
AC_PATH_TOOL([HPCRUN_PROGRAM],    [hpcrun])
AC_PATH_TOOL([HPCPROF_PROGRAM],   [hpcprof])
AC_PATH_TOOL([SQLITE3_PROGRAM],   [sqlite3])

#------------------------------------------------------------------------------
# Externals (headers, lib)
#
AC_ARG_WITH([externals],
    [AS_HELP_STRING([--with-externals],
        [use this argument to set prefix path of PerfExpert externals])], [
    CPPFLAGS+=" -I$withval/include"
    LDFLAGS+=" -L$withval/lib"])

#------------------------------------------------------------------------------
# 'perfexpert' requirements
#
# Libraries: rt
#
AC_CHECK_LIB([rt], [clock_gettime], [], [AC_MSG_ERROR([librt.so not found])])

AS_IF([test "x$PERL_PROGRAM"      == x], [AC_MSG_ERROR([perl not found])])
AS_IF([test "x$HPCSTRUCT_PROGRAM" == x], [AC_MSG_ERROR([hpcstruct not found])])
AS_IF([test "x$HPCRUN_PROGRAM"    == x], [AC_MSG_ERROR([hpcrun not found])])
AS_IF([test "x$HPCPROF_PROGRAM"   == x], [AC_MSG_ERROR([hpcprof not found])])

#------------------------------------------------------------------------------
# 'hound' requirements
#

#------------------------------------------------------------------------------
# 'sniffer' requirements
#
# PAPI support (headers and lib)
#
AC_ARG_WITH([papi],
    [AS_HELP_STRING([--with-papi],
        [use this argument to set PAPI prefix path (where PAPI library and
        include directories are located)])], [
    CPPFLAGS+=" -I$withval/include"
    LDFLAGS+=" -L$withval/lib"], [])

AC_ARG_WITH([papi-lib],
    [AS_HELP_STRING([--with-papi-lib],
        [use this argument to set PAPI library path])],
    [LDFLAGS+=" -L$withval"], [])

AC_ARG_WITH([papi-include],
    [AS_HELP_STRING([--with-papi-include],
        [use this argument to set PAPI include path])],
    [CPPFLAGS+=" -I$withval"], [])

AC_CHECK_LIB([papi], [PAPI_library_init, PAPI_add_event], [],
    [AC_MSG_ERROR([papi.so not found])])
AC_CHECK_HEADER([papi.h], [], [AC_MSG_ERROR([papi.h not found])])

#------------------------------------------------------------------------------
# 'perfexpert_analyzer' requirements
#
# GNU Math Evaluator support (headers and lib)
#
AC_ARG_WITH([matheval],
    [AS_HELP_STRING([--with-matheval],
        [use this argument to set GNU Math Evaluator prefix path (where matheval
        library and include directories are located)])], [
    CPPFLAGS+=" -I$withval/include"
    LDFLAGS+=" -L$withval/lib"])

AC_ARG_WITH([matheval-lib],
    [AS_HELP_STRING([--with-matheval-lib],
        [use this argument to set GNU matheval library path])],
    [LDFLAGS+=" -L$withval"])

AC_ARG_WITH([matheval-include],
    [AS_HELP_STRING([--with-matheval-include],
        [use this argument to set GNU matheval include path])],
    [CPPFLAGS+=" -I$withval"])

AC_CHECK_LIB([matheval], [evaluator_create], [],
    [AC_MSG_ERROR([libmatheval.so not found])])
AC_CHECK_HEADER([matheval.h], [], [AC_MSG_ERROR([matheval.h not found])])

#
# XML2 support (headers and lib)
#
AC_ARG_WITH([libxml2],
    [AS_HELP_STRING([--with-libxml2],
        [use this argument to set LibXML2 prefix path (where LibXML2 library and
        include directories are located)])], [
    CPPFLAGS+=" -I$withval/include"
    LDFLAGS+=" -L$withval/lib"])

AC_ARG_WITH([libxml2-lib],
    [AS_HELP_STRING([--with-libxml2-lib],
        [use this argument to set LibXML2 library path])],
    [LDFLAGS+=" -L$withval"])

AC_ARG_WITH([libxml2-include],
    [AS_HELP_STRING([--with-libxml2-include],
        [use this argument to set LibXML2 include path])],
    [CPPFLAGS+=" -I$withval"])

AC_CHECK_LIB([xml2], [xmlParseFile], [], [AC_MSG_ERROR([libxml2.so not found])])
AC_CHECK_HEADER([libxml/parser.h], [], [AC_MSG_ERROR([parser.h not found])])

#------------------------------------------------------------------------------
# 'recommender' requirements
#
AC_ARG_ENABLE([recommender],
    [AS_HELP_STRING([--disable-recommender],
        [disable optimization recommender])],
    [ENABLE_RECOMMENDER=$enableval], [ENABLE_RECOMMENDER=yes])

AC_MSG_CHECKING([whether to enable the optimization recommender tool])
AS_IF([test x"$ENABLE_RECOMMENDER" = xyes], [
    AC_MSG_RESULT([yes])

    #
    # SQLite support (headers, lib, and program)
    #
    AC_ARG_WITH([sqlite],
        [AS_HELP_STRING([--with-sqlite],
            [use this argument to set SQLite3 prefix path (where SQLite3 library
            and include directories are located)])], [
        CPPFLAGS+=" -I$withval/include"
        LDFLAGS+=" -L$withval/lib"])

    AC_ARG_WITH([sqlite-lib],
        [AS_HELP_STRING([--with-sqlite-lib],
            [use this argument to set SQLite3 library path])],
        [LDFLAGS+=" -L$withval"])

    AC_ARG_WITH([sqlite-include],
        [AS_HELP_STRING([--with-sqlite-include],
            [use this argument to set SQLite3 include path])],
        [CPPFLAGS+=" -I$withval"])

    AC_CHECK_LIB([sqlite3], [sqlite3_open, sqlite3_close, sqlite3_exec], [],
        [AC_MSG_ERROR([not found: libsqlit3.so])])
    AC_CHECK_HEADER([sqlite3.h], [], [AC_MSG_ERROR([not found: sqlite3.h])])

    ENABLE_RECOMMENDER=recommender
    AC_CONFIG_FILES([tools/recommender/Makefile])
], [
    AC_MSG_RESULT([no])
    ENABLE_RECOMMENDER=
])
AC_SUBST([ENABLE_RECOMMENDER])

#------------------------------------------------------------------------------
# 'minst' (MACPO) requirements
#
AC_ARG_ENABLE([minst],
    [AS_HELP_STRING([--disable-minst], [disable data access instrumentation])],
    [ENABLE_MINST=$enableval], [ENABLE_MINST=yes])

AC_ARG_WITH([jvm],
    [AS_HELP_STRING([--with-jvm], 
        [use this argument to set JVM library path])],
    [LDFLAGS+=" -L$withval"])

AC_LANG_PUSH([C++])

AC_MSG_CHECKING([whether to enable Minst (MACPO)])
AS_IF([test x"$ENABLE_MINST" = xyes], [
    AC_MSG_RESULT([yes])

    #
    # JVM support
    #
    AC_CHECK_LIB([jvm], [JVM_Open, JVM_Close], [],
        [AC_MSG_ERROR([not found: libjvm.so (Java Virtual Machine)])])

    #
    # ROSE Compiler support (headers and lib)
    #
    AC_ARG_WITH([rose],
        [AS_HELP_STRING([--with-rose],
            [use this argument to set ROSE prefix path (where ROSE library and
            include directories are located)])], [
        CPPFLAGS+=" -I$withval/include"
        LDFLAGS+=" -L$withval/lib"])

    AC_ARG_WITH([rose-libs],
        [AS_HELP_STRING([--with-rose-libs],
            [use this argument to set Compiler library path])],
        [LDFLAGS+=" -L$withval"])

    AC_ARG_WITH([rose-include],
        [AS_HELP_STRING([--with-rose-include],
            [use this argument to set ROSE include path])],
        [CPPFLAGS+=" -I$withval/include"])

    AC_CHECK_HEADER([rose.h], [], [AC_MSG_ERROR([not found: rose.h])])

    #
    # GNU Multiple Precision Arithmetic Library support (headers and lib)
    #
    AC_ARG_WITH([gmp],
        [AS_HELP_STRING([--with-gmp],
            [use this argument to set GNU Multiple Precision Library prefix path
            (where GMP library and include directories are located)])], [
        CPPFLAGS+=" -I$withval/include"
        LDFLAGS+=" -L$withval/lib"])

    AC_ARG_WITH([gmp-lib],
        [AS_HELP_STRING([--with-gmp-lib],
            [use this argument to set GMP library path])],
        [LDFLAGS+=" -L$withval"])

    AC_ARG_WITH([gmp-include],
        [AS_HELP_STRING([--with-gmp-include],
            [use this argument to set GMP include path])],
        [CPPFLAGS+=" -I$withval"])

    AC_CHECK_LIB([gmp], [__gmpf_init, __gmpf_clear], [],
        [AC_MSG_ERROR([not found: libgmp.so])])
    AC_CHECK_HEADER([gmp.h], [], [AC_MSG_ERROR([not found: gmp.h])])

    #
    # Check for Google SparseHash (headers)
    #
    AC_ARG_WITH([sparse-hash-include],
        [AS_HELP_STRING([--with-sparse-hash-include],
            [use this argument to set Google Sparse Hash include path])],
        [CPPFLAGS+=" -I$withval"], [])

    AC_CHECK_HEADER([google/sparse_hash_map], [],
        [AC_MSG_ERROR([not found: google/sparse_hash_map])])

    ENABLE_MINST=minst
    AC_CONFIG_FILES([tools/minst/Makefile])
    AC_CONFIG_FILES([tools/minst/macpo.sh], [chmod +x tools/minst/macpo.sh])
    AC_CONFIG_FILES([tools/minst/libmrt/Makefile])
    AC_CONFIG_FILES([tools/minst/analyze/Makefile])
], [
    AC_MSG_RESULT([no])
    ENABLE_MINST=
])
AC_SUBST([ENABLE_MINST])
AC_LANG_POP

#------------------------------------------------------------------------------
# 'perfexpert_ct' requirements
#
AC_ARG_ENABLE([perfexpert_ct],
    [AS_HELP_STRING([--disable-perfexpert_ct], [disable code transformer])],
    [ENABLE_CT=$enableval], [ENABLE_CT=yes])

AC_MSG_CHECKING([whether to enable code transformation])
AS_IF([test x"$ENABLE_CT" = xyes], [
    AC_MSG_RESULT([yes])

    #
    # JVM support
    #
    AC_CHECK_LIB([jvm], [JVM_Open, JVM_Close], [],
        [AC_MSG_ERROR([not found: libjvm.so (Java Virtual Machine)])])

    #
    # ROSE Compiler support (headers and lib)
    #
    AC_ARG_WITH([rose],
        [AS_HELP_STRING([--with-rose],
            [use this argument to set ROSE prefix path (where ROSE library and
            include directories are located)])], [
        CPPFLAGS+=" -I$withval/include"
        LDFLAGS+=" -L$withval/lib"])

    AC_ARG_WITH([rose-libs],
        [AS_HELP_STRING([--with-rose-libs],
            [use this argument to set Compiler library path])],
        [LDFLAGS+=" -L$withval"])

    AC_ARG_WITH([rose-include],
        [AS_HELP_STRING([--with-rose-include],
            [use this argument to set ROSE include path])],
        [CPPFLAGS+=" -I$withval/include"])

    AC_LANG_PUSH([C++])
    AC_CHECK_HEADER([rose.h], [], [AC_MSG_ERROR([not found: rose.h])])
    AC_LANG_POP

    # Check for Lex/Flex and Yacc/Bison
    AC_PROG_LEX
    AC_PROG_YACC

    ENABLE_CT=ct
    AC_DEFINE([HAVE_CODE_TRANSFORMATION], [1], [Enable code transformation])
    AC_CONFIG_FILES([tools/ct/Makefile])
    AC_CONFIG_FILES([tools/ct/recognizers/Makefile])
    AC_CONFIG_FILES([tools/ct/recognizers/nested_c_loops/Makefile])
    AC_CONFIG_FILES([tools/ct/transformers/Makefile])
    AC_CONFIG_FILES([tools/ct/transformers/c_loop_interchange/Makefile])
], [
    AC_MSG_RESULT([no])
    ENABLE_CT=
])
AC_SUBST([ENABLE_CT])

#------------------------------------------------------------------------------
AC_CONFIG_FILES([tests/run_tests.pl], [chmod +x tests/run_tests.pl])
LIBS=""
AC_OUTPUT
AC_MSG_NOTICE([CC=$CC])
AC_MSG_NOTICE([CXX=$CXX])
AC_MSG_NOTICE([CPPFLAGS=$CPPFLAGS])
AC_MSG_NOTICE([CFLAGS=$CFLAGS])
AC_MSG_NOTICE([CXXFLAGS=$CXXFLAGS])
AC_MSG_NOTICE([LDFLAGS=$LDFLAGS])

# EOF
