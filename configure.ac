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

# Requires autoconf 2.67 at least
AC_PREREQ(2.63)

# Define the package name, version, author, etc.
AC_INIT([perfexpert], [4.1.1],
    [PerfExpert mailing list: perfexpert@lists.tacc.utexas.edu],
    [perfexpert-4.1.1.tar.gz], [http://www.tacc.com.br/perfexpert/])
AC_COPYRIGHT([University of Texas at Austin])
AC_REVISION([$Revision: 4.1.1 $])
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_MACRO_DIR([m4])

#------------------------------------------------------------------------------
# Generate output files and global config.h and install_dirs.h files
#
AC_CONFIG_FILES([Makefile tools/Makefile modules/Makefile])
AC_CONFIG_FILES([common/install_dirs.h])
AC_CONFIG_SRCDIR([common/config.h.in])
AC_CONFIG_HEADERS([common/config.h])

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
AC_TYPE_SIZE_T
AC_FUNC_MALLOC
LT_INIT
PKG_PROG_PKG_CONFIG

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
AC_CHECK_HEADER([argp.h],         [], [AC_MSG_ERROR([argp.h not found])])
AC_CHECK_HEADER([dirent.h],       [], [AC_MSG_ERROR([dirent.h not found])])
AC_CHECK_HEADER([errno.h],        [], [AC_MSG_ERROR([errno.h not found])])
AC_CHECK_HEADER([fcntl.h],        [], [AC_MSG_ERROR([fcntl.h not found])])
AC_CHECK_HEADER([float.h],        [], [AC_MSG_ERROR([float.h not found])])
AC_CHECK_HEADER([getopt.h],       [], [AC_MSG_ERROR([getopt.h not found])])
AC_CHECK_HEADER([inttypes.h],     [], [AC_MSG_ERROR([inttypes.h not found])])
AC_CHECK_HEADER([linux/limits.h], [], [AC_MSG_ERROR([linux/limits.h not found])])
AC_CHECK_HEADER([math.h],         [], [AC_MSG_ERROR([math.h not found])])
AC_CHECK_HEADER([sched.h],        [], [AC_MSG_ERROR([sched.h not found])])
AC_CHECK_HEADER([signal.h],       [], [AC_MSG_ERROR([signal.h not found])])
AC_CHECK_HEADER([stdarg.h],       [], [AC_MSG_ERROR([stdarg.h not found])])
AC_CHECK_HEADER([stddef.h],       [], [AC_MSG_ERROR([stddef.h not found])])
AC_CHECK_HEADER([stdio.h],        [], [AC_MSG_ERROR([stdio.h not found])])
AC_CHECK_HEADER([stdlib.h],       [], [AC_MSG_ERROR([stdlib.h not found])])
AC_CHECK_HEADER([string.h],       [], [AC_MSG_ERROR([string.h not found])])
AC_CHECK_HEADER([strings.h],      [], [AC_MSG_ERROR([strings.h not found])])
AC_CHECK_HEADER([sys/mman.h],     [], [AC_MSG_ERROR([sys/mman.h not found])])
AC_CHECK_HEADER([sys/stat.h],     [], [AC_MSG_ERROR([sys/stat.h not found])])
AC_CHECK_HEADER([sys/types.h],    [], [AC_MSG_ERROR([sys/types.h not found])])
AC_CHECK_HEADER([time.h],         [], [AC_MSG_ERROR([time.h not found])])
AC_CHECK_HEADER([unistd.h],       [], [AC_MSG_ERROR([unistd.h not found])])

#------------------------------------------------------------------------------
# Basic binaries: Bash shell
#
AC_PATH_TOOL([BASH_PROGRAM], [bash])

#------------------------------------------------------------------------------
# Argument for pre-requirements (headers, lib)
#
AC_ARG_WITH([externals],
    [AS_HELP_STRING([--with-externals],
        [set prefix path of PerfExpert externals])], [
    CPPFLAGS+=" -I$withval/include"
    LDFLAGS+=" -L$withval/lib"])

AC_ARG_WITH([boost],
    [AS_HELP_STRING([--with-boost], [set Boost library and include path])], [
    CPPFLAGS+=" -I$withval/include"
    LDFLAGS+=" -L$withval/lib"])

AC_ARG_WITH([boost-lib],
    [AS_HELP_STRING([--with-boost-lib], [set Boost library path])],
    [LDFLAGS+=" -L$withval"])

AC_ARG_WITH([boost-include],
    [AS_HELP_STRING([--with-boost-include], [set Boost include path])],
    [CPPFLAGS+=" -I$withval"])

AC_ARG_WITH([libxml2],
    [AS_HELP_STRING([--with-libxml2], [set LibXML2 library and include path])],
    [CPPFLAGS+=" -I$withval/include"
    LDFLAGS+=" -L$withval/lib"])

AC_ARG_WITH([libxml2-lib],
    [AS_HELP_STRING([--with-libxml2-lib], [set LibXML2 library path])],
    [LDFLAGS+=" -L$withval"])

AC_ARG_WITH([libxml2-include],
    [AS_HELP_STRING([--with-libxml2-include], [set LibXML2 include path])],
    [CPPFLAGS+=" -I$withval"])

AC_ARG_WITH([papi],
    [AS_HELP_STRING([--with-papi], [set PAPI library and include path])], [
    CPPFLAGS+=" -I$withval/include"
    LDFLAGS+=" -L$withval/lib"])

AC_ARG_WITH([papi-lib],
    [AS_HELP_STRING([--with-papi-lib], [set PAPI library path])],
    [LDFLAGS+=" -L$withval"])

AC_ARG_WITH([papi-include],
    [AS_HELP_STRING([--with-papi-include], [set PAPI include path])],
    [CPPFLAGS+=" -I$withval"])

AC_ARG_WITH([sqlite],
    [AS_HELP_STRING([--with-sqlite], [set SQLite3 library and include path])], [
    CPPFLAGS+=" -I$withval/include"
    LDFLAGS+=" -L$withval/lib"])

AC_ARG_WITH([sqlite-lib],
    [AS_HELP_STRING([--with-sqlite-lib], [set SQLite3 library path])],
    [LDFLAGS+=" -L$withval"])

AC_ARG_WITH([sqlite-include],
    [AS_HELP_STRING([--with-sqlite-include], [set SQLite3 include path])],
    [CPPFLAGS+=" -I$withval"])

AC_ARG_WITH([matheval],
    [AS_HELP_STRING([--with-matheval],
        [set GNU matheval library and include path])], [
    CPPFLAGS+=" -I$withval/include"
    LDFLAGS+=" -L$withval/lib"])

AC_ARG_WITH([matheval-lib],
    [AS_HELP_STRING([--with-matheval-lib], [set GNU matheval library path])],
    [LDFLAGS+=" -L$withval"])

AC_ARG_WITH([matheval-include],
    [AS_HELP_STRING([--with-matheval-include],
        [set GNU matheval include path])],
    [CPPFLAGS+=" -I$withval"])

AC_ARG_WITH([rose],
    [AS_HELP_STRING([--with-rose], [set ROSE library and include path])], [
    CPPFLAGS+=" -I$withval/include"
    LDFLAGS+=" -L$withval/lib"])

AC_ARG_WITH([rose-libs],
    [AS_HELP_STRING([--with-rose-libs], [set ROSE library path])],
    [LDFLAGS+=" -L$withval"])

AC_ARG_WITH([rose-include],
    [AS_HELP_STRING([--with-rose-include], [set ROSE include path])],
    [CPPFLAGS+=" -I$withval/include"])

AC_ARG_WITH([gmp],
    [AS_HELP_STRING([--with-gmp], [set GMP library and include path])], [
    CPPFLAGS+=" -I$withval/include"
    LDFLAGS+=" -L$withval/lib"])

AC_ARG_WITH([gmp-lib],
    [AS_HELP_STRING([--with-gmp-lib], [set GMP library path])],
    [LDFLAGS+=" -L$withval"])

AC_ARG_WITH([gmp-include],
    [AS_HELP_STRING([--with-gmp-include], [set GMP include path])],
    [CPPFLAGS+=" -I$withval"])

AC_ARG_WITH([sparsehash],
    [AS_HELP_STRING([--with-sparsehash], [set Google SparseHash include path])],
    [CPPFLAGS+=" -I$withval"], [])

#------------------------------------------------------------------------------
# readelf module requirements
#
AC_ARG_ENABLE([module-readelf],
    [AS_HELP_STRING([--disable-module-readelf],
        [disable ReadELF measurement module])],
    [ENABLE_READELF=$enableval], [ENABLE_READELF=yes])

AC_MSG_CHECKING([whether to enable readelf measurement module])
AS_IF([test x"$ENABLE_READELF" = xyes], [
    AC_MSG_RESULT([yes])

    # DWARF (library and header files)
    #
    AC_CHECK_LIB([dwarf], [dwarf_init], [NOOP=noop],
        [AC_MSG_ERROR([libdwarf.so not found])], [-lelf])
    AC_CHECK_HEADER([dwarf.h],    [], [AC_MSG_ERROR([dwarf.h not found])])
    AC_CHECK_HEADER([libdwarf.h], [], [AC_MSG_ERROR([libdwarf.h not found])])

    ENABLE_READELF=readelf
    ENABLED_MODULES+="readelf "
    AC_CONFIG_FILES([modules/readelf/Makefile])
], [
    AC_MSG_RESULT([no])
    ENABLE_READELF=
])
AC_SUBST([ENABLE_READELF])

#------------------------------------------------------------------------------
# HPCToolkit module requirements
#
AC_ARG_ENABLE([module-hpctoolkit],
    [AS_HELP_STRING([--disable-module-hpctoolkit],
        [disable HPCToolkit measurement module])],
    [ENABLE_HPCTOOLKIT=$enableval], [ENABLE_HPCTOOLKIT=yes])

AC_MSG_CHECKING([whether to enable HPCToolkit measurement module])
AS_IF([test x"$ENABLE_HPCTOOLKIT" = xyes], [
    AC_MSG_RESULT([yes])

    # Real Time library
    #
    AC_CHECK_LIB([rt], [clock_gettime], [],
        [AC_MSG_ERROR([librt.so not found])])

    # Programs: hpcstruct, hpcrun, hpcprof
    #
    AC_PATH_TOOL([HPCSTRUCT_PROGRAM], [hpcstruct],
        [AC_MSG_ERROR([hpcstruct not found])])
    AC_PATH_TOOL([HPCRUN_PROGRAM], [hpcrun],
        [AC_MSG_ERROR([hpcrun not found])])
    AC_PATH_TOOL([HPCPROF_PROGRAM], [hpcprof],
        [AC_MSG_ERROR([hpcprof not found])])

    # XML2 support (headers and lib)
    #
    PKG_CHECK_MODULES(XML2, libxml-2.0 >= 2.4)
    CPPFLAGS+=" $XML2_CFLAGS"
    AC_CHECK_LIB([xml2], [xmlParseFile], [],
        [AC_MSG_ERROR([libxml2.so not found])])
    AC_CHECK_HEADER([libxml/parser.h], [],
        [AC_MSG_ERROR([parser.h not found])])

    # PAPI support (headers and lib)
    #
    AC_CHECK_LIB([papi], [PAPI_library_init, PAPI_add_event], [],
        [AC_MSG_ERROR([papi.so not found])])
    AC_CHECK_HEADER([papi.h], [], [AC_MSG_ERROR([papi.h not found])])

    # SQLite support (headers, lib, and program)
    #
    AC_PATH_TOOL([SQLITE3_PROGRAM], [sqlite3],
        [AC_MSG_ERROR([not found: sqlite3])])
    AC_CHECK_LIB([sqlite3], [sqlite3_open, sqlite3_close, sqlite3_exec], [],
        [AC_MSG_ERROR([not found: libsqlit3.so])])
    AC_CHECK_HEADER([sqlite3.h], [], [AC_MSG_ERROR([not found: sqlite3.h])])

    ENABLE_HPCTOOLKIT=hpctoolkit
    ENABLED_MODULES+="hpctoolkit "
    AC_CONFIG_FILES([modules/hpctoolkit/Makefile])
], [
    AC_MSG_RESULT([no])
    ENABLE_HPCTOOLKIT=
])
AC_SUBST([ENABLE_HPCTOOLKIT])

#------------------------------------------------------------------------------
# timb module requirements
#
AC_ARG_ENABLE([module-timb],
    [AS_HELP_STRING([--disable-module-timb],
        [disable TIMB (thread imbalance) analysis module])],
    [ENABLE_TIMB=$enableval], [ENABLE_TIMB=yes])

AC_MSG_CHECKING([whether to enable TIMB (thread imbalance) analysis module])
AS_IF([test x"$ENABLE_TIMB" = xyes], [
    AC_MSG_RESULT([yes])

    # Math library
    #
    AC_CHECK_LIB([m], [rint], [], [AC_MSG_ERROR([libm.so not found])])

    # Requires HPCToolkit module
    #
    AS_IF([test x"$ENABLE_HPCTOOLKIT" = x],
        [AC_MSG_ERROR([TIMB module requires HPTCoolkit module])], [])

    ENABLE_TIMB=timb
    ENABLED_MODULES+="timb "
    AC_CONFIG_FILES([modules/timb/Makefile])
], [
    AC_MSG_RESULT([no])
    ENABLE_TIMB=
])
AC_SUBST([ENABLE_TIMB])

#------------------------------------------------------------------------------
# LCPI module requirements
#
AC_ARG_ENABLE([module-lcpi],
    [AS_HELP_STRING([--disable-module-lcpi], [disable LCPI analysis module])],
    [ENABLE_LCPI=$enableval], [ENABLE_LCPI=yes])

AC_MSG_CHECKING([whether to enable LCPI analysis module])
AS_IF([test x"$ENABLE_LCPI" = xyes], [
    AC_MSG_RESULT([yes])

    # Requires HPCToolkit module
    #
    AS_IF([test x"$ENABLE_HPCTOOLKIT" = x],
        [AC_MSG_ERROR([LCPI module requires HPTCoolkit module])], [])

    # Math library
    #
    AC_CHECK_LIB([m], [isnormal, isnan, isinf], [],
        [AC_MSG_ERROR([libm.so not found])])

    # GNU Math Evaluator support (headers and lib)
    #
    AC_CHECK_LIB([matheval], [evaluator_create], [],
        [AC_MSG_ERROR([libmatheval.so not found])])
    AC_CHECK_HEADER([matheval.h], [], [AC_MSG_ERROR([matheval.h not found])])

    # SQLite support (headers, lib, and program)
    #
    AC_PATH_TOOL([SQLITE3_PROGRAM], [sqlite3],
        [AC_MSG_ERROR([not found: sqlite3])])
    AC_CHECK_LIB([sqlite3], [sqlite3_open, sqlite3_close, sqlite3_exec], [],
        [AC_MSG_ERROR([not found: libsqlit3.so])])
    AC_CHECK_HEADER([sqlite3.h], [], [AC_MSG_ERROR([not found: sqlite3.h])])

    ENABLE_LCPI=lcpi
    ENABLED_MODULES+="lcpi "
    AC_CONFIG_FILES([modules/lcpi/Makefile])
], [
    AC_MSG_RESULT([no])
    ENABLE_LCPI=
])
AC_SUBST([ENABLE_LCPI])

#------------------------------------------------------------------------------
# 'perfexpert' requirements
#
AC_ARG_ENABLE([perfexpert],
    [AS_HELP_STRING([--disable-perfexpert], [disable PerfExpert tool])],
    [ENABLE_PERFEXPERT=$enableval], [ENABLE_PERFEXPERT=yes])

AC_MSG_CHECKING([whether to enable PerfExpert])
AS_IF([test x"$ENABLE_PERFEXPERT" = xyes], [
    AC_MSG_RESULT([yes])

    AC_MSG_CHECKING([for enabled modules])
    AS_IF([test x"$ENABLED_MODULES" = x], [
        AC_MSG_RESULT([no])
        AC_MSG_ERROR([PerfExpert requires at least one module enabled])
    ], [
        AC_MSG_RESULT($ENABLED_MODULES: OK!)
    ])

    # SQLite support (headers, lib, and program)
    #
    AC_PATH_TOOL([SQLITE3_PROGRAM], [sqlite3],
        [AC_MSG_ERROR([not found: sqlite3])])
    AC_CHECK_LIB([sqlite3], [sqlite3_open, sqlite3_close, sqlite3_exec], [],
        [AC_MSG_ERROR([not found: libsqlit3.so])])
    AC_CHECK_HEADER([sqlite3.h], [], [AC_MSG_ERROR([not found: sqlite3.h])])

    ENABLE_PERFEXPERT=perfexpert
    ENABLED_TOOLS+="perfexpert "
    AC_CONFIG_FILES([tools/perfexpert/Makefile])
    AC_CONFIG_FILES([tools/perfexpert/perfexpert_setup_db.sh],
        [chmod +x tools/perfexpert/perfexpert_setup_db.sh])
], [
    AC_MSG_RESULT([no])
    ENABLE_PERFEXPERT=
])
AC_SUBST([ENABLE_PERFEXPERT])

#------------------------------------------------------------------------------
# 'hound' requirements
#
AC_ARG_ENABLE([hound],
    [AS_HELP_STRING([--disable-hound], [disable the performance benchmarks])],
    [ENABLE_HOUND=$enableval], [ENABLE_HOUND=yes])

AC_MSG_CHECKING([whether to enable the performance benchmarks])
AS_IF([test x"$ENABLE_HOUND" = xyes], [
    AC_MSG_RESULT([yes])

    ENABLE_HOUND=hound
    ENABLED_TOOLS+="hound "
    AC_CONFIG_FILES([tools/hound/Makefile])
], [
    AC_MSG_RESULT([no])
    ENABLE_HOUND=
])
AC_SUBST([ENABLE_HOUND])

#------------------------------------------------------------------------------
# 'perfexpert_analyzer' requirements
#
#AC_ARG_ENABLE([analyzer],
#    [AS_HELP_STRING([--disable-analyzer],
#    [disable the standalone performance analyzer tool])],
#    [ENABLE_ANALYZER=$enableval], [ENABLE_ANALYZER=yes])
#
#AC_MSG_CHECKING([whether to enable the performance analyzer tool])
#AS_IF([test x"$ENABLE_ANALYZER" = xyes], [
#    AC_MSG_RESULT([yes])
#
#    ENABLE_ANALYZER=analyzer
#    ENABLED_TOOLS+="analyzer "
#    AC_CONFIG_FILES([tools/analyzer/Makefile])
#], [
#    AC_MSG_RESULT([no])
#    ENABLE_ANALYZER=
#])
#AC_SUBST([ENABLE_ANALYZER])

#------------------------------------------------------------------------------
# 'recommender' requirements
#
AC_ARG_ENABLE([recommender], [AS_HELP_STRING([--disable-recommender],
        [disable optimization recommender tool])],
    [ENABLE_RECOMMENDER=$enableval], [ENABLE_RECOMMENDER=yes])

AC_MSG_CHECKING([whether to enable the optimization recommender tool])
AS_IF([test x"$ENABLE_RECOMMENDER" = xyes], [
    AC_MSG_RESULT([yes])

    # Math library
    #
    AC_CHECK_LIB([m], [isnormal, isnan, isinf], [],
        [AC_MSG_ERROR([libm.so not found])])

    # SQLite support (headers, lib, and program)
    #
    AC_PATH_TOOL([SQLITE3_PROGRAM], [sqlite3],
        [AC_MSG_ERROR([not found: sqlite3])])
    AC_CHECK_LIB([sqlite3], [sqlite3_open, sqlite3_close, sqlite3_exec], [],
        [AC_MSG_ERROR([not found: libsqlit3.so])])
    AC_CHECK_HEADER([sqlite3.h], [], [AC_MSG_ERROR([not found: sqlite3.h])])

    ENABLE_RECOMMENDER=recommender
    ENABLED_TOOLS+="recommender "
    AC_CONFIG_FILES([tools/recommender/Makefile])
], [
    AC_MSG_RESULT([no])
    ENABLE_RECOMMENDER=
])
AC_SUBST([ENABLE_RECOMMENDER])

#------------------------------------------------------------------------------
# 'macpo' requirements
#
AC_ARG_ENABLE([macpo], [AS_HELP_STRING([--disable-macpo], [disable MACPO tool
and module (data-centric instrumentation, measurement, and analysis)])],
    [ENABLE_MACPO=$enableval], [ENABLE_MACPO=yes])

AC_ARG_WITH([jvm],
    [AS_HELP_STRING([--with-jvm], [set JVM library path])],
    [LDFLAGS+=" -L$withval"])

AC_LANG_PUSH([C++])

AC_MSG_CHECKING([whether to enable MACPO tool and MACPO module])
AS_IF([test x"$ENABLE_MACPO" = xyes], [
    AC_MSG_RESULT([yes])

    # JVM support
    #
    AC_CHECK_LIB([jvm], [JVM_Open, JVM_Close], [],
        [AC_MSG_ERROR([not found: libjvm.so (Java Virtual Machine)])])

    # ROSE Compiler support (headers and lib)
    #
    AC_CHECK_HEADER([rose.h], [], [AC_MSG_ERROR([not found: rose.h])],
        [#include "rose.h"])

    # GNU Multiple Precision Arithmetic Library support (headers and lib)
    #
    AC_CHECK_LIB([gmp], [__gmpf_init, __gmpf_clear], [],
        [AC_MSG_ERROR([not found: libgmp.so])])
    AC_CHECK_HEADER([gmp.h], [], [AC_MSG_ERROR([not found: gmp.h])])

    # Check for Google SparseHash (headers)
    #
    AC_CHECK_HEADER([google/sparse_hash_map], [],
        [AC_MSG_ERROR([not found: google/sparse_hash_map])])

    ENABLE_MACPO=macpo
    ENABLED_TOOLS+="macpo "
    ENABLED_MODULES+="macpo "
    AC_CONFIG_FILES([tools/macpo/Makefile])
    AC_CONFIG_FILES([tools/macpo/macpo.sh], [chmod +x tools/macpo/macpo.sh])
    AC_CONFIG_FILES([tools/macpo/libmrt/Makefile])
    AC_CONFIG_FILES([tools/macpo/analyze/Makefile])
    AC_CONFIG_FILES([modules/macpo/Makefile])
], [
    AC_MSG_RESULT([no])
    ENABLE_MACPO=
])
AC_SUBST([ENABLE_MACPO])
AC_LANG_POP

#------------------------------------------------------------------------------
# 'perfexpert_ct' requirements
#
AC_ARG_ENABLE([ct], [AS_HELP_STRING([--disable-ct],
        [disable the automatic code transformer tool])],
    [ENABLE_CT=$enableval], [ENABLE_CT=yes])

AC_MSG_CHECKING([whether to enable code transformation])
AS_IF([test x"$ENABLE_CT" = xyes], [
    AC_MSG_RESULT([yes])

    # JVM support
    #
    AC_CHECK_LIB([jvm], [JVM_Open, JVM_Close], [],
        [AC_MSG_ERROR([not found: libjvm.so (Java Virtual Machine)])])

    # ROSE Compiler support (headers and lib)
    #
    AC_CHECK_HEADER([rose.h], [], [AC_MSG_ERROR([not found: rose.h])],
        [#include "rose.h"])

    # Check for Lex/Flex and Yacc/Bison
    AC_PROG_LEX
    AC_PROG_YACC

    ENABLE_CT=ct
    ENABLED_TOOLS+="perfexpert_ct "
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
AC_MSG_NOTICE([Enabled tools: $ENABLED_TOOLS])
AC_MSG_NOTICE([Enabled modules: $ENABLED_MODULES])

# EOF
