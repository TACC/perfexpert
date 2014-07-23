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
AC_CONFIG_FILES([Makefile common/Makefile common/install_dirs.h
    modules/Makefile tools/Makefile])
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
AS_IF([test x"$BASH_PROGRAM" = x], [AC_MSG_ERROR([Shell is not Bash])])

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

AC_ARG_WITH([libdwarf],
    [AS_HELP_STRING([--with-libdwarf], [set DWARF library and include path])],
    [CPPFLAGS+=" -I$withval/include"
    LDFLAGS+=" -L$withval/lib"])

AC_ARG_WITH([libdwarf-lib],
    [AS_HELP_STRING([--with-libdwarf-lib], [set DWARF library path])],
    [LDFLAGS+=" -L$withval"])

AC_ARG_WITH([libdwarf-include],
    [AS_HELP_STRING([--with-libdwarf-include], [set DWARF include path])],
    [LDFLAGS+=" -L$withval"])

AC_ARG_WITH([libelf],
    [AS_HELP_STRING([--with-libelf], [set ELF library and include path])],
    [CPPFLAGS+=" -I$withval/include"
    LDFLAGS+=" -L$withval/lib"
    LIBELF_LIB="$withval/lib"])

AC_ARG_WITH([libelf-lib],
    [AS_HELP_STRING([--with-libelf-lib], [set ELF library path])],
    [LDFLAGS+=" -L$withval"
    LIBELF_LIB="$withval"])

AC_ARG_WITH([libelf-include],
    [AS_HELP_STRING([--with-libelf-include], [set ELF include path])],
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

AC_ARG_WITH([hwloc],
    [AS_HELP_STRING([--with-hwloc], [set HWLOC library and include path])], [
    CPPFLAGS+=" -I$withval/include"
    LDFLAGS+=" -L$withval/lib"])

AC_ARG_WITH([hwloc-libs],
    [AS_HELP_STRING([--with-hwloc-libs], [set HWLOC library path])],
    [LDFLAGS+=" -L$withval"])

AC_ARG_WITH([hwloc-include],
    [AS_HELP_STRING([--with-hwloc-include], [set HWLOC include path])],
    [CPPFLAGS+=" -I$withval/include"])

AC_ARG_WITH([jvm],
    [AS_HELP_STRING([--with-jvm], [set JVM library path])],
    [LDFLAGS+=" -L$withval"])

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

AC_ARG_WITH([gsl],
    [AS_HELP_STRING([--with-gsl], [set GNU Scientific library path])], [
    CPPFLAGS+=" -I$withval/include"
    LDFLAGS+=" -L$withval/lib"])

#------------------------------------------------------------------------------
# 'perfexpert' requirements
#
AC_ARG_ENABLE([perfexpert],
    [AS_HELP_STRING([--disable-perfexpert],
    [disable PerfExpert workflow tool])],
    [ENABLE_PERFEXPERT=$enableval], [ENABLE_PERFEXPERT=yes])

AC_MSG_CHECKING([whether to enable PerfExpert])
AS_IF([test x"$ENABLE_PERFEXPERT" = xyes], [
    AC_MSG_RESULT([yes])

    # SQLite support (headers, lib, and program)
    #
    AC_PATH_TOOL([SQLITE3_PROGRAM], [sqlite3])
    AS_IF([test x"$SQLITE3_PROGRAM" = x], [
        AC_MSG_ERROR([sqlite3 not found. Do you have SQLite3 installed?])
    ])
    AC_CHECK_LIB([sqlite3], [sqlite3_open, sqlite3_close, sqlite3_exec], [],
        [AC_MSG_ERROR([not found: libsqlite3.so])])
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
    [AS_HELP_STRING([--disable-hound], [disable performance benchmarks])],
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
# 'memsniffer' requirements
#
AC_ARG_ENABLE([memsniffer],
    [AS_HELP_STRING([--disable-memsniffer], [disable memory sniffer])],
    [ENABLE_MEMSNIFFER=$enableval], [ENABLE_MEMSNIFFER=yes])

AC_MSG_CHECKING([whether to enable the memory sniffer])
AS_IF([test x"$ENABLE_MEMSNIFFER" = xyes], [
    AC_MSG_RESULT([yes])

    # HWLOC support (headers and lib)
    #
    AC_CHECK_LIB([hwloc], [hwloc_topology_init, hwloc_topology_load], [],
        [AC_MSG_ERROR([not found: libhwloc.so])])
    AC_CHECK_HEADER([hwloc.h], [], [AC_MSG_ERROR([not found: hcloc.h])])

    ENABLE_MEMSNIFFER=memsniffer
    ENABLED_TOOLS+="memsniffer "
    AC_CONFIG_FILES([tools/memsniffer/Makefile])
], [
    AC_MSG_RESULT([no])
    ENABLE_MEMSNIFFER=
])
AC_SUBST([ENABLE_MEMSNIFFER])

#------------------------------------------------------------------------------
# 'recommender' requirements
#
#AC_ARG_ENABLE([recommender], [AS_HELP_STRING([--disable-recommender],
#        [disable optimization recommender tool])],
#    [ENABLE_RECOMMENDER=$enableval], [ENABLE_RECOMMENDER=yes])
#
#AC_MSG_CHECKING([whether to enable optimization recommender tool])
#AS_IF([test x"$ENABLE_RECOMMENDER" = xyes], [
#    AC_MSG_RESULT([yes])
#
#    # Math library
#    #
#    AC_CHECK_LIB([m], [isnormal, isnan, isinf], [],
#        [AC_MSG_ERROR([libm.so not found])])
#
#    # SQLite support (headers, lib, and program)
#    #
#    AC_PATH_TOOL([SQLITE3_PROGRAM], [sqlite3])
#    AS_IF([test x"$SQLITE3_PROGRAM" = x], [
#        AC_MSG_ERROR([sqlite3 not found. Do you have SQLite3 installed?])
#    ])
#    AC_CHECK_LIB([sqlite3], [sqlite3_open, sqlite3_close, sqlite3_exec], [],
#        [AC_MSG_ERROR([not found: libsqlite3.so])])
#    AC_CHECK_HEADER([sqlite3.h], [], [AC_MSG_ERROR([not found: sqlite3.h])])
#
#    ENABLE_RECOMMENDER=recommender
#    ENABLED_TOOLS+="recommender "
#    AC_CONFIG_FILES([tools/recommender/Makefile])
#], [
#    AC_MSG_RESULT([no])
#    ENABLE_RECOMMENDER=
#])
#AC_SUBST([ENABLE_RECOMMENDER])

#------------------------------------------------------------------------------
# 'perfexpert_ct' requirements
#
AC_ARG_ENABLE([ct], [AS_HELP_STRING([--disable-ct],
        [disable automatic code transformer tool])],
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
    AC_LANG_PUSH([C++])
    AC_CHECK_LIB([rose], [validate_operands_and_clobbers], [],
        [AC_MSG_ERROR([not found: librose.so])])
    AC_CHECK_HEADER([rose.h], [], [AC_MSG_ERROR([not found: rose.h])],
        [#include "rose.h"])
    AC_LANG_POP

    # Check for Lex/Flex and Yacc/Bison
    AC_PROG_YACC
    AC_PROG_LEX

    ENABLE_CT=ct
    ENABLED_TOOLS+="perfexpert_ct "
    AC_DEFINE([HAVE_CODE_TRANSFORMATION], [1], [Enable code transformation])
    AC_CONFIG_FILES([tools/ct/Makefile])
    AC_CONFIG_FILES([tools/ct/recognizers/Makefile])
    AC_CONFIG_FILES([tools/ct/recognizers/nested_c_loops/Makefile])
    AC_CONFIG_FILES([tools/ct/transformers/Makefile])
    AC_CONFIG_FILES([tools/ct/transformers/c_loop_interchange/Makefile])
    AC_CONFIG_FILES([tools/ct/transformers/c_loop_tiling/Makefile])
], [
    AC_MSG_RESULT([no])
    ENABLE_CT=
])
AC_SUBST([ENABLE_CT])

#------------------------------------------------------------------------------
# 'macpo' requirements
#
AC_ARG_ENABLE([macpo], [AS_HELP_STRING([--disable-macpo], [disable MACPO tool
and module (data-centric instrumentation, measurement, and analysis)])],
    [ENABLE_MACPO=$enableval], [ENABLE_MACPO=yes])

AC_MSG_CHECKING([whether to enable MACPO tool and MACPO module])
AS_IF([test x"$ENABLE_MACPO" = xyes], [
    AC_MSG_RESULT([yes])

    # JVM support
    #
    AC_CHECK_LIB([jvm], [JVM_Open, JVM_Close], [],
        [AC_MSG_ERROR([not found: libjvm.so (Java Virtual Machine)])])

    # ROSE Compiler support (headers and lib)
    #
    AC_LANG_PUSH([C++])
    AC_CHECK_LIB([rose], [validate_operands_and_clobbers], [],
        [AC_MSG_ERROR([not found: librose.so])])
    AC_CHECK_HEADER([rose.h], [], [AC_MSG_ERROR([not found: rose.h])],
        [#include "rose.h"])
    AC_LANG_POP

    # GNU Multiple Precision Arithmetic Library support (headers and lib)
    #
    AC_CHECK_LIB([gmp], [__gmpf_init, __gmpf_clear], [],
        [AC_MSG_ERROR([not found: libgmp.so])])
    AC_CHECK_HEADER([gmp.h], [], [AC_MSG_ERROR([not found: gmp.h])])

    # Check for GNU Scientific library
    #
    AC_CHECK_HEADER([gsl/gsl_histogram.h], [],
        [AC_MSG_ERROR([not found: gsl/gsl_histogram.h])])

    AC_CHECK_LIB([gslcblas], [cblas_caxpy], [],
        [AC_MSG_ERROR([not found: libgslcblas.so])])

    AC_CHECK_LIB([gsl], [gsl_histogram_alloc, gsl_histogram_free], [],
        [AC_MSG_ERROR([not found: libgsl.so])], [-lgslcblas])

    # Check for libelf (for resolving function addresses to filenames)
    #
    AC_CHECK_LIB([elf],
        [elf_version, elf_begin, elf_nextscn, elf_getdata, elf_strptr, elf_end],
        [NOOP=noop], [AC_MSG_ERROR([libelf.so not found])], [-lelf])
    AC_CHECK_HEADER([libelf.h], [], [AC_MSG_ERROR([libelf.h not found])])

    AC_SUBST(LIBELF_LIB)

    ENABLE_MACPO=macpo
    ENABLED_TOOLS+="macpo "
    ENABLED_MODULES+="macpo "
    AC_CONFIG_FILES([tools/macpo/Makefile])
    AC_CONFIG_FILES([tools/macpo/macpo.sh], [chmod +x tools/macpo/macpo.sh])
    AC_CONFIG_FILES([tools/macpo/inst/Makefile])
    AC_CONFIG_FILES([tools/macpo/libmrt/Makefile])
    AC_CONFIG_FILES([tools/macpo/analyze/Makefile])
    AC_CONFIG_FILES([tools/macpo/tests/Makefile])
    AC_CONFIG_FILES([tools/macpo/tests/libmrt/Makefile])
    AC_CONFIG_FILES([tools/macpo/libmacpo/Makefile])
    AC_CONFIG_FILES([modules/macpo/Makefile])
], [
    AC_MSG_RESULT([no])
    ENABLE_MACPO=
])
AC_SUBST([ENABLE_MACPO])

#------------------------------------------------------------------------------
# make module requirements
#
AC_ARG_ENABLE([module-make],
    [AS_HELP_STRING([--disable-module-make], [disable Make compile module])],
    [ENABLE_MAKE=$enableval], [ENABLE_MAKE=yes])

AC_MSG_CHECKING([whether to enable make compile module])
AS_IF([test x"$ENABLE_MAKE" = xyes], [
    AC_MSG_RESULT([yes])

    ENABLE_MAKE=make
    ENABLED_MODULES+="make "
    AC_CONFIG_FILES([modules/make/Makefile])
], [
    AC_MSG_RESULT([no])
    ENABLE_MAKE=
])
AC_SUBST([ENABLE_MAKE])

#------------------------------------------------------------------------------
# gcc module requirements
#
AC_ARG_ENABLE([module-gcc],
    [AS_HELP_STRING([--disable-module-gcc], [disable GNU C compiler module])],
    [ENABLE_GCC=$enableval], [ENABLE_GCC=yes])

AC_MSG_CHECKING([whether to enable GNU C compiler module])
AS_IF([test x"$ENABLE_GCC" = xyes], [
    AC_MSG_RESULT([yes])

    # Programs: gcc
    #
    AC_PATH_TOOL([GCC_PROGRAM], [gcc])
    AS_IF([test x"$GCC_PROGRAM" = x], [
        AC_MSG_ERROR([gcc not found. Do you have gcc in the PATH?])
    ])

    ENABLE_GCC=gcc
    ENABLED_MODULES+="gcc "
    AC_CONFIG_FILES([modules/gcc/Makefile])
], [
    AC_MSG_RESULT([no])
    ENABLE_GCC=
])
AC_SUBST([ENABLE_GCC])

#------------------------------------------------------------------------------
# gpp module requirements
#
AC_ARG_ENABLE([module-gpp],
    [AS_HELP_STRING([--disable-module-gpp], [disable GNU C++ compiler module])],
    [ENABLE_GPP=$enableval], [ENABLE_GPP=yes])

AC_MSG_CHECKING([whether to enable GNU C++ compiler module])
AS_IF([test x"$ENABLE_GPP" = xyes], [
    AC_MSG_RESULT([yes])

    # Programs: g++
    #
    AC_PATH_TOOL([GPP_PROGRAM], [g++])
    AS_IF([test x"$GPP_PROGRAM" = x], [
        AC_MSG_ERROR([g++ not found. Do you have g++ in the PATH?])
    ])

    ENABLE_GPP=gpp
    ENABLED_MODULES+="g++ "
    AC_CONFIG_FILES([modules/gpp/Makefile])
], [
    AC_MSG_RESULT([no])
    ENABLE_GPP=
])
AC_SUBST([ENABLE_GPP])

#------------------------------------------------------------------------------
# icc module requirements
#
AC_ARG_ENABLE([module-icc],
    [AS_HELP_STRING([--disable-module-icc], [disable Intel C compiler module])],
    [ENABLE_ICC=$enableval], [ENABLE_ICC=yes])

AC_MSG_CHECKING([whether to enable Intel C compiler module])
AS_IF([test x"$ENABLE_ICC" = xyes], [
    AC_MSG_RESULT([yes])

    # Programs: icc
    #
    AC_PATH_TOOL([ICC_PROGRAM], [icc])
    AS_IF([test x"$ICC_PROGRAM" = x], [
        AC_MSG_ERROR([icc not found. Do you have icc in the PATH?])
    ])

    ENABLE_ICC=icc
    ENABLED_MODULES+="icc "
    AC_CONFIG_FILES([modules/icc/Makefile])
], [
    AC_MSG_RESULT([no])
    ENABLE_ICC=
])
AC_SUBST([ENABLE_ICC])

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
    AC_DEFINE([HAVE_HPCTOOLKIT], [1], [Enable HPCToolkit support])

    # Real Time library
    #
    AC_CHECK_LIB([rt], [clock_gettime], [],
        [AC_MSG_ERROR([librt.so not found])])

    # Programs: hpcstruct, hpcrun, hpcprof
    #
    AC_PATH_TOOL([HPCSTRUCT_PROGRAM], [hpcstruct])
    AC_PATH_TOOL([HPCRUN_PROGRAM], [hpcrun])
    AC_PATH_TOOL([HPCPROF_PROGRAM], [hpcprof])
    AS_IF([test x"$HPCSTRUCT_PROGRAM" = x], [
        AC_MSG_ERROR([hpcstruct not found. Do you have HPCToolkit installed?])
    ])
    AS_IF([test x"$HPCRUN_PROGRAM" = x], [
        AC_MSG_ERROR([hpcstruct not found. Do you have HPCToolkit installed?])
    ])
    AS_IF([test x"$HPCPROF_PROGRAM" = x], [
        AC_MSG_ERROR([hpcstruct not found. Do you have HPCToolkit installed?])
    ])

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
        [AC_MSG_ERROR([libpapi.so not found])])
    AC_CHECK_HEADER([papi.h], [], [AC_MSG_ERROR([papi.h not found])])

    # SQLite support (headers, lib, and program)
    #
    AC_PATH_TOOL([SQLITE3_PROGRAM], [sqlite3])
    AS_IF([test x"$SQLITE3_PROGRAM" = x], [
        AC_MSG_ERROR([sqlite3 not found. Do you have SQLite3 installed?])
    ])
    AC_CHECK_LIB([sqlite3], [sqlite3_open, sqlite3_close, sqlite3_exec], [],
        [AC_MSG_ERROR([not found: libsqlite3.so])])
    AC_CHECK_HEADER([sqlite3.h], [], [AC_MSG_ERROR([not found: sqlite3.h])])

    ENABLE_HPCTOOLKIT=hpctoolkit
    ENABLED_MODULES+="hpctoolkit "
    AC_CONFIG_FILES([modules/hpctoolkit/Makefile])
    AC_CONFIG_FILES([modules/hpctoolkit/hpctoolkit_path.h])
], [
    AC_MSG_RESULT([no])
    ENABLE_HPCTOOLKIT=
])
AC_SUBST([ENABLE_HPCTOOLKIT])

AC_ARG_ENABLE([module-hpctoolkit-mic],
    [AS_HELP_STRING([--disable-module-hpctoolkit-mic],
        [disable HPCToolkit support for Intel(R) Xeon Phi chips])],
    [HAVE_HPCTOOLKIT_MIC=$enableval], [HAVE_HPCTOOLKIT_MIC=yes])

AC_MSG_CHECKING([whether to enable HPCToolkit MIC support])
AS_IF([test x"$HAVE_HPCTOOLKIT_MIC" = xyes], [
    AC_MSG_RESULT([yes])
    AC_DEFINE([HAVE_HPCTOOLKIT_MIC], [1], [Enable HPCToolkit MIC support])

    # Programs: MIC version of hpcstruct, hpcrun, hpcprof
    #
    AC_ARG_WITH([mic-hpctoolkit-bins],
        [AS_HELP_STRING([--with-mic-hpctoolkit-bins],
        [set path for MIC version of HPCToolkit binaries])],
        [HPCTOOLKIT_MIC_BIN_PATH="$withval"], [
            AC_MSG_ERROR([You should define a HPCToolkit MIC binaries path])
        ])
    AC_SUBST([HPCTOOLKIT_MIC_BIN_PATH])
], [
    AC_MSG_RESULT([no])
])

#------------------------------------------------------------------------------
# VTune module requirements
#
AC_ARG_ENABLE([module-vtune],
    [AS_HELP_STRING([--disable-module-vtune],
        [disable Intel Vtune measurement module])],
    [ENABLE_VTUNE=$enableval], [ENABLE_VTUNE=yes])

AC_MSG_CHECKING([whether to enable VTune measurement module])
AS_IF([test x"$ENABLE_VTUNE" = xyes], [
    AC_MSG_RESULT([yes])
    AC_DEFINE([HAVE_VTUNE], [1], [Enable VTune support])

    # Programs: amplxe-cl
    #
    AC_PATH_TOOL([VTUNE_PROGRAM], [amplxe-cl])
    AS_IF([test x"$HPCSTRUCT_PROGRAM" = x], [
        AC_MSG_ERROR([amplxe-cl not found. Do you have VTune installed?])
    ])

    # SQLite support (headers, lib, and program)
    #
    AC_PATH_TOOL([SQLITE3_PROGRAM], [sqlite3])
    AS_IF([test x"$SQLITE3_PROGRAM" = x], [
        AC_MSG_ERROR([sqlite3 not found. Do you have SQLite3 installed?])
    ])
    AC_CHECK_LIB([sqlite3], [sqlite3_open, sqlite3_close, sqlite3_exec], [],
        [AC_MSG_ERROR([not found: libsqlite3.so])])
    AC_CHECK_HEADER([sqlite3.h], [], [AC_MSG_ERROR([not found: sqlite3.h])])

    ENABLE_VTUNE=vtune
    ENABLED_MODULES+="vtune "
    AC_CONFIG_FILES([modules/vtune/Makefile])
], [
    AC_MSG_RESULT([no])
    ENABLE_VTUNE=
])
AC_SUBST([ENABLE_VTUNE])

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
    AC_PATH_TOOL([SQLITE3_PROGRAM], [sqlite3])
    AS_IF([test x"$SQLITE3_PROGRAM" = x], [
        AC_MSG_ERROR([sqlite3 not found. Do you have SQLite3 installed?])
    ])
    AC_CHECK_LIB([sqlite3], [sqlite3_open, sqlite3_close, sqlite3_exec], [],
        [AC_MSG_ERROR([not found: libsqlite3.so])])
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
# SQL Rules module requirements
#
AC_ARG_ENABLE([module-sqlrules], [AS_HELP_STRING([--disable-module-sqlrules],
        [disable the SQL Rule-based recommendation module])],
    [ENABLE_SQLRULES=$enableval], [ENABLE_SQLRULES=yes])

AC_MSG_CHECKING([whether to enable SQL Rules module])
AS_IF([test x"$ENABLE_SQLRULES" = xyes], [
    AC_MSG_RESULT([yes])

    # Math library
    #
    AC_CHECK_LIB([m], [isnormal, isnan, isinf], [],
        [AC_MSG_ERROR([libm.so not found])])

    # SQLite support (headers, lib, and program)
    #
    AC_PATH_TOOL([SQLITE3_PROGRAM], [sqlite3])
    AS_IF([test x"$SQLITE3_PROGRAM" = x], [
        AC_MSG_ERROR([sqlite3 not found. Do you have SQLite3 installed?])
    ])
    AC_CHECK_LIB([sqlite3], [sqlite3_open, sqlite3_close, sqlite3_exec], [],
        [AC_MSG_ERROR([not found: libsqlite3.so])])
    AC_CHECK_HEADER([sqlite3.h], [], [AC_MSG_ERROR([not found: sqlite3.h])])

    ENABLE_SQLRULES=sqlrules
    ENABLED_MODULES+="sqlrules "
    AC_CONFIG_FILES([modules/sqlrules/Makefile])
], [
    AC_MSG_RESULT([no])
    ENABLE_SQLRULES=
])
AC_SUBST([ENABLE_SQLRULES])

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
