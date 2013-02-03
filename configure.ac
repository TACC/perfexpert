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

###############################################################################
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
    AC_CHECK_LIB([sqlite3], [sqlite3_open],,
        [AC_MSG_ERROR([SQLite3 not found!])])

    ENABLE_RECOMMENDER=recommender
    AC_CONFIG_FILES([tools/recommender/Makefile])
    AC_SUBST([ENABLE_RECOMMENDER])
else
    AC_MSG_RESULT(no)
    AC_DEFINE(OPTTRAN_ENABLE_RECOMMENDER, [0], [Disable recommender])
    ENABLE_RECOMMENDER=
    AC_SUBST([ENABLE_RECOMMENDER])
fi
# Enable/Disable 'recommender' (END)

###############################################################################
# Enable/Disable 'opttran_pr' (BEGIN)
opttran_pr_default="yes"
AC_MSG_CHECKING([whether to enable the 'OptTran Pattern Recognizer'])
AC_ARG_ENABLE(opttran_pr,
    [AS_HELP_STRING([--disable-opttran_pr],
        [disable the pattern recognizer tool [default=enabled]])],,
    enable_opttran_pr=$opttran_pr_default)

if test "x$enable_opttran_pr" = "xyes"; then
    AC_MSG_RESULT(yes)
    AC_DEFINE(OPTTRAN_ENABLE_PR, [1], [Disable opttran_pr])

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
    AC_DEFINE(OPTTRAN_ENABLE_PR, [0], [Disable opttran_pr])
    ENABLE_PR=
    AC_SUBST([ENABLE_PR])
fi
# Enable/Disable 'opttran_pr' (END)

###############################################################################
# Enable/Disable 'opttran_ct' (BEGIN)
opttran_ct_default="yes"
AC_MSG_CHECKING([whether to enable the 'OprTran Code Transformer'])
AC_ARG_ENABLE(opttran_ct,
    [AS_HELP_STRING([--disable-opttran_ct],
        [disable the code transformer tool [default=enabled]])],,
    enable_opttran_ct=$opttran_ct_default)

if test "x$enable_opttran_ct" = "xyes"; then
    AC_MSG_RESULT(yes)
    AC_DEFINE(OPTTRAN_ENABLE_CT, [1], [Disable opttran_ct])

    ENABLE_CT=ct
#    AC_CONFIG_FILES([tools/ct/Makefile])
    AC_SUBST([ENABLE_CT])
else
    AC_MSG_RESULT(no)
    AC_DEFINE(OPTTRAN_ENABLE_CT, [0], [Disable opttran_ct])
    ENABLE_CT=
    AC_SUBST([ENABLE_CT])
fi
# Enable/Disable 'opttran_ct' (END)

###############################################################################
# Enable/Disable 'opttran_cr' (BEGIN)
opttran_cr_default="yes"
AC_MSG_CHECKING([whether to enable the 'OprTran Code Re-Assembler'])
AC_ARG_ENABLE(opttran_cr,
    [AS_HELP_STRING([--disable-opttran_cr],
        [disable the code re-assembler tool [default=enabled]])],,
    enable_opttran_cr=$opttran_cr_default)

if test "x$enable_opttran_cr" = "xyes"; then
    AC_MSG_RESULT(yes)
    AC_DEFINE(OPTTRAN_ENABLE_CR, [1], [Disable opttran_cr])

    ENABLE_CR=cr
#    AC_CONFIG_FILES([tools/cr/Makefile])
    AC_SUBST([ENABLE_CR])
else
    AC_MSG_RESULT(no)
    AC_DEFINE(OPTTRAN_ENABLE_CR, [0], [Disable opttran_cr])
    ENABLE_CR=cr
    AC_SUBST([ENABLE_CR])
fi
# Enable/Disable 'opttran_cr' (END)

# Add path to install_dirs.h
AC_CONFIG_FILES([config/install_dirs.h])

# Generate output files
AC_OUTPUT

AC_MSG_NOTICE([])
AC_MSG_NOTICE([================================================])
AC_MSG_NOTICE([  Great news! Now you just have to run "make".])
AC_MSG_NOTICE([================================================])
AC_MSG_NOTICE([])

# EOF
