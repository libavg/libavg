
AC_PREREQ(2.53)

dnl Use this file to bump version on release
m4_include([m4/avg_version.m4])

AC_INIT(libavg, [VERSION_MAJOR.VERSION_MINOR.VERSION_MICRO], uzadow@libavg.de)

dnl Hack to detect virtualenv
if [[ x"$VIRTUAL_ENV" != "x" ]] && [[ $prefix = NONE ]]; then
    prefix=$VIRTUAL_ENV
    AC_MSG_RESULT([Virtualenv detected, setting prefix to $prefix])
fi

AC_CANONICAL_TARGET

AC_DISABLE_STATIC
AC_PROG_LIBTOOL

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/avgconfig.h.in])
AM_INIT_AUTOMAKE([check-news dist-zip])
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])
# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
CXXFLAGS="-O3 -ffast-math -Wall -pipe -Wno-invalid-offsetof -Woverloaded-virtual -Wswitch -Wuninitialized -Wempty-body $CXXFLAGS"
CFLAGS="-O3 -ffast-math -Wall -pipe $CFLAGS"
if [[ $host_cpu == i386 ]] || [[ $host_cpu == i686 ]] || [[ $host_cpu == x86_64 ]]; then
    CXXFLAGS="-msse2 $CXXFLAGS"
    CFLAGS="-msse2 $CFLAGS"
fi
OBJC="gcc"
OBJCFLAGS=""
OBJCLDFLAGS="-lobjc"
AC_SUBST(OBJC)
AC_SUBST(OBJCFLAGS)
AC_SUBST(OBJCLDFLAGS)
_AM_DEPENDENCIES([OBJC])
LDFLAGS="$LDFLAGS -rdynamic"

if [[ $target_vendor = apple ]] && [[ x"`which port`" = x"/opt/local/bin/port" ]]; then
    AC_MSG_RESULT([Macports detected. Adding include and lib search paths])
    CFLAGS="$CFLAGS -I/opt/local/include"
    CXXFLAGS="$CXXFLAGS -I/opt/local/include"
    LDFLAGS="$LDFLAGS -L/opt/local/lib"
fi

AC_SEARCH_LIBS([clock_gettime],[rt])
AC_CHECK_LIB([dl], [dlopen])

AC_PATH_GENERIC(xml2,,,AC_MSG_ERROR([libxml2 not found. Aborting.]))
PKG_CHECK_MODULES([PANGOFT2], [pangoft2])
AC_PATH_GENERIC(freetype,,,AC_MSG_ERROR([libfreetype not found. Aborting.]))

PKG_CHECK_MODULES([GDK_PIXBUF], [gdk-pixbuf-2.0])
PKG_CHECK_MODULES([LIBRSVG], [librsvg-2.0])
PKG_CHECK_MODULES([FONTCONFIG], [fontconfig])

PKG_CHECK_MODULES([FFMPEG], [libswscale libavformat libavcodec libavutil], [LIBFFMPEG="$FFMPEG_LIBS"], [:])
AC_SUBST(LIBFFMPEG)
AC_CHECK_HEADERS([libavformat/avformat.h])
if test -z "$FFMPEG_LIBS"; then
  AC_MSG_ERROR([Could not find ffmpeg development libraries.])
fi

PKG_CHECK_MODULES([AVRESAMPLE], [libavresample], [LIBAVRESAMPLE="$AVRESAMPLE_LIBS"], [:])
AC_SUBST(LIBAVRESAMPLE)
AC_CHECK_HEADERS([libavresample/avresample.h])

AM_PATH_PYTHON
AX_PYTHON_DEVEL

AC_ARG_VAR(BOOST_PYTHON_LIBS, [linker flags for BOOST, defaults to -lboost_python])
if test -z "$BOOST_PYTHON_LIBS"
then
  AC_CHECK_LIB(boost_python, main, libboost_python_installed=yes, , $PYTHON_LDFLAGS)
  if test -z "$libboost_python_installed"
  then
    AC_MSG_ERROR([boost.python not found. Aborting.])
  fi
  BOOST_PYTHON_LIBS="-lboost_python"
fi
    
AX_BOOST_THREAD
if test -z "$BOOST_THREAD_LIBS"; then
   AC_MSG_ERROR([boost.thread not found. Aborting.])
fi

AC_ARG_ENABLE(dc1394,
              AC_HELP_STRING([--enable-dc1394], 
                    [compile support for firewire cameras]),
                    enable_1394=$enableval, enable_1394=maybe)
if test "$enable_1394" = maybe; then
    PKG_CHECK_MODULES([DC1394_2], [libdc1394-2],,AC_MSG_NOTICE([libdc1394 ver 2 not found]))
fi
AM_CONDITIONAL(ENABLE_1394_2, test x"$DC1394_2_LIBS" != x )
if test x"$DC1394_2_LIBS" != "x"; then
    AC_DEFINE(AVG_ENABLE_1394_2, 1, [Enable firewire camera support v.2])
fi

if test $target_vendor = apple; then
    AM_CONDITIONAL(HAVE_XI2_1, false)
    AM_CONDITIONAL(HAVE_XI2_2, false)
else
    PKG_CHECK_MODULES(XI2_1, [xi >= 1.4.1.99.1] [inputproto >= 2.0.99.1] [inputproto < 2.1.99.5],
                      HAVE_XI2_1="yes"; AC_DEFINE(HAVE_XI2_1, 1, [XI2_1 available]),
                      HAVE_XI2_1="no");
    PKG_CHECK_MODULES(XI2_2, [xi >= 1.5.99.2] [inputproto >= 2.1.99.5],
                      HAVE_XI2_2="yes"; AC_DEFINE(HAVE_XI2_2, 1, [XI2_2 available]),
                      HAVE_XI2_2="no");
    AM_CONDITIONAL(HAVE_XI2_1, [ test "$HAVE_XI2_1" = "yes" ])
    AM_CONDITIONAL(HAVE_XI2_2, [ test "$HAVE_XI2_2" = "yes" ])
fi

AC_ARG_ENABLE(v4l2,
          AC_HELP_STRING([--enable-v4l2],
                 [compile support for video4linux v2 video devices]),
                 enable_v4l2=$enableval, enable_v4l2=maybe)
if test "$enable_v4l2" = maybe; then
    AC_CHECK_TYPE([struct v4l2_buffer],
        [enable_v4l2=yes],
        [enable_v4l2=no],
        [#include <sys/time.h>
         #include <linux/videodev2.h>])
fi
AM_CONDITIONAL(ENABLE_V4L2, test x$enable_v4l2 = xyes)
if test "$enable_v4l2" = yes; then
    AC_DEFINE(AVG_ENABLE_V4L2, 1, [Enable Video4Linux2 camera support])
fi

AC_CHECK_HEADER([linux/ppdev.h], enable_ParPort=yes, enable_ParPort=no)
AM_CONDITIONAL(ENABLE_PARPORT, test x$enable_ParPort = xyes)
if test "$enable_ParPort" = yes; then
    AC_DEFINE(AVG_ENABLE_PARPORT, 1, [Enable parallel port support])
fi 

AC_ARG_ENABLE(egl,
        AC_HELP_STRING([--enable-egl], [include EGL support [default=no]]),
        ,
        enable_egl=no)

AC_ARG_ENABLE(rpi,
        AC_HELP_STRING([--enable-rpi], [configure for RaspberryPi [default=no]]),
        ,
        enable_rpi=no)

if test "$enable_rpi" = yes; then
    CPPFLAGS="$CPPFLAGS -I/opt/vc/include -I/opt/vc/include/interface/vmcs_host/linux -I/opt/vc/include/interface/vcos/pthreads"
    LDFLAGS="$LDFLAGS -L/opt/vc/lib"
    AC_DEFINE(AVG_ENABLE_RPI, 1, [Build on RaspberryPi])
    AM_CONDITIONAL(ENABLE_RPI, true)
    if test "$enable_egl" = no; then
        enable_egl=yes
        AC_MSG_NOTICE([--enable-rpi requires --enable-egl, added --enable-egl])
    fi
    # suppress several hundreds "swp{b} use is deprecated for this architecture" assembler warnings
    CFLAGS="$CFLAGS -Wa,--no-warn"
    CXXFLAGS="$CXXFLAGS -Wa,--no-warn"
else
    AM_CONDITIONAL(ENABLE_RPI, false)
fi

if test "$enable_egl" = yes; then
    AC_CHECK_HEADERS(["EGL/egl.h"], [egl_present=yes;])
    AC_CHECK_HEADERS(["GLES2/gl2.h"], [gles2_present=yes;])
    if [[ x"$egl_present" == x ]] || [[ x"$gles2_present" == x ]]; then
        AC_MSG_ERROR([EGL not found. Aborting])
    fi
    AC_DEFINE(AVG_ENABLE_EGL, 1, [Enable EGL support])
else
    AX_CHECK_GL
    AX_CHECK_GLU
    if test x"$GL_LIBS" = x; then
        AC_MSG_ERROR([OpenGL not found. Aborting])
    fi
    AC_DEFINE(AVG_ENABLE_OPENGL, 1, [Enable OpenGL support])
fi
AM_CONDITIONAL(ENABLE_EGL, test x"$egl_present" != x)
AM_CONDITIONAL(ENABLE_OPENGL, test x"$GL_LIBS" != x)

AM_PATH_SDL(1.2.10,:,AC_MSG_ERROR([libSDL not found. Aborting.]))

AC_CHECK_LIB([vdpau],[main],let vdpau_lib=1,let vdpau_lib=0)
AC_CHECK_HEADERS([libavcodec/vdpau.h],let vdpau_h1=1,let vdpau_h1=0)
AC_CHECK_HEADERS([vdpau/vdpau.h],let vdpau_h2=1,let vdpau_h2=0)
if [[ $vdpau_lib -eq 1 ]] && [[ $vdpau_h1 -eq 1 ]] && [[ $vdpau_h2 -eq 1 ]]; then
  AC_DEFINE(AVG_ENABLE_VDPAU, 1, [Enable VDPAU support])
  LIBVIDEO_LDADD="-lvdpau"
  AM_CONDITIONAL(USE_VDPAU_SRC, true)
else
  LIBVIDEO_LDADD=""
  AM_CONDITIONAL(USE_VDPAU_SRC, false)
fi
AC_SUBST([LIBVIDEO_LDADD])

AM_CONDITIONAL(APPLE, test $target_vendor = apple)

PREFIX=$prefix
AC_SUBST(PREFIX)
AC_CONFIG_HEADERS(src/avgconfig.h)
AC_OUTPUT(Makefile 
    src/Makefile 
    src/test/Makefile 
    src/test/plugin/Makefile
    src/base/Makefile 
    src/base/triangulate/Makefile
    src/lmfit/Makefile 
    src/wrapper/Makefile 
    src/player/Makefile 
    src/graphics/Makefile 
    src/graphics/shaders/Makefile 
    src/python/Makefile
    src/imaging/Makefile 
    src/video/Makefile 
    src/audio/Makefile 
    src/utils/Makefile 
    src/anim/Makefile 
    src/python/data/Makefile 
    src/samples/Makefile 
    src/samples/firebirds/Makefile
    src/samples/firebirds/plugin/Makefile
    src/samples/firebirds/media/Makefile
    src/python/widget/Makefile 
    src/python/app/Makefile 
    src/oscpack/Makefile 
    man/Makefile)
