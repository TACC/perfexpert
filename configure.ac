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
# Author: Leonardo Fialho
#         Ashay Rane
#
# $HEADER$
#------------------------------------------------------------------------------

# Requires autoconf 2.67 at least
AC_PREREQ(2.63)

# Define the package name, version, author, etc.
AC_INIT([perfexpert], [0.1], [Leonardo Fialho: fialho@utexas.edu],
    [perfexpert.tar.gz], [http://www.tacc.com.br/perfexpert/])
AC_COPYRIGHT([University of Texas at Austin])
AC_REVISION([$Revision: 0.1 $])

# Used by autotools
AC_CONFIG_AUX_DIR(config)

# Generate global config.h and install_dirs.h files
AC_CONFIG_SRCDIR([tools/common/config.h.in])
AC_CONFIG_HEADERS([tools/common/config.h])
AC_CONFIG_FILES([tools/common/install_dirs.h])

# Initialize Automake
AM_INIT_AUTOMAKE([foreign -Wall -Werror])

# Ok Automake 1.12, shut up!
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

# Just to have a cleaner output during 'make'
AM_SILENT_RULES([yes])

# We need some work here to save hound from -O2 on CFLAGS. This overwrites any
# user-defined CFLAGS :-/
CFLAGS="-g"
CXXFLAGS="-O3 -g"

# Requires only a standard C compiler
AM_PROG_CC_C_O
AC_PROG_CXX
AC_PROG_MKDIR_P
AC_PROG_LN_S
AC_PROG_INSTALL

# Minst (MACPO) requires Libtool
AC_CONFIG_MACRO_DIR([m4])
LT_INIT

# Checks for typedefs, structures, and compiler characteristics
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions
AC_FUNC_MALLOC

# Output files
AC_CONFIG_FILES([Makefile tools/Makefile])

#------------------------------------------------------------------------------
# Check for the operation system to include specific defines (if needed!)
#
AC_CANONICAL_HOST
AC_MSG_CHECKING([for the OS])
AS_CASE([${host}],
    [*-*-darwin*], [
        AC_DEFINE([OS_MACOS], [1], [The host OS is OSX])
        AC_MSG_RESULT([you are a happy Mac user, I suppose...])
        HOST_OS=darwin],
    [*-*-freebsd*], [
        AC_DEFINE([OS_FREEBSD], [1], [The host OS is FreeBSD])
        AC_MSG_RESULT([hum... Are you happy with whis FreeBSD?])
        HOST_OS=freebsd],
    [*-*-linux*], [
        AC_DEFINE([OS_LINUX], [1], [The host OS is Linux])
        AC_MSG_RESULT([one more geek using Linux])
        HOST_OS=linux],
    [*-*-netbsd*], [
        AC_DEFINE([OS_NETBSD], [1], [The host OS is NetBSD])
        AC_MSG_RESULT([really? There is at least one NetBSD user in the world!])
        HOST_OS=netbsd],
    [*-*-solaris2*], [
        AC_DEFINE([OS_SOLARIS], [1], [The host OS is Solaris])
        AC_MSG_RESULT([I understand, you are stucked on Solaris, right?])
        HOST_OS=solaris],
    [*-*-mingw* | *-*-cygwin*], [
        AC_DEFINE([OS_WINDOWS], [1], [The host OS is Windows])
        AC_MSG_RESULT([I will pray for you, Windows user :-/])
        HOST_OS=cygwin],
    [AC_MSG_RESULT([Unsupported operating system: ${host}])
     HOST_OS=unknown
])

#------------------------------------------------------------------------------
# Any architecture-specific requirement?
#
AC_MSG_CHECKING([for the architecture])
AS_CASE([${host}],
    [i?86-* | k?-* | athlon-* | pentium*-], [
        AC_DEFINE([ARCH_X86], [1], [Architecture is x86])
        AC_MSG_RESULT([x86])],
    [x86_64-*], [
        AC_DEFINE([ARCH_X86_64], [1], [Architecture is x86-64])
        AC_MSG_RESULT([x86-64])],
    [ppc-* | powerpc-*], [
        AC_DEFINE([ARCH_PPC], [1], [Architecture is PowerPC])
        AC_MSG_RESULT([PowerPC])],
    [AC_MSG_ERROR([Unknown Architecture])
])

#------------------------------------------------------------------------------
# Debug
#
AC_ARG_ENABLE([debug],
    [AS_HELP_STRING([--enable-debug],
        [Enable/Disable debugging [default=disabled]])],
    [ENABLE_DEBUG=$enableval], [ENABLE_DEBUG=no])

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
# Checks for required (but generic) header files
#
AC_CHECK_HEADER([stdio.h],     [], [AC_MSG_ERROR([not found: stdio.h])])
AC_CHECK_HEADER([getopt.h],    [], [AC_MSG_ERROR([not found: getopt.h])])
AC_CHECK_HEADER([ctype.h],     [], [AC_MSG_ERROR([not found: ctype.h])])
AC_CHECK_HEADER([errno.h],     [], [AC_MSG_ERROR([not found: errno.h])])
AC_CHECK_HEADER([fcntl.h],     [], [AC_MSG_ERROR([not found: fcntl.h])])
AC_CHECK_HEADER([sys/wait.h],  [], [AC_MSG_ERROR([not found: sys/wait.h])])

#------------------------------------------------------------------------------
# Checks for SQLite headers, lib and program
#
AC_CHECK_LIB([sqlite3], [sqlite3_open, sqlite3_close, sqlite3_exec,
    sqlite3_free, sqlite3_errmsg], [],
    [AC_MSG_ERROR([SQLite3 library not found])])

AC_CHECK_HEADER([sqlite3.h], [
    WITH_SQLITE3_SUPPORT=yes
    AC_DEFINE([HAVE_SQLITE3], [1], [SQLite3 support])
], [
    WITH_SQLITE3_SUPPORT=no
    AC_DEFINE([HAVE_SQLITE3], [0], [SQLite3 support])
])

AC_PATH_TOOL([SQLITE3_PROGRAM], [sqlite3], [no])

#------------------------------------------------------------------------------
# Checks for PIPS, optionally, using an argument (required for opttran_ct)
#
AC_ARG_WITH([pips],
    [AS_HELP_STRING([--with-pips],
        [Use this argument to set where PIPS binries are installed (not
        necessary if it is in the PATH). TPIPS (one of the PIPS binaries) is
        necessary to compile OptTran:CT])], [
    PIPS_PATH="$withval"
    AC_PATH_TOOL([TPIPS_PROGRAM], [tpips], [no], [$PIPS_PATH/bin])
    AS_IF([test x"$TPIPS_PROGRAM" = x"no"],
        [AC_MSG_ERROR([TPIPS not found in $PIPS_PATH])])], [
    AC_PATH_TOOL([TPIPS_PROGRAM], [tpips], [no])])

#------------------------------------------------------------------------------
# Find Apache ANT, optionally, using an argument (required for Analyzer)
#
AC_ARG_WITH([apache-ant],
    [AS_HELP_STRING([--with-apache-ant],
        [Use this argument to set where Apache Ant program is installed (not
        necessary if it is in the PATH). Ant is necessary to compile
        Analyzer])], [
    ANT_PATH="$withval"
    AC_PATH_TOOL([ANT_PROGRAM], [ant], [no], [$ANT_PATH])
    AS_IF([test x"$ANT_PROGRAM" = x"no"],
        [AC_MSG_ERROR([Ant not found in $ANT_PATH])])], [
        AC_PATH_TOOL([ANT_PROGRAM], [ant], [no])])

#------------------------------------------------------------------------------
# Most scripts require Bash shell
#
AC_PATH_TOOL([BASH_PROGRAM], [bash], [no])
AS_IF([test x"$BASH_PROGRAM" == x"no"],
    [AC_MSG_ERROR([Most of our scripts require the Bash shell.])], [])

#------------------------------------------------------------------------------
# Check for PAPI, optionally, using an argument (required for Analyzer)
#
AC_ARG_WITH([papi],
    [AS_HELP_STRING([--with-papi],
        [Use this argument to set PAPI prefix path (where PAPI library
        and include directories are located), if you do that there is no
        need to set neither '--with-papi-lib' nor '--with-papi-include'.
        This argument is not necessary if PAPI is installed into default
        directories ('/usr' and '/usr/local')])], [
    CPPFLAGS+=" -I$withval/include"
    LDFLAGS+=" -L$withval/lib"], [])

AC_ARG_WITH([papi-lib],
    [AS_HELP_STRING([--with-papi-lib],
        [use this argument to set PAPI library path (it overwrites
        '--with-papi' argument)])], [LDFLAGS+=" -L$withval"], [])

AC_ARG_WITH([papi-include],
    [AS_HELP_STRING([--with-papi-include],
        [use this argument to set PAPI include path (it overwrites
        '--with-papi' argument)])], [CPPFLAGS+=" -I$withval"], [])

# Checks for PAPI library and header file
AC_CHECK_LIB([papi], [PAPI_library_init, PAPI_add_event,
    PAPI_create_eventset, PAPI_get_event_info, PAPI_event_name_to_code,
    PAPI_query_event], [], [HAVE_PAPI_LIB=no])
AS_IF([test x"$HAVE_PAPI_LIB" == x"no"],
    [AC_MSG_ERROR([libpapi not found])], [])

AC_CHECK_HEADER([papi.h], [HAVE_PAPI_HEADER=yes], [HAVE_PAPI_HEADER=no])
AS_IF([test x"$HAVE_PAPI_HEADER" == x"no"],
    [AC_MSG_ERROR([papi.h not found])], [])

AC_DEFINE([HAVE_PAPI], [1], [PAPI support])

#------------------------------------------------------------------------------
# Find HPCToolkit programs, optionally, using an argument
#
AC_ARG_WITH([hpctoolkit],
    [AS_HELP_STRING([--with-hpctoolkit],
        [Use this argument to set where HPCTollkit binaries are
        installed (not necessary if they are in the PATH)])], [
    HPCTOOLKIT_PATH="$withval"
    AC_PATH_TOOL([HPCSTRUCT_PROGRAM], [hpcstruct], [no], [$HPCTOOLKIT_PATH/bin])
    AC_PATH_TOOL([HPCRUN_PROGRAM],    [hpcrun],    [no], [$HPCTOOLKIT_PATH/bin])
    AC_PATH_TOOL([HPCPROF_PROGRAM],   [hpcprof],   [no], [$HPCTOOLKIT_PATH/bin])
    AS_IF([test x"$HPCSTRUCT_PROGRAM" = x"no"],
        [AC_MSG_ERROR([hpcstruct not found in $HPCTOOLKIT_PATH/bin])])
    AS_IF([test x"$HPCRUN_PROGRAM" = x"no"],
        [AC_MSG_ERROR([hpcrun not found in $HPCTOOLKIT_PATH/bin])])
    AS_IF([test x"$HPCPROF_PROGRAM" = x"no"],
        [AC_MSG_ERROR([hpcprof not found in $HPCTOOLKIT_PATH/bin])])
], [
    AC_PATH_TOOL([HPCSTRUCT_PROGRAM], [hpcstruct], [no])
    AC_PATH_TOOL([HPCRUN_PROGRAM],    [hpcrun],    [no])
    AC_PATH_TOOL([HPCPROF_PROGRAM],   [hpcprof],   [no])
])

#------------------------------------------------------------------------------
# Support to JVM (optional), using an argument
#
AC_ARG_WITH([jvm],
    [AS_HELP_STRING([--with-jvm], [JVM is required when Rose is selected,
        use this argument to set JVM library path])], [
    WITH_JVM_LIBS=$withval
    LDFLAGS+=" -L$WITH_JVM_LIBS"

    # A random number of C symbols of JVM library
    AC_CHECK_LIB([jvm], [JVM_InitializeCompiler, JVM_Open, JVM_Close], [],
        [AC_MSG_ERROR([Java Virtual Machine (JVM) library not found!])])
    WITH_JVM=yes], [WITH_JVM=no]
)

#------------------------------------------------------------------------------
# Support to Rose Compiler (optional), using an argument
#
AC_ARG_WITH([rose],
    [AS_HELP_STRING([--with-rose],
        [support to Rose Compiler @<:@default=no@:>@ which enables some
        advanced features on the Optimization Recommender tool, as well is
        required to enable OptTran:CT (Code Transformer tool), and
        OptTran:CR (Code Re-Assembler tool). Use this argument to set Rose
        prefix path (where Rose libraries and include directories are
        located), if you do that there is no need to set neither
        '--with-rose-libs' nor '--with-rose-include'])], [
    WITH_ROSE_LIBS=$withval/lib
    WITH_ROSE_INCLUDE=$withval/include
    WITH_ROSE=yes
], [
    WITH_ROSE=no
])

AC_ARG_WITH([rose-libs],
    [AS_HELP_STRING([--with-rose-libs],
        [support to Rose Compiler @<:@default=no@:>@, use this argument to
        set Rose library path (it overwrites '--with-rose' argument)])], [
    WITH_ROSE_LIBS=$withval
    WITH_ROSE=yes], [])

AC_ARG_WITH([rose-include],
    [AS_HELP_STRING([--with-rose-include],
        [support to Rose Compiler @<:@default=no@:>@, use this argument to
        set Rose include path (it overwrites '--with-rose' argument)])], [
    WITH_ROSE_INCLUDE=$withval
    WITH_ROSE=yes], [])

AS_IF([test x"$WITH_ROSE" == x"yes"], [
    # Rose requires JVM, so...
    AS_IF([test x"$WITH_JVM" == x"no"], [
        AC_MSG_ERROR([Rose requires a JVM, use '--with-jvm' to define it])], [])

    AS_IF([test x"$WITH_ROSE_LIBS" = x""], [
        AC_MSG_ERROR([You chose to enable Rose support but Rose library path
            was not defined. Use '--with-rose-libs=path' or
            '--with-rose=prefix' to define it])], [])

    AS_IF([test x"$WITH_ROSE_INCLUDE" = x""], [
        AC_MSG_ERROR([You chose to enable Rose support but Rose include path
            was not defined. Use '--with-rose-include=path' or
            '--with-rose=prefix' to define it])], [])

    CPPFLAGS+=" -I$WITH_ROSE_INCLUDE"
    LDFLAGS+=" -L$WITH_ROSE_LIBS"

    # A random number of C symbols of Rose library
    AC_CHECK_LIB([rose], [node_has_side_effects, add_to_scopes_list,
        copy_node, node_complete_object_type, set_node_operator],
        [], [AC_MSG_ERROR([Rose library not found!])])

    # How the hell I can make the rose.h header check work?
    #AC_CHECK_HEADER([rose.h], [], [AC_MSG_ERROR([not found: rose.h])])

    WITH_ROSE_SUPPORT=yes
    AC_DEFINE([HAVE_ROSE], [1], [Enable Rose support])
], [
    WITH_ROSE_SUPPORT=no
    AC_DEFINE([HAVE_ROSE], [0], [Enable Rose support])
])

AM_CONDITIONAL([WITH_ROSE_SUPPORT], [test x"$WITH_ROSE_SUPPORT" = x"yes"])

#------------------------------------------------------------------------------
# Enable/Disable 'analyzer'
#
AC_ARG_ENABLE([analyzer],
    [AS_HELP_STRING([--disable-analyzer],
        [Enable/Disable the application profiler tool [default=enabled]])],
    [ENABLE_OPTTRAN_ANALYZER=$enableval], [ENABLE_OPTTRAN_ANALYZER=yes])

AC_MSG_CHECKING([whether to enable Analyzer])
AS_IF([test x"$ENABLE_OPTTRAN_ANALYZER" = x"yes"], [
    AC_MSG_RESULT([yes])

    # Apache ANT is required
    AS_IF([test x"$ANT_PROGRAM" = x"no"],
        [AC_MSG_ERROR([Ant not found, try using '--with-apache-ant'])])

    # Currently, hound and sniffer only works on Linux.
    AS_IF([test x"$HOST_OS" == x"linux"], [

        # Headers required exclusively for hound
        AC_CHECK_HEADER([sched.h],    [], [AC_MSG_ERROR([not found: sched.h])])
        AC_CHECK_HEADER([time.h],     [], [AC_MSG_ERROR([not found: time.h])])
        AC_CHECK_HEADER([sys/mman.h], [], [AC_MSG_ERROR([not found: sys/mman.h])])

        # PAPI is required
        AS_IF([test x"HAVE_PAPI_LIB" == x"no"],
            [AC_MSG_ERROR([PAPI not found, try using '--with-papi'])], [])
        AS_IF([test x"HAVE_PAPI_HEADER" == x"no"],
            [AC_MSG_ERROR([PAPI not found, try using '--with-papi'])], [])

        # HPCToolkit is required
        AS_IF([test x"$HPCSTRUCT_PROGRAM" = x"no"],
            [AC_MSG_ERROR([hpcstruct not found, try using '--with-hpctoolkit'])])
        AS_IF([test x"$HPCRUN_PROGRAM" = x"no"],
            [AC_MSG_ERROR([hpcrun not found, try using '--with-hpctoolkit'])])
        AS_IF([test x"$HPCPROF_PROGRAM" = x"no"],
            [AC_MSG_ERROR([hpcprof not found, try using '--with-hpctoolkit'])])

        # Generate hound and sniffer Makefiles and also analyzer_run_exp.sh
        ENABLE_HOUND=hound
        ENABLE_SNIFFER=sniffer
        AC_CONFIG_FILES([tools/analyzer/hound/Makefile])
        AC_CONFIG_FILES([tools/analyzer/sniffer/Makefile])
        AC_CONFIG_FILES([tools/analyzer/sniffer/analyzer_run_exp.sh_template],
            [chmod +x tools/analyzer/sniffer/analyzer_run_exp.sh_template])
    ], [
        # Disable hound and sniffer
        ENABLE_HOUND=
        ENABLE_SNIFFER=
    ])

    # Generate Makefiles for Analyzer
    ENABLE_ANALYZER=analyzer
    AC_CONFIG_FILES([tools/analyzer/Makefile])
    AC_CONFIG_FILES([tools/analyzer/hpcdata.sh],
        [chmod +x tools/analyzer/hpcdata.sh])
    AC_CONFIG_FILES([tools/analyzer/analyzer.sh],
        [chmod +x tools/analyzer/analyzer.sh])
    AC_CONFIG_FILES([tools/analyzer/argp/Makefile])
    AC_CONFIG_FILES([tools/analyzer/argp/build.xml])
    AC_CONFIG_FILES([tools/analyzer/hpcdata/Makefile])
    AC_CONFIG_FILES([tools/analyzer/hpcdata/build.xml])
    AC_CONFIG_FILES([tools/analyzer/analyzer/Makefile])
    AC_CONFIG_FILES([tools/analyzer/analyzer/build.xml])
], [
    AC_MSG_RESULT([no])
    ENABLE_ANALYZER=
])
AC_SUBST([ENABLE_HOUND])
AC_SUBST([ENABLE_SNIFFER])
AC_SUBST([ENABLE_ANALYZER])

#------------------------------------------------------------------------------
# Enable/Disable 'minst' (MACPO)
#
AC_ARG_ENABLE([minst],
    [AS_HELP_STRING([--disable-minst],
        [Enable/Disable the data access instrumentation tool [default=enabled]])],
    [ENABLE_OPTTRAN_MINST=$enableval], [ENABLE_OPTTRAN_MINST=yes])

AC_MSG_CHECKING([whether to enable Minst (MACPO)])
AS_IF([test x"$ENABLE_OPTTRAN_MINST" = x"yes"], [
    AC_MSG_RESULT([yes])

    # ROSE is required
    AS_IF([test x"$WITH_ROSE_SUPPORT" = x"no"],
        [AC_MSG_ERROR([ROSE not found, try using '--with-rose'])], [])

    ENABLE_MINST=minst
    AC_CONFIG_FILES([tools/minst/Makefile])
    AC_CONFIG_FILES([tools/minst/libmrt/Makefile])
], [
    AC_MSG_RESULT([no])
    ENABLE_MINST=
])
AC_SUBST([ENABLE_MINST])

#------------------------------------------------------------------------------
# Enable/Disable 'recommender'
#
AC_ARG_ENABLE([recommender],
    [AS_HELP_STRING([--disable-recommender],
        [Enable/Disable the optimization recommender tool [default=enabled]])],
    [ENABLE_OPTTRAN_RECOMMENDER=$enableval], [ENABLE_OPTTRAN_RECOMMENDER=yes])

AC_MSG_CHECKING([whether to enable the Optimization Recommender])
AS_IF([test x"$ENABLE_OPTTRAN_RECOMMENDER" = x"yes"], [
    AC_MSG_RESULT([yes])

    # SQLite is required
    AS_IF([test x"$WITH_SQLITE3_SUPPORT" = x"no"],
        [AC_MSG_ERROR([Please install SQLite3])], [])

    ENABLE_RECOMMENDER=recommender
    AC_CONFIG_FILES([tools/recommender/Makefile])
], [
    AC_MSG_RESULT([no])
    ENABLE_RECOMMENDER=
])
AC_SUBST([ENABLE_RECOMMENDER])

#------------------------------------------------------------------------------
# Enable/Disable 'opttran_pr'
#
# It looks like there is a bug while checking for LEX which makes the LEX check
# result appears before the result message (yes/no).
#
AC_ARG_ENABLE([opttran_pr],
    [AS_HELP_STRING([--disable-opttran_pr],
        [Enable/Disable the pattern recognizer tool [default=enabled]])],
    [ENABLE_OPTTRAN_PR=$enableval], [ENABLE_OPTTRAN_PR=yes])

AC_MSG_CHECKING([whether to enable OptTran Pattern Recognizer])
AS_IF([test x"$ENABLE_OPTTRAN_PR" = x"yes"], [
    AC_MSG_RESULT([yes])

    # Check for Lex/Flex and Yacc/Bison
    AC_PROG_LEX
    AC_PROG_YACC

    ENABLE_PR=pr
    AC_CONFIG_FILES([tools/pr/Makefile])
    AC_CONFIG_FILES([tools/pr/c_loop1/Makefile])
    AC_CONFIG_FILES([tools/pr/c_loop2/Makefile])
], [
    AC_MSG_RESULT([no])
    ENABLE_PR=
])
AC_SUBST([ENABLE_PR])

#------------------------------------------------------------------------------
# Enable/Disable 'opttran_ct'
#
AC_MSG_CHECKING([whether to enable OptTran Code Transformer])
AC_ARG_ENABLE([opttran_ct],
    [AS_HELP_STRING([--disable-opttran_ct],
        [Enable/Disable the code transformer tool [default=enabled]])],
    [ENABLE_OPTTRAN_CT=$enableval], [ENABLE_OPTTRAN_CT=yes])

AS_IF([test x"$ENABLE_OPTTRAN_CT" = x"yes"], [
    AC_MSG_RESULT([yes])

    # PIPS is required
    AS_IF([test x"$TPIPS_PROGRAM" = x"no"],
        [AC_MSG_ERROR([TPIPS not found, try using '--with-pips'])], [])

    AC_CONFIG_FILES([tools/ct/c_loop2/Makefile])
    AC_CONFIG_FILES([tools/ct/c_loop2/ct_c_loop2.sh],
        [chmod +x tools/ct/c_loop2/ct_c_loop2.sh])

    ENABLE_CT=ct
    AC_CONFIG_FILES([tools/ct/Makefile])
], [
    AC_MSG_RESULT([no])
    ENABLE_CT=
])
AC_SUBST([ENABLE_CT])

#------------------------------------------------------------------------------
# Enable/Disable 'opttran_ci'
#
AC_MSG_CHECKING([whether to enable OptTran Code Integrator])
AC_ARG_ENABLE([opttran_ci],
    [AS_HELP_STRING([--disable-opttran_ci],
        [Enable/Disable the code integrator tool [default=enabled]])],
    [ENABLE_OPTTRAN_CI=$enableval], [ENABLE_OPTTRAN_CI=yes])

AS_IF([test x"$ENABLE_OPTTRAN_CI" = x"yes"], [
    AC_MSG_RESULT(yes)

    # ROSE is required
    AS_IF([test x"$WITH_ROSE_SUPPORT" = x"no"],
        [AC_MSG_ERROR([ROSE not found, try using '--with-rose'])], [])

    ENABLE_CI=ci
    AC_CONFIG_FILES([tools/ci/Makefile])
], [
    AC_MSG_RESULT(no)
    ENABLE_CI=
])
AC_SUBST([ENABLE_CI])

#------------------------------------------------------------------------------
# Do we have all the tools? So generate the ninja-super-power-script opttran.sh
#
#AS_IF([test x"$ENABLE_OPTTRAN_ANALYZER" = x"yes"], [
#    AS_IF([test x"$ENABLE_OPTTRAN_RECOMMENDER" = x"yes"], [
#        AS_IF([test x"$ENABLE_OPTTRAN_PR" = x"yes"], [
#            AS_IF([test x"$ENABLE_OPTTRAN_CT" = x"yes"], [
#                AS_IF([test x"$ENABLE_OPTTRAN_CR" = x"yes"], [
AC_CONFIG_FILES([tools/opttran.sh], [chmod +x tools/opttran.sh])
#                    WITH_OPTTRAN_SUPPORT=yes
#                ], [
#                    WITH_OPTTRAN_SUPPORT=no
#                ])
#            ])
#        ])
#    ])
#])
#AC_SUBST([WITH_OPTTRAN_SUPPORT])

#------------------------------------------------------------------------------
# Generate output files
#
AC_OUTPUT
AC_MSG_NOTICE([========================================================])
AC_MSG_NOTICE([ Analyzer:                  $ENABLE_OPTTRAN_ANALYZER    ])
AC_MSG_NOTICE([ Minst (MACPO):             $ENABLE_OPTTRAN_MINST       ])
AC_MSG_NOTICE([ Optimizations Recommender: $ENABLE_OPTTRAN_RECOMMENDER ])
AC_MSG_NOTICE([ Code Pattern Recognizer:   $ENABLE_OPTTRAN_PR          ])
AC_MSG_NOTICE([ Code Transformer:          $ENABLE_OPTTRAN_CT          ])
AC_MSG_NOTICE([ Code Integrator:           $ENABLE_OPTTRAN_CI          ])
AC_MSG_NOTICE([                                                        ])
AC_MSG_NOTICE([ Debug mode: $ENABLE_DEBUG                              ])
AC_MSG_NOTICE([ PAPI:       $HAVE_PAPI_HEADER                          ])
AC_MSG_NOTICE([ HPCToolkit: $HPCRUN_PROGRAM, $HPCSTRUCT_PROGRAM, $HPCPROF_PROGRAM])
AC_MSG_NOTICE([ SQLite3:    $WITH_SQLITE3_SUPPORT                      ])
AC_MSG_NOTICE([ Rose:       $WITH_ROSE                                 ])
AC_MSG_NOTICE([ Java VM:    $WITH_JVM                                  ])
AC_MSG_NOTICE([ PIPS:       $TPIPS_PROGRAM                             ])
AC_MSG_NOTICE([                                                        ])
AC_MSG_NOTICE([ CC=$CC                                                 ])
AC_MSG_NOTICE([ CXX=$CXX                                               ])
AC_MSG_NOTICE([ CPPFLAGS=$CPPFLAGS                                     ])
AC_MSG_NOTICE([ CFLAGS=$CFLAGS                                         ])
AC_MSG_NOTICE([ CXXFLAGS=$CXXFLAGS                                     ])
AC_MSG_NOTICE([ LDFLAGS=$LDFLAGS                                       ])
AC_MSG_NOTICE([ LIBS=$LIBS                                             ])
AS_IF([test x"$HOST_OS" != x"linux"], [
    AS_IF([test x"$ENABLE_OPTTRAN_ANALYZER" = x"yes"], [
        AC_MSG_NOTICE([========================================================])
        AC_MSG_NOTICE([                    !!! WARNING !!!                     ])
        AC_MSG_NOTICE([                                                        ])
        AC_MSG_NOTICE([ Since you are not running Linux, Analyzer is unable to ])
        AC_MSG_NOTICE([ characterize you machine (using hound tool) neither to ])
        AC_MSG_NOTICE([ generate LCPI configuration file (sniffer tool). Thus, ])
        AC_MSG_NOTICE([ 'analyzer_run' script will not be available to you.    ])
        AC_MSG_NOTICE([ However, if you want to run Analyzer on this           ])
        AC_MSG_NOTICE([ installation just to analyze an experiment file you're ])
        AC_MSG_NOTICE([ planning to run on another host you just have to copy  ])
        AC_MSG_NOTICE([ the lcpi.properties and machine.properties files from  ])
        AC_MSG_NOTICE([ the host's \$prefix/etc where you are planning to run  ])
        AC_MSG_NOTICE([ the program you want to analyze/optimize to the        ])
        AC_MSG_NOTICE([ $prefix/etc directory on this machine.                 ])
    ], [])
], [])
AC_MSG_NOTICE([========================================================])
AC_MSG_NOTICE([ Great news! Now you just have to run "make -s install" ])
AC_MSG_NOTICE([========================================================])
AC_MSG_NOTICE([])

# EOF
