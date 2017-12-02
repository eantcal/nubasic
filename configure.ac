#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([nubasic], [1.48], [antonino.calderone@gmail.com])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([main.cc])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_CHECK_PROG(AR, ar, ar)
dnl Set default for ARFLAGS, since autoconf does not have a macro for it.
dnl This allows people to set it when running configure or make
test -n "$ARFLAGS" || ARFLAGS="cr"
AC_PROG_RANLIB
AC_PROG_CXX
AC_PROG_CC


# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdio_ext.h stdlib.h string.h sys/ioctl.h termios.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([floor pow select strcasecmp strerror])


gnome_terminal="no"

# If Tiny version, skip external dependency check
# ---------------------------------------------------------------------------- #

AC_CONFIG_FILES([Makefile lib/Makefile])

AC_OUTPUT
