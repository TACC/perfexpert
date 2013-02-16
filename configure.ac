#
# Copyright (c) 2013  University of Texas at Austin. All rights reserved.
#
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# This file is part of OptTran and PerfExpert.
#
# OptTran as well PerfExpert are free software: you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation, either version 3 of the License,
# or (at your option) any later version.
#
# OptTran and PerfExpert are distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
# General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with OptTran or PerfExpert. If not, see <http://www.gnu.org/licenses/>.
#
# Author: Leonardo Fialho
#
# $HEADER$
#------------------------------------------------------------------------------

# Requires autoconf 2.67 at least
AC_PREREQ(2.63)

# Define the package name, version, author, etc.
AC_INIT([opttran], [0.1], [Leonardo Fialho: fialho@utexas.edu],
    [opttran.tar.gz], [http://www.tacc.com.br/opttran/])
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

# We need some work here to save hound from -O2 on CFLAGS. This overwrites any
# user-defined CFLAGS :-/
CFLAGS=-g

# Requires only a standard C compiler
AC_PROG_CC
AC_PROG_CXX
AC_PROG_MKDIR_P
AC_PROG_LN_S
AC_PROG_INSTALL
AM_PROG_CC_C_O

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
        AC_DEFINE([OS_MACOS], 1, [The host OS is OSX])
        AC_MSG_RESULT([you are a happy Mac user, I suppose...])
        HOST_OS=darwin],
    [*-*-freebsd*], [
        AC_DEFINE([OS_FREEBSD], 1, [The host OS is FreeBSD])
        AC_MSG_RESULT([hum... Are you happy with whis FreeBSD?])
        HOST_OS=freebsd],
    [*-*-linux*], [
        AC_DEFINE([OS_LINUX], 1, [The host OS is Linux])
        AC_MSG_RESULT([one more geek using Linux])
        HOST_OS=linux],
    [*-*-netbsd*], [
        AC_DEFINE([OS_NETBSD], 1, [The host OS is NetBSD])
        AC_MSG_RESULT([really? There is at least one NetBSD user in the world!])
        HOST_OS=netbsd],
    [*-*-solaris2*], [
        AC_DEFINE([OS_SOLARIS], 1, [The host OS is Solaris])
        AC_MSG_RESULT([I understand, you are stucked on Solaris, right?])
        HOST_OS=solaris],
    [*-*-mingw* | *-*-cygwin*], [
        AC_DEFINE([OS_WINDOWS], 1, [The host OS is Windows])
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
        AC_DEFINE(ARCH_X86, 1, [Architecture is x86])
        AC_MSG_RESULT([x86])],
    [x86_64-*], [
        AC_DEFINE(ARCH_X86_64, 1, [Architecture is x86-64])
        AC_MSG_RESULT([x86-64])],
    [ppc-* | powerpc-*], [
        AC_DEFINE(ARCH_PPC, 1, [Architecture is PowerPC])
        AC_MSG_RESULT([PowerPC])],
    [AC_MSG_ERROR([Unknown Architecture])
])

#------------------------------------------------------------------------------
# Debug
#
AC_MSG_CHECKING([whether to enable debug])
AC_ARG_ENABLE(debug,
    AS_HELP_STRING([--enable-debug],
               [Enable/Disable debugging @<:@default=disabled@:>@]]),
    ENABLE_DEBUG=$enableval, ENABLE_DEBUG=no)

AM_CONDITIONAL(ENABLE_DEBUG, test x"$ENABLE_DEBUG" = x"yes")

AS_IF([test x"$ENABLE_DEBUG" = x"yes"], [
    AC_MSG_RESULT(yes)
    AC_DEFINE(ENABLE_DEBUG, [1], [Enable/Disable debug])
    ENABLE_DEBUG=true
], [
    AC_MSG_RESULT(no)
    AC_DEFINE(ENABLE_DEBUG, [0], [Enable/Disable debug])
    ENABLE_DEBUG=false
])

#------------------------------------------------------------------------------
# Checks for required (but generic) header files
#
AC_CHECK_HEADER([stdio.h],     [], [AC_MSG_ERROR([not found: stdio.h])])
AC_CHECK_HEADER([getopt.h],    [], [AC_MSG_ERROR([not found: getopt.h])])
AC_CHECK_HEADER([ctype.h],     [], [AC_MSG_ERROR([not found: ctype.h])])
AC_CHECK_HEADER([stdlib.h],    [], [AC_MSG_ERROR([not found: stdlib.h])])
AC_CHECK_HEADER([stdarg.h],    [], [AC_MSG_ERROR([not found: stdarg.h])])
AC_CHECK_HEADER([string.h],    [], [AC_MSG_ERROR([not found: string.h])])
AC_CHECK_HEADER([unistd.h],    [], [AC_MSG_ERROR([not found: unistd.h])])
AC_CHECK_HEADER([sys/stat.h],  [], [AC_MSG_ERROR([not found: sys/stat.h])])
AC_CHECK_HEADER([sys/types.h], [], [AC_MSG_ERROR([not found: sys/types.h])])
AC_CHECK_HEADER([errno.h],     [], [AC_MSG_ERROR([not found: errno.h])])

#------------------------------------------------------------------------------
# Enable/Disable 'perfexpert' (BEGIN)
#
AC_MSG_CHECKING([whether to enable PerfExpert])
AC_ARG_ENABLE(perfexpert,
    [AS_HELP_STRING([--disable-perfexpert],
        [Enable/Disable the application profiler tool @<:@default=enabled@:>@])],
    ENABLE_PERFEXPERT=$enableval, ENABLE_PERFEXPERT=yes)

AS_IF([test x"$ENABLE_PERFEXPERT" = x"yes"], [
    AC_MSG_RESULT(yes)
    AC_DEFINE(OPTTRAN_ENABLE_PERFEXPERT, [1], [Enable/Disable perfexpert])

    # First of all, we need Apache Ant
    AC_CHECK_PROG(ANT_PROGRAM, ant, yes)
    AS_IF([test x"$ANT_PROGRAM" != x"yes"],
        [AC_MSG_ERROR([Please install Apache Ant.])],
        [AC_PATH_TOOL(ANT_PROGRAM, ant)])

    # hpcdata, perfexpert and perfexert_run script requires Bash shell
    AC_CHECK_PROG(BASH_PROGRAM, bash, yes)
    AS_IF([test x"$BASH_PROGRAM" != x"yes"],
        [AC_MSG_ERROR([Some of PerfExpert scripts require Bash shell.])],
        [AC_PATH_TOOL(BASH_PROGRAM, bash)])

    # Currently, hound and sniffer only works on Linux.
    AS_IF([test x"$HOST_OS" == x"linux"], [

        # Headers required exclusivelly for hound
        AC_CHECK_HEADER([inttypes.h], [], [AC_MSG_ERROR([not found: inttypes.h])])
        AC_CHECK_HEADER([sched.h],    [], [AC_MSG_ERROR([not found: sched.h])])
        AC_CHECK_HEADER([time.h],     [], [AC_MSG_ERROR([not found: time.h])])
        AC_CHECK_HEADER([sys/mman.h], [], [AC_MSG_ERROR([not found: sys/mman.h])])

        # Headers and libraries required for sniffer
        AC_SEARCH_LIBS([PAPI_library_init, PAPI_add_event, PAPI_create_eventset,
            PAPI_get_event_info, PAPI_event_name_to_code, PAPI_query_event],
            [papi],, [AC_MSG_ERROR([PAPI library not found!])])
        AC_CHECK_HEADER([papi.h], [], [AC_MSG_ERROR([not found: papi.h])])

        # Some tools from HPCToolKit are required to perfexpert_run_exp.sh
        AC_CHECK_PROG(HPCSTRUCT_PROGRAM, hpcstruct, yes)
        AS_IF([test x"$HPCSTRUCT_PROGRAM" != x"yes"],
            [AC_MSG_ERROR([It seems like you don't have HPCToolkit installed.])],
            [AC_PATH_TOOL(HPCSTRUCT_PROGRAM, hpcstruct)])

        AC_CHECK_PROG(HPCRUN_PROGRAM, hpcrun, yes)
        AS_IF([test x"$HPCRUN_PROGRAM" != x"yes"],
            [AC_MSG_ERROR([It seems like you don't have HPCToolkit installed.])],
            [AC_PATH_TOOL(HPCRUN_PROGRAM, hpcrun)])

        AC_CHECK_PROG(HPCPROF_PROGRAM, hpcprof, yes)
        AS_IF([test x"$HPCPROF_PROGRAM" != x"yes"],
            [AC_MSG_ERROR([It seems like you don't have HPCToolkit installed.])],
            [AC_PATH_TOOL(HPCPROF_PROGRAM, hpcprof)])

        # Generate hound and sniffer Makefiles and also perfexpert_run_exp.sh
        ENABLE_HOUND=hound
        ENABLE_SNIFFER=sniffer
        AC_CONFIG_FILES([tools/perfexpert/hound/Makefile])
        AC_CONFIG_FILES([tools/perfexpert/sniffer/Makefile])
        AC_CONFIG_FILES([tools/perfexpert/sniffer/perfexpert_run_exp.sh_template],
            [chmod +x tools/perfexpert/sniffer/perfexpert_run_exp.sh_template])
        AC_SUBST([ENABLE_HOUND])
        AC_SUBST([ENABLE_SNIFFER])
    ], [
        # Disable hound and sniffer
        ENABLE_HOUND=
        ENABLE_SNIFFER=
        AC_SUBST([ENABLE_HOUND])
        AC_SUBST([ENABLE_SNIFFER])
    ])

    # Generate Makefiles for perfexpert
    ENABLE_PERFEXPERT=perfexpert
    AC_CONFIG_FILES([tools/perfexpert/Makefile])
    AC_CONFIG_FILES([tools/perfexpert/hpcdata.sh],
        [chmod +x tools/perfexpert/hpcdata.sh])
    AC_CONFIG_FILES([tools/perfexpert/perfexpert.sh],
        [chmod +x tools/perfexpert/perfexpert.sh])
    AC_CONFIG_FILES([tools/perfexpert/argp/Makefile])
    AC_CONFIG_FILES([tools/perfexpert/argp/build.xml])
    AC_CONFIG_FILES([tools/perfexpert/hpcdata/Makefile])
    AC_CONFIG_FILES([tools/perfexpert/hpcdata/build.xml])
    AC_CONFIG_FILES([tools/perfexpert/perfexpert/Makefile])
    AC_CONFIG_FILES([tools/perfexpert/perfexpert/build.xml])
    AC_SUBST([ENABLE_PERFEXPERT])
], [
    AC_MSG_RESULT(no)
    AC_DEFINE(OPTTRAN_ENABLE_PERFEXPERT, [0], [Enable/Disable perfexpert])
    ENABLE_PERFEXPERT=
    AC_SUBST([ENABLE_PERFEXPERT])
])
#
# Enable/Disable 'perfexpert' (END)
#------------------------------------------------------------------------------
# Enable/Disable 'recommender' (BEGIN)
#
AC_MSG_CHECKING([whether to enable the Optimization Recommender])
AM_CONDITIONAL([WITH_ROSE_SUPPORT], [test x"$WITH_ROSE_SUPPORT" = x"yes"])
AC_ARG_ENABLE([recommender],
    [AS_HELP_STRING([--disable-recommender],
        [Enable/Disable the optimization recommender tool @<:@default=enabled@:>@])],
    [ENABLE_RECOMMENDER=$enableval], [ENABLE_RECOMMENDER=yes])

AS_IF([test x"$ENABLE_RECOMMENDER" = x"yes"], [
    AC_MSG_RESULT(yes)

    # Checks for SQLite headers, lib and program (only if 'recommender' is enabled)
    AC_SEARCH_LIBS([sqlite3_open, sqlite3_close, sqlite3_exec, sqlite3_free,
        sqlite3_errmsg], [sqlite3],, [AC_MSG_ERROR([SQLite3 library not found])])
    AC_CHECK_HEADER([sqlite3.h], [], [AC_MSG_ERROR([not found: sqlite3.h])])

    # Check for the SQLite program
    AC_CHECK_PROG(SQLITE3_PROGRAM, sqlite3, yes)
    AS_IF([test x"$SQLITE3_PROGRAM" = x"yes"],
        [AC_PATH_TOOL(SQLITE3_PROGRAM, sqlite3)],
        [AC_MSG_ERROR([Please install SQLite3 program])])

    # Support to Rose Compiler (optional)
    AC_MSG_CHECKING([whether to enable the Rose support on Recommender])
    AC_ARG_WITH([rose],
        [AS_HELP_STRING([--with-rose],
            [support to Rose Compiler @<:@default=no@:>@ which enables some
            advanced features on the Optimization Recommender tool, as well is
            required to enable OptTran:CT (Code Transformer tool), and
            OptTran:CR (Code Re-Assembler tool). Use this argument to set Rose's
            prefix path (where Rose's libraries and include directories are
            located), if you do that there is no need to set neither
            '--with-rose-libs' nor '--with-rose-include'])], [
        WITH_ROSE=$withval
        WITH_ROSE_LIBS=$WITH_ROSE/lib
        WITH_ROSE_INCLUDE=$WITH_ROSE/include], [WITH_ROSE=no])

    AC_ARG_WITH([rose-libs],
        [AS_HELP_STRING([--with-rose-libs],
            [support to Rose Compiler @<:@default=no@:>@, use this argument to
             set Rose's library path (it overwrites '--with-rose argument)])], [
        WITH_ROSE_LIBS=$withval
        WITH_ROSE=yes], [])

    AC_ARG_WITH([rose-include],
        [AS_HELP_STRING([--with-rose-include],
            [support to Rose Compiler @<:@default=no@:>@, use this argument to
             set Rose's include path (it overwrites '--with-rose argument)])], [
        WITH_ROSE_INCLUDE=$withval
        WITH_ROSE=yes], [])

    # Both WITH_ROSE_LIBS and WITH_ROSE_INCLUDE have to be defined
    AS_IF([test x"$WITH_ROSE_LIBS" = x""], [WITH_ROSE=no], [])
    AS_IF([test x"$WITH_ROSE_INCLUDE" = x""], [WITH_ROSE=no], [])

    # Rose requires JVM, so we have to check for JVM
    AC_ARG_WITH([jvm],
        [AS_HELP_STRING([--with-jvm], [JVM is required when Rose is selected,
            use this argument to set JVM library path])],
        [WITH_JVM=$withval], [WITH_JVM=no])

    AS_IF([test x"$WITH_ROSE" = x"no"], [
        AC_MSG_RESULT(no)

        AS_IF([test x"$WITH_ROSE_LIBS" = x""], [
            AC_MSG_WARN([You chose to enable Rose support but Rose library path
                was not defined. Use '--with-rose-libs=path' or
                '--with-rose=prefix' to define it])], [])

        AS_IF([test x"$WITH_ROSE_INCLUDE" = x""], [
            AC_MSG_WARN([You chose to enable Rose support but Rose include path
                was not defined. Use '--with-rose-include=path' or
                '--with-rose=prefix' to define it])], [])

        AC_MSG_WARN([Rose support disabled])
        AC_MSG_WARN([- OptTran automatic source optimization will be disabled])
        AC_MSG_WARN([- Recommender tool will disable fragments extraction])
    ], [
        AC_MSG_RESULT(yes)
        AS_IF([test x"$WITH_JVM" = x"no"], [
            AC_MSG_WARN([JVM not selected, so Rose support will be disabled])
            AC_MSG_WARN([- OptTran automatic source optimization will be disabled])
            AC_MSG_WARN([- Recommender will not be able to extract code fragments])
            WITH_ROSE=no
        ], [
            WITH_ROSE_SUPPORT=yes
            # TODO: check for Rose libraries
            AC_MSG_RESULT([checking for Rose libraries at $WITH_ROSE_LIBS])


            # TODO: check for Rose header files
            AC_MSG_RESULT([checking for Rose include files at $WITH_ROSE_INCLUDE])


            AC_SUBST([WITH_ROSE_LIBS])
            AC_SUBST([WITH_ROSE_INCLUDE])
            # TODO: check for JVM libraries
            AC_MSG_RESULT([checking for JVM libraries at $WITH_JVM])


            AC_SUBST([WITH_JVM_LIBS])
        ])
        WITH_ROSE_SUPPORT=no
    ])
    AC_DEFINE(OPTTRAN_ENABLE_RECOMMENDER, [1], [Enable/Disable recommender])
    ENABLE_RECOMMENDER=recommender
    AC_CONFIG_FILES([tools/recommender/Makefile])
    AC_SUBST([ENABLE_RECOMMENDER])
], [
    AC_MSG_RESULT(no)
    AC_DEFINE(OPTTRAN_ENABLE_RECOMMENDER, [0], [Enable/Disable recommender])
    ENABLE_RECOMMENDER=
    AC_SUBST([ENABLE_RECOMMENDER])
])
#
# Enable/Disable 'recommender' (END)
#------------------------------------------------------------------------------
# Enable/Disable 'opttran_pr' (BEGIN)
#
# It looks like there is a bug while checking for LEX which makes the LEX check
# result appears before the result message (yes/no).
#
AC_MSG_CHECKING([whether to enable OptTran Pattern Recognizer])
AC_ARG_ENABLE(opttran_pr,
    [AS_HELP_STRING([--disable-opttran_pr],
        [Enable/Disable the pattern recognizer tool @<:@default=enabled@:>@])],
    ENABLE_OPTTRAN_PR=$enableval, ENABLE_OPTTRAN_PR=yes)

AS_IF([test x"$ENABLE_OPTTRAN_PR" = x"yes"], [
    AC_MSG_RESULT(yes)
    AC_DEFINE(OPTTRAN_ENABLE_PR, [1], [Enable/Disable opttran_pr])

# Check for Lex/Flex and Yacc/Bison
    AC_PROG_LEX
    AC_PROG_YACC
    
    ENABLE_PR=pr
    AC_CONFIG_FILES([tools/pr/Makefile])
    AC_CONFIG_FILES([tools/pr/c_loop1/Makefile])
    AC_CONFIG_FILES([tools/pr/c_loop2/Makefile])
    AC_SUBST([ENABLE_PR])
], [
    AC_MSG_RESULT(no)
    AC_DEFINE(OPTTRAN_ENABLE_PR, [0], [Enable/Disable opttran_pr])
    ENABLE_PR=
    AC_SUBST([ENABLE_PR])
])
#
# Enable/Disable 'opttran_pr' (END)
#------------------------------------------------------------------------------
# Enable/Disable 'opttran_ct' (BEGIN)
#
AC_MSG_CHECKING([whether to enable OptTran Code Transformer])
AC_ARG_ENABLE(opttran_ct,
    [AS_HELP_STRING([--disable-opttran_ct],
        [Enable/Disable the code transformer tool @<:@default=disabled@:>@])],
    [ENABLE_OPTTRAN_CT=$enableval], [ENABLE_OPTTRAN_CT=no])

AS_IF([test x"$ENABLE_OPTTRAN_CT" = x"yes"], [
    AC_MSG_RESULT(yes)
    AC_DEFINE(OPTTRAN_ENABLE_CT, [1], [Enable/Disable opttran_ct])

    ENABLE_CT=ct
    AC_CONFIG_FILES([tools/ct/Makefile])
    AC_SUBST([ENABLE_CT])
], [
    AC_MSG_RESULT(no)
    AC_DEFINE(OPTTRAN_ENABLE_CT, [0], [Enable/Disable opttran_ct])
    ENABLE_CT=
    AC_SUBST([ENABLE_CT])
])
#
# Enable/Disable 'opttran_ct' (END)
#------------------------------------------------------------------------------
# Enable/Disable 'opttran_cr' (BEGIN)
#
AC_MSG_CHECKING([whether to enable OptTran Code Re-Assembler])
AC_ARG_ENABLE(opttran_cr,
    [AS_HELP_STRING([--disable-opttran_cr],
        [Enable/Disable the code re-assembler tool @<:@default=disabled@:>@])],
    [ENABLE_OPTTRAN_CR=$enableval], [ENABLE_OPTTRAN_CR=no])

AS_IF([test x"$ENABLE_OPTTRAN_CR" = x"yes"], [
    AC_MSG_RESULT(yes)
    AC_DEFINE(OPTTRAN_ENABLE_CR, [1], [Enable/Disable opttran_cr])

    ENABLE_CR=cr
    AC_CONFIG_FILES([tools/cr/Makefile])
    AC_SUBST([ENABLE_CR])
], [
    AC_MSG_RESULT(no)
    AC_DEFINE(OPTTRAN_ENABLE_CR, [0], [Enable/Disable opttran_cr])
    ENABLE_CR=
    AC_SUBST([ENABLE_CR])
])
#
# Enable/Disable 'opttran_cr' (END)
#------------------------------------------------------------------------------
# Generate output files
#
AC_OUTPUT

AC_MSG_NOTICE([])
if test x"$HOST_OS" != x"linux"; then
    AC_MSG_NOTICE([=========================================================])
    AC_MSG_NOTICE([                     !!! WARNING !!!                     ])
    AC_MSG_NOTICE([                                                         ])
    AC_MSG_NOTICE([  Since you are not running Linux, Perfexpert is unable  ])
    AC_MSG_NOTICE([  to characterize you machine (using hound tool) neither ])
    AC_MSG_NOTICE([  to generate LCPI configuration file (sniffer tool).    ])
    AC_MSG_NOTICE([  Thus, 'perfexpert_run' script will not be available    ])
    AC_MSG_NOTICE([  to you. However, if you want to run PerfExpert on this ])
    AC_MSG_NOTICE([  installation just to analyze an experiment file you're ])
    AC_MSG_NOTICE([  planning to run on another host you just have to copy  ])
    AC_MSG_NOTICE([  the lcpi.properties and machine.properties files from  ])
    AC_MSG_NOTICE([  the host's \$prefix/etc where you are planning to run  ])
    AC_MSG_NOTICE([  the program you want to analyze/optimize to the        ])
    AC_MSG_NOTICE([  $prefix/etc directory on this machine.                 ])
fi
AC_MSG_NOTICE([=========================================================])
AC_MSG_NOTICE([   Great news! Now you just have to run "make install"   ])
AC_MSG_NOTICE([=========================================================])
AC_MSG_NOTICE([])

# EOF
