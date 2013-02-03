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
AC_INIT([OptTran], [0.1], [Leonardo Fialho: fialho@utexas.edu], [opttran.tar.gz], [http://www.tacc.com.br/opttran/])
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

# Check the operation system
case "$host" in
	*-darwin*)
	AC_DEFINE([OS_MACOS], 1, [The target operating system is a Mac OSX])
	AC_DEFINE([_XOPEN_SOURCE], 600, [Generate dependencies for Mountain Lion])
	;;
	*-linux*)
	AC_DEFINE([OS_LINUX], 1, [The target operating system is a Linux])
	;;
esac

# Checks for required (but generic) header files
AC_CHECK_HEADER([stdio.h],  [AC_DEFINE([HAVE_STDIO_H],  1, [You have <stdio.h>.])],  [AC_MSG_ERROR([not found: stdio.h])])
AC_CHECK_HEADER([getopt.h], [AC_DEFINE([HAVE_GETOPT_H], 1, [You have <getopt.h>.])], [AC_MSG_ERROR([not found: getopt.h])])
AC_CHECK_HEADER([ctype.h],  [AC_DEFINE([HAVE_CTYPE_H],  1, [You have <ctype.h>.])],  [AC_MSG_ERROR([not found: ctype.h])])
AC_CHECK_HEADER([stdlib.h], [AC_DEFINE([HAVE_STDLIB_H], 1, [You have <stdlib.h>.])], [AC_MSG_ERROR([not found: stdlib.h])])
AC_CHECK_HEADER([stdarg.h], [AC_DEFINE([HAVE_STDARG_H], 1, [You have <stdarg.h>.])], [AC_MSG_ERROR([not found: stdarg.h])])
AC_CHECK_HEADER([string.h], [AC_DEFINE([HAVE_STRING_H], 1, [You have <string.h>.])], [AC_MSG_ERROR([not found: string.h])])
AC_CHECK_HEADER([unistd.h], [AC_DEFINE([HAVE_UNISTD_H], 1, [You have <unistd.h>.])], [AC_MSG_ERROR([not found: unistd.h])])

# Checks for typedefs, structures, and compiler characteristics
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions
AC_FUNC_MALLOC

# Output files
AC_CONFIG_FILES([Makefile
	tools/Makefile])

# Enable/Disable 'recommender' (BEGIN)
recommender_default="yes"
AC_MSG_CHECKING([whether to enable the 'Optimization Recommender'])
AC_ARG_ENABLE(recommender,
    [AS_HELP_STRING([--disable-recommender],
        [disable the optimization recommender tool [default=enabled]])],,
    enable_recommender=$recommender_default)

if test "x$enable_recommender" = "xyes"; then
    AC_MSG_RESULT(yes)
    AC_DEFINE(OPTTRAN_ENABLE_RECOMMENDER, [1], [Disable recommender])

# Checks for SQLite library functions (only if 'recommender' is enabled)
    AC_CHECK_LIB([sqlite3], [sqlite3_open],,   [AC_MSG_ERROR([SQLite3 not found!])])
    AC_CHECK_LIB([sqlite3], [sqlite3_close],,  [AC_MSG_ERROR([SQLite3 not found!])])
    AC_CHECK_LIB([sqlite3], [sqlite3_exec],,   [AC_MSG_ERROR([SQLite3 not found!])])
    AC_CHECK_LIB([sqlite3], [sqlite3_free],,   [AC_MSG_ERROR([SQLite3 not found!])])
    AC_CHECK_LIB([sqlite3], [sqlite3_errmsg],, [AC_MSG_ERROR([SQLite3 not found!])])

    AC_CONFIG_FILES([tools/recommender/Makefile])
else
    AC_MSG_RESULT(no)
    AC_DEFINE(OPTTRAN_ENABLE_RECOMMENDER, [0], [Disable recommender])
fi
# Enable/Disable 'recommender' (END)

# Enable/Disable 'opttran_pr' (BEGIN)
opttran_pr_default="yes"
AC_MSG_CHECKING([whether to enable the 'Pattern Recognizer'])
AC_ARG_ENABLE(opttran_pr,
    [AS_HELP_STRING([--disable-recommender],
        [disable the pattern recognizer tool [default=enabled]])],,
    enable_opttran_pr=$opttran_pr_default)

if test "x$enable_opttran_pr" = "xyes"; then
    AC_MSG_RESULT(yes)
    AC_DEFINE(OPTTRAN_ENABLE_PR, [1], [Disable opttran_pr])

# Check for Lex/Flex and Yacc/Bison
    AC_PROG_LEX
    AC_PROG_YACC
    
#    AC_CONFIG_FILES([tools/pr/Makefile])
else
    AC_MSG_RESULT(no)
    AC_DEFINE(OPTTRAN_ENABLE_PR, [0], [Disable opttran_pr])
fi
# Enable/Disable 'opttran_pr' (END)

# Add path to config.h
AC_CONFIG_FILES([config/install_dirs.h])

# Generate output files
AC_OUTPUT

AC_MSG_NOTICE([])
AC_MSG_NOTICE([================================================])
AC_MSG_NOTICE([  Great news! Now you just have to run "make".])
AC_MSG_NOTICE([================================================])
AC_MSG_NOTICE([])

# EOF
