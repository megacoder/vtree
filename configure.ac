#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

OWNER=[Tommy.Reynolds@MegaCoder.com]

AC_PREREQ([2.66])
AC_INIT([vtree], [1.0.0], OWNER)
AC_CONFIG_SRCDIR([src/vtree.c])
AC_CONFIG_HEADERS([config.h])

# Setup other autotools

AM_INIT_AUTOMAKE

AC_SUBST(OWNER)

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([malloc.h stdlib.h string.h sys/param.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UID_T
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([getcwd strcasecmp strchr strdup strerror])

AC_CONFIG_FILES([
        Makefile
        vtree.spec
        src/Makefile
        man/Makefile
])
AC_OUTPUT
