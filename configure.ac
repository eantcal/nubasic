#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([nubasic], [1.32], [acaldmail@gmail.com])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([nu_expr_function.h])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
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

# ---------------------------------------------------------------------------- #
# Check if compiling tiny version
# ---------------------------------------------------------------------------- #

AC_MSG_CHECKING([whether compiling tinyver])
AC_ARG_ENABLE(
   [tinyver],
   [AS_HELP_STRING([--enable-tinyver], [strip graphics and sound support])],
   [want_tinyver=${enableval}],
   [
      if test "x${want_tinyver}" = "x" ; then
         want_tinyver="no"
      fi
   ]
)

case ${want_tinyver} in
yes)
   AC_MSG_RESULT(yes)
   ;;
no)
   AC_MSG_RESULT(no)
   ;;
*)
   AC_MSG_FAILURE([invalid argument for --eanble-tinyver: "$want_tinyver"])
   ;;
esac

AM_CONDITIONAL([COMPILE_TINYVER], [test "x$want_tinyver" = "xyes" ])

# ---------------------------------------------------------------------------- #

# If Tiny version, skip external dependency check
if test x"$want_tinyver" != x"yes" ; then

   AC_CHECK_PROG(XTERM_CHECK,xterm,yes)
   if test x"$XTERM_CHECK" != x"yes" ; then
      AC_MSG_ERROR([Please install xterm before installing.])
   fi

   AC_CHECK_PROG(XMSG_CHECK,xmessage,yes)
   if test x"$XMSG_CHECK" != x"yes" ; then
      AC_MSG_ERROR([Please install xmessage before installing.])
   fi


   # Checks for libraries.
   AC_PATH_X
   AC_CHECK_LIB([X11], [XDrawLine])

   # ------- SDL 2 Support ------------------------------------------------------ #

   AC_MSG_CHECKING([whether compiling sdl2])
   AC_ARG_ENABLE(
      [sdl2],
      [AS_HELP_STRING([--enable-sdl2], [use sdl2 library])],
      [want_sdl2=${enableval}],
      [
         if test "x${want_sdl2}" = "x" ; then
            want_sdl2="no"
         fi
      ]
   )

   case ${want_sdl2} in
   yes)
      AC_MSG_RESULT(yes)
      ;;
   no)
      AC_MSG_RESULT(no)
      ;;
   *)
      AC_MSG_FAILURE([invalid argument for --eanble-sdl2: "$want_sdl2"])
      ;;
   esac

   AM_CONDITIONAL([COMPILE_SDL2], [test "x$want_sdl2" = "xyes" ])

   if test "x$want_sdl2" = "xyes" ; then
      AC_CHECK_LIB([SDL2], [main])
   else
      AC_CHECK_PROG(XMSG_CHECK,aplay,yes)
      if test x"$XMSG_CHECK" != x"yes" ; then
          AC_MSG_ERROR([Please install aplay before installing.])
      fi
   fi
   
else
   AM_CONDITIONAL([COMPILE_SDL2],[test 0])
fi
# ---------------------------------------------------------------------------- #

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
