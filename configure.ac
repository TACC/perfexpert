#
# Copyright (c) 2013  University of Texas at Austin. All rights reserved.
#
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#

# Requires autoconf 2.67 at least
AC_PREREQ(2.63)

# Define the package name, version, author, etc.
AC_INIT([OptTran], [0.1], [Leonardo Fialho: fialho@utexas.edu],
    [opttran.tar.gz], [http://www.tacc.com.br/opttran/])
AC_COPYRIGHT([University of Texas at Austin])
AC_REVISION([$Revision: 0.1 $])

# Used by autotools
AC_CONFIG_AUX_DIR(config)

# Generate a global HEADER file
AC_CONFIG_SRCDIR([config/config.h.in])
AC_CONFIG_HEADERS([config/config.h])

# Initialize Automake
AM_INIT_AUTOMAKE([foreign -Wall -Werror])

# Ok Automake 1.12, shut up!
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

# Requires only a standard C compiler
AC_PROG_CC
AC_PROG_INSTALL

#------------------------------------------------------------------------------
# Check for the operation system to include specific defines (if needed!)
AC_CANONICAL_HOST
AC_MSG_CHECKING([for the OS])
case "${host}" in
    *-*-darwin*)
        AC_DEFINE([OS_MACOS], 1, [The host OS is OSX])
        AC_MSG_RESULT([you are a happy Mac user, I suppose.])
        ;;
    *-*-freebsd*)
        AC_DEFINE([OS_FREEBSD], 1, [The host OS is FreeBSD])
        AC_MSG_RESULT([hum... Are you happy with whis FreeBSD?])
        ;;
    *-*-linux*)
        AC_DEFINE([OS_LINUX], 1, [The host OS is Linux])
        AC_MSG_RESULT([OK, one more geek using Linux.])
        ;;
    *-*-netbsd*)
        AC_DEFINE([OS_NETBSD], 1, [The host OS is NetBSD])
        AC_MSG_RESULT([Really? There is at least one NetBSD user in the world!])
        ;;
    *-*-solaris2*)
        AC_DEFINE([OS_SOLARIS], 1, [The host OS is Solaris])
        AC_MSG_RESULT([I understand, you are stucked on this Solaris, right?.])
        ;;
    *-*-mingw* | *-*-cygwin*)
        AC_DEFINE([OS_WINDOWS], 1, [The host OS is Windows])
        AC_MSG_RESULT([I will pray for you Windows user.])
        ;;
    *)
        AC_MSG_RESULT([Unsupported operating system: ${host}])
        ;;
esac

#------------------------------------------------------------------------------
# Any architecture-specific requirement?
AC_MSG_CHECKING([for the architecture])
case "$host" in
    i?86-* | k?-* | athlon-* | pentium*-)
        AC_DEFINE(ARCH_X86, 1, [Architecture is x86])
        AC_MSG_RESULT([x86])
        ;;
    x86_64-*)
        AC_DEFINE(ARCH_X86_64, 1, [Architecture is x86-64])
        AC_MSG_RESULT([x86-64])
        ;;
    ppc-* | powerpc-*)
        AC_DEFINE(ARCH_PPC, 1, [Architecture is PowerPC])
        AC_MSG_RESULT([PowerPC])
        ;;
    *)
        AC_MSG_ERROR([Unknown Architecture])
        ;;
esac

#------------------------------------------------------------------------------
# Debug
AC_MSG_CHECKING([whether to enable debug])
AC_ARG_ENABLE(debug,
    AS_HELP_STRING([--enable-debug],
               [Enable/Disable debugging, @<:@default=disabled@:>@]]),
    ENABLE_DEBUG=$enableval, ENABLE_DEBUG=no)

if test x"$ENABLE_DEBUG" = x"yes"; then
    AC_MSG_RESULT(yes)
    AC_DEFINE(ENABLE_DEBUG, [1], [Enable/Disable debug])
    ENABLE_DEBUG=true
else
    AC_MSG_RESULT(no)
    AC_DEFINE(ENABLE_DEBUG, [0], [Enable/Disable debug])
    ENABLE_DEBUG=false
fi

#------------------------------------------------------------------------------
# Checks for required (but generic) header files
AC_CHECK_HEADER([stdio.h],  [AC_DEFINE([HAVE_STDIO_H],  1,
    [You have <stdio.h>.])],  [AC_MSG_ERROR([not found: stdio.h])])
AC_CHECK_HEADER([getopt.h], [AC_DEFINE([HAVE_GETOPT_H], 1,
    [You have <getopt.h>.])], [AC_MSG_ERROR([not found: getopt.h])])
AC_CHECK_HEADER([ctype.h],  [AC_DEFINE([HAVE_CTYPE_H],  1,
    [You have <ctype.h>.])],  [AC_MSG_ERROR([not found: ctype.h])])
AC_CHECK_HEADER([stdlib.h], [AC_DEFINE([HAVE_STDLIB_H], 1,
    [You have <stdlib.h>.])], [AC_MSG_ERROR([not found: stdlib.h])])
AC_CHECK_HEADER([stdarg.h], [AC_DEFINE([HAVE_STDARG_H], 1,
    [You have <stdarg.h>.])], [AC_MSG_ERROR([not found: stdarg.h])])
AC_CHECK_HEADER([string.h], [AC_DEFINE([HAVE_STRING_H], 1,
    [You have <string.h>.])], [AC_MSG_ERROR([not found: string.h])])
AC_CHECK_HEADER([unistd.h], [AC_DEFINE([HAVE_UNISTD_H], 1,
    [You have <unistd.h>.])], [AC_MSG_ERROR([not found: unistd.h])])

# Checks for typedefs, structures, and compiler characteristics
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions
AC_FUNC_MALLOC

# Output files
AC_CONFIG_FILES([Makefile
	tools/Makefile])

#------------------------------------------------------------------------------
# Enable/Disable 'perfexpert' (BEGIN)
AC_MSG_CHECKING([whether to enable 'PerfExpert'])
AC_ARG_ENABLE(perfexpert,
    [AS_HELP_STRING([--disable-perfexpert],
        [Enable/Disable the application profiler tool @<:@default=enabled@:>@])],
    ENABLE_PERFEXPERT=$enableval, ENABLE_PERFEXPERT=yes)

if test x"$ENABLE_PERFEXPERT" = x"yes"; then
    AC_MSG_RESULT(yes)
    AC_DEFINE(OPTTRAN_ENABLE_PERFEXPERT, [1], [Enable/Disable perfexpert])

# inttypes.h is required for hound
    AC_CHECK_HEADER([inttypes.h], [AC_DEFINE([HAVE_UNISTD_H], 1,
        [You have <inttypes.h>.])], [AC_MSG_ERROR([not found: inttypes.h])])

    ENABLE_PERFEXPERT=perfexpert
    AC_CONFIG_FILES([tools/perfexpert/Makefile])
    AC_CONFIG_FILES([tools/perfexpert/hound/Makefile])
    AC_SUBST([ENABLE_PERFEXPERT])
else
    AC_MSG_RESULT(no)
    AC_DEFINE(OPTTRAN_ENABLE_PERFEXPERT, [0], [Enable/Disable perfexpert])
    ENABLE_PERFEXPERT=
    AC_SUBST([ENABLE_PERFEXPERT])
fi
# Enable/Disable 'perfexpert' (END)

#------------------------------------------------------------------------------
# Enable/Disable 'recommender' (BEGIN)
AC_MSG_CHECKING([whether to enable the 'Optimization Recommender'])
AC_ARG_ENABLE(recommender,
    [AS_HELP_STRING([--disable-recommender],
        [Enable/Disable the optimization recommender tool @<:@default=enabled@:>@])],
    ENABLE_RECOMMENDER=$enableval, ENABLE_RECOMMENDER=yes)

if test x"$ENABLE_RECOMMENDER" = x"yes"; then
    AC_MSG_RESULT(yes)
    AC_DEFINE(OPTTRAN_ENABLE_RECOMMENDER, [1], [Enable/Disable recommender])

# Checks for SQLite headers, lib and program (only if 'recommender' is enabled)
    AC_SEARCH_LIBS([sqlite3_open, sqlite3_close, sqlite3_exec, sqlite3_free,
        sqlite3_errmsg], [sqlite3],
        [AC_MSG_RESULT([You have SQLite3 (at least the library), great!])],
        [AC_MSG_ERROR([SQLite3 not found ;-(])])
    AC_CHECK_HEADER([sqlite3.h], [AC_DEFINE([HAVE_SQLITE3_H], 1,
        [Awesome! You also have sqlite3.h!])],
        [AC_MSG_ERROR([not found: sqlite3.h])])

# Check for the SQLite utility
    AC_CHECK_PROG(SQLITE3_PROGRAM, sqlite3, yes)
    if test x"$SQLITE3_PROGRAM" != x"yes" ; then
        AC_MSG_ERROR([Please install SQLite3 utility before.])
    fi

    ENABLE_RECOMMENDER=recommender
    AC_CONFIG_FILES([tools/recommender/Makefile])
    AC_SUBST([ENABLE_RECOMMENDER])
else
    AC_MSG_RESULT(no)
    AC_DEFINE(OPTTRAN_ENABLE_RECOMMENDER, [0], [Enable/Disable recommender])
    ENABLE_RECOMMENDER=
    AC_SUBST([ENABLE_RECOMMENDER])
fi
# Enable/Disable 'recommender' (END)

#------------------------------------------------------------------------------
# Enable/Disable 'opttran_pr' (BEGIN)
AC_MSG_CHECKING([whether to enable the 'OptTran Pattern Recognizer'])
AC_ARG_ENABLE(opttran_pr,
    [AS_HELP_STRING([--disable-opttran_pr],
        [Enable/Disable the pattern recognizer tool @<:@default=enabled@:>@])],
    ENABLE_OPTTRAN_PR=$enableval, ENABLE_OPTTRAN_PR=yes)

if test x"$ENABLE_OPTTRAN_PR" = x"yes"; then
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
else
    AC_MSG_RESULT(no)
    AC_DEFINE(OPTTRAN_ENABLE_PR, [0], [Enable/Disable opttran_pr])
    ENABLE_PR=
    AC_SUBST([ENABLE_PR])
fi
# Enable/Disable 'opttran_pr' (END)

#------------------------------------------------------------------------------
# Enable/Disable 'opttran_ct' (BEGIN)
AC_MSG_CHECKING([whether to enable the 'OprTran Code Transformer'])
AC_ARG_ENABLE(opttran_ct,
    [AS_HELP_STRING([--disable-opttran_ct],
        [Enable/Disable the code transformer tool @<:@default=disabled@:>@])],
    ENABLE_OPTTRAN_CT=$enableval, ENABLE_OPTTRAN_CT=no)

if test x"$ENABLE_OPTTRAN_CT" = x"yes"; then
    AC_MSG_RESULT(yes)
    AC_DEFINE(OPTTRAN_ENABLE_CT, [1], [Enable/Disable opttran_ct])

    ENABLE_CT=ct
#    AC_CONFIG_FILES([tools/ct/Makefile])
    AC_SUBST([ENABLE_CT])
else
    AC_MSG_RESULT(no)
    AC_DEFINE(OPTTRAN_ENABLE_CT, [0], [Enable/Disable opttran_ct])
    ENABLE_CT=
    AC_SUBST([ENABLE_CT])
fi
# Enable/Disable 'opttran_ct' (END)

#------------------------------------------------------------------------------
# Enable/Disable 'opttran_cr' (BEGIN)
AC_MSG_CHECKING([whether to enable the 'OprTran Code Re-Assembler'])
AC_ARG_ENABLE(opttran_cr,
    [AS_HELP_STRING([--disable-opttran_cr],
        [Enable/Disable the code re-assembler tool @<:@default=disabled@:>@])],
    ENABLE_OPTTRAN_CR=$enableval, ENABLE_OPTTRAN_CR=no)

if test x"$ENABLE_OPTTRAN_CR" = x"yes"; then
    AC_MSG_RESULT(yes)
    AC_DEFINE(OPTTRAN_ENABLE_CR, [1], [Enable/Disable opttran_cr])

    ENABLE_CR=cr
#    AC_CONFIG_FILES([tools/cr/Makefile])
    AC_SUBST([ENABLE_CR])
else
    AC_MSG_RESULT(no)
    AC_DEFINE(OPTTRAN_ENABLE_CR, [0], [Enable/Disable opttran_cr])
    ENABLE_CR=
    AC_SUBST([ENABLE_CR])
fi
# Enable/Disable 'opttran_cr' (END)

#------------------------------------------------------------------------------
# Add path to install_dirs.h
# AC_CONFIG_FILES([config/install_dirs.h])

# Generate output files
AC_OUTPUT

AC_MSG_NOTICE([])
AC_MSG_NOTICE([================================================])
AC_MSG_NOTICE([  Great news! Now you just have to run "make".])
AC_MSG_NOTICE([================================================])
AC_MSG_NOTICE([])

# EOF
