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

# AC_CANONICAL_HOST is needed to access the 'host_os' variable    
AC_CANONICAL_HOST

build_linux=no
build_windows=no
build_mac=no

# Detect the target system
case "${host_os}" in
    linux*)
        build_linux=yes
        ;;
    cygwin*|mingw*)
        build_windows=yes
        ;;
    darwin*)
        build_mac=yes
        ;;
    *)
        AC_MSG_ERROR(["OS $host_os is not supported"])
        ;;
esac

# Pass the conditionals to automake
AM_CONDITIONAL([LINUX], [test "$build_linux" = "yes"])
AM_CONDITIONAL([WINDOWS], [test "$build_windows" = "yes"])
AM_CONDITIONAL([OSX], [test "$build_mac" = "yes"])

if test "x${build_mac}" = "xyes" ; then
  want_tinyver="yes"
  want_gtkide="no"
fi

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdio_ext.h stdlib.h string.h sys/ioctl.h termios.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([floor pow select strcasecmp strerror])

# ---------------------------------------------------------------------------- #
# Check if compiling gtk ide
# ---------------------------------------------------------------------------- #
AC_MSG_CHECKING([whether compiling gtkide])
AC_ARG_ENABLE(
   [gtkide],
   [AS_HELP_STRING([--enable-gtkide], [build gtk ide])],
   [want_gtkide=${enableval}],
   [
      if test "x${want_gtkide}" = "x" ; then
         want_gtkide="no"
      fi
   ]
)

case ${want_gtkide} in
yes)
   AC_MSG_RESULT(yes)
   ;;
no)
   AC_MSG_RESULT(no)
   ;;
*)
   AC_MSG_FAILURE([invalid argument for --enable-gtkide: "$want_gtkide"])
   ;;
esac

AM_CONDITIONAL([COMPILE_GTKIDE], [test "x$want_gtkide" = "xyes" ])

# If Tiny version, skip external dependency check
if test x"$want_gtkide" == x"yes" ; then
   PKG_CHECK_MODULES([NUBASICIDE], [
     glib-2.0
     gmodule-2.0
     gtk+-2.0])
fi


# ---------------------------------------------------------------------------- #

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

gnome_terminal="no"

# If Tiny version, skip external dependency check
if test x"$want_tinyver" != x"yes" ; then

   AC_CHECK_PROG(XTERM_CHECK,xterm,yes)

   AC_MSG_WARN([Please install xterm for graphical programs.])

   if test x"$XTERM_CHECK" != x"yes" ; then
   AC_CHECK_PROG(XTERM_CHECK,gnome-terminal,yes)
        
       if test x"$XTERM_CHECK" != x"yes" ; then
          AC_MSG_ERROR([Please install xterm or gnome-terminal before install.])
       else
          gnome_terminal="yes"
       fi
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

AC_CONFIG_FILES([Makefile lib/Makefile])

AM_CONDITIONAL([GNOME_TERMINAL], [test "x$gnome_terminal" = "xyes" ])
# ---------------------------------------------------------------------------- #

AM_CONDITIONAL([GTKIDE], [test x"$want_gtkide" == x"yes"])
AM_COND_IF([GTKIDE], [AC_CONFIG_FILES([miptknzr/lib/Makefile mipjson/lib/Makefile ide/gtk/Makefile])])

AC_OUTPUT
