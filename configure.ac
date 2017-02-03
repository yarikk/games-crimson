dnl Process this file with autoconf to produce a configure script.

AC_PREREQ([2.52])
AC_INIT([Crimson Fields], [0.5.3], [jensgr@gmx.net], [crimson])
AC_COPYRIGHT([Copyright (C) 2000-2009 Jens Granseuer])
AC_CONFIG_SRCDIR([src/common/globals.h])
AC_CONFIG_AUX_DIR([config])
AM_INIT_AUTOMAKE([foreign dist-bzip2 nostdinc])

dnl build time options
ZLIB_HOME=
AC_ARG_WITH([zlib],
  AC_HELP_STRING([--with-zlib=DIR], [use zlib in DIR]),
  [if test "x$withval" != xno; then
    use_zlib=yes
    if test "x$withval" != xyes; then
      ZLIB_HOME="$withval"
    fi
  else
    use_zlib=no
  fi], [use_zlib=yes])

AC_ARG_ENABLE([sound],
              AC_HELP_STRING([--disable-sound], [compile without sound support]),,
              [enable_sound=yes])

AC_ARG_ENABLE([network],
              AC_HELP_STRING([--disable-network], [compile without networking support]),,
              [enable_network=yes])

AC_ARG_ENABLE([cfed],
              AC_HELP_STRING([--enable-cfed], [build and install the level compiler]),,
              [enable_cfed=no])
AM_CONDITIONAL([BUILD_CFED], [test x$enable_cfed = xyes])

AC_ARG_ENABLE([comet],
              AC_HELP_STRING([--enable-comet], [build and install the graphical level editor]),,
              [enable_comet=no])
AM_CONDITIONAL([BUILD_COMET], [test x$enable_comet = xyes])

AC_ARG_ENABLE([bi2cf],
              AC_HELP_STRING([--enable-bi2cf], [build and install the Battle Isle map converter]),,
              [enable_bi2cf=no])
AM_CONDITIONAL([BUILD_BI2CF], [test x$enable_bi2cf = xyes])

AC_ARG_ENABLE([cf2bmp],
              AC_HELP_STRING([--enable-cf2bmp], [build and install the map to image converter]),,
              [enable_cf2bmp=no])
AM_CONDITIONAL([BUILD_CF2BMP], [test x$enable_cf2bmp = xyes])

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PATH_PROG([XSLTPROC], [xsltproc])
AC_PATH_PROG([XMLLINT], [xmllint])

dnl Checks for libraries.
SDL_VERSION=1.1.5
AM_PATH_SDL([$SDL_VERSION],:,
            [AC_MSG_ERROR([*** SDL version $SDL_VERSION not found!])])
CFLAGS="$CFLAGS $SDL_CFLAGS"
CXXFLAGS="$CXXFLAGS $SDL_CFLAGS"
CPPFLAGS="$CPPFLAGS $SDL_CFLAGS"
LIBS="$LIBS $SDL_LIBS"

AC_CHECK_LIB(SDL_ttf, TTF_OpenFont,
  [AC_CHECK_HEADER(SDL_ttf.h,
    [CF_LIBS="-lSDL_ttf"],
    [AC_MSG_ERROR([*** SDL_ttf.h not found!])])],
  [AC_MSG_ERROR([*** SDL_ttf not found!])])

if test x$enable_sound = xyes; then
  AC_CHECK_LIB(SDL_mixer, Mix_OpenAudio,
    [AC_CHECK_HEADER(SDL_mixer.h,
      [CF_LIBS="$CF_LIBS -lSDL_mixer"],
      [AC_MSG_WARN([*** SDL_mixer.h not found - disabling audio support!])
      enable_sound=no])],
    [AC_MSG_WARN([*** SDL_mixer not found - disabling audio support!])
    enable_sound=no])
fi
if test x$enable_sound != xyes; then
  AC_DEFINE(DISABLE_SOUND)
fi

if test x$enable_network = xyes; then
  AC_CHECK_LIB(SDL_net, SDLNet_Init,
    [AC_CHECK_HEADER(SDL_net.h,
      [CF_LIBS="$CF_LIBS -lSDL_net"],
      [AC_MSG_WARN([*** SDL_net.h not found - disabling network support!])
      enable_network=no])],
    [AC_MSG_WARN([*** SDL_net not found - disabling network support!])
    enable_network=no])
fi
if test x$enable_network != xyes; then
  AC_DEFINE(DISABLE_NETWORK)
fi

if test x$use_zlib = xyes; then
  ac_save_cppflags="$CPPFLAGS"
  ac_save_ldflags="$LDFLAGS"
  if test -n "$ZLIB_HOME"; then
    CPPFLAGS="-I${ZLIB_HOME}/include $CPPFLAGS"
    LDFLAGS="-L${ZLIB_HOME}/lib $LDFLAGS"
  fi
  AC_CHECK_LIB(z, compress,
    [AC_CHECK_HEADER(zlib.h,
      [AC_DEFINE(HAVE_LIBZ)
      LIBS="-lz $LIBS"]
    )]
  )
  if test "x$ac_cv_header_zlib_h" = xno; then
    CPPFLAGS="$ac_save_cppflags"
    LDFLAGS="$ac_save_ldflags"
  fi
fi

dnl Checks for header files and functions.
AC_HEADER_DIRENT
AC_CHECK_FUNCS(strcasecmp strncasecmp)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

AC_SUBST(CF_LIBS)

AC_CONFIG_FILES([doc/Makefile gfx/Makefile levels/Makefile \
                 locale/Makefile music/Makefile sfx/Makefile \
                 src/Makefile src/cf/Makefile src/comet/Makefile \
                 tools/Makefile Makefile crimson.spec])
AC_OUTPUT

