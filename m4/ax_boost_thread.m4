dnl @synopsis AX_BOOST_THREAD
dnl
dnl This macro checks to see if the Boost.Thread library is installed.
dnl It also attempts to guess the currect library name using several
dnl attempts. It tries to build the library name using a user supplied
dnl name or suffix and then just the raw library.
dnl
dnl If the library is found, HAVE_BOOST_THREAD is defined and
dnl BOOST_THREAD_LIBS is set to the name of the library.
dnl
dnl This macro calls AC_SUBST(BOOST_THREAD_LIBS).
dnl
dnl @category InstalledPackages
dnl @author Michael Tindal <mtindal@paradoxpoint.com>
dnl @version 2004-09-20
dnl @license GPLWithACException

AC_DEFUN([AX_BOOST_THREAD],
[AC_REQUIRE([AC_CXX_NAMESPACES])dnl

AC_LANG_SAVE
AC_LANG_CPLUSPLUS
CXXFLAGS_SAVE=$CXXFLAGS
LIBS_SAVE=$LIBS
dnl FIXME: need to include a generic way to check for the flag
dnl to turn on threading support.
CXXFLAGS="-pthread $CXXFLAGS"

AC_CACHE_CHECK(whether the Boost::Thread library is available,
ax_cv_boost_thread,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <boost/thread/thread.hpp>]],
                       [[boost::thread_group thrds; return 0;]])],
               ax_cv_boost_thread=yes, ax_cv_boost_thread=no)
])

if test "$ax_cv_boost_thread" = yes; then
  AC_DEFINE(HAVE_BOOST_THREAD,,[define if the Boost::Thread library is available])
  dnl Now determine the appropriate file names
  AC_ARG_WITH([boost-thread],AS_HELP_STRING([--with-boost-thread],
  [specify the boost thread library suffix to use]),
  [if test "x$with_boost_thread" != "xno"; then
    ax_boost_thread_lib=boost_thread$with_boost_thread
  fi])
  for ax_lib in $ax_boost_thread_lib boost_thread boost_thread-mt; do
    AC_CHECK_LIB($ax_lib, main, [BOOST_THREAD_LIBS=-l$ax_lib; break])
  done

  # OXullo 2012-07-18: since boost 1.50, boost::thread depends on boost::system
  AC_CACHE_CHECK(whether Boost::Thread needs Boost::System library,
  ax_cv_boost_thread_system,
  [LIBS="$LIBS $BOOST_THREAD_LIBS"
  AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <boost/thread/thread.hpp>]],
            [[boost::thread_group thrds; return 0;]])],
            [ax_cv_boost_thread_system=no],
            [LIBS="$LIBS $BOOST_THREAD_LIBS -lboost_system$with_boost_thread"
                AC_LINK_IFELSE([
                    AC_LANG_PROGRAM([[#include <boost/thread/thread.hpp>]],
                        [[boost::thread_group thrds; return 0;]])
                    ],
                    [BOOST_THREAD_LIBS="$BOOST_THREAD_LIBS -lboost_system$with_boost_thread"
                        ax_cv_boost_thread_system=yes],
                    [AC_ERROR([Cannot use Boost::Thread])]
                    )])
  ])
  
  CXXFLAGS=$CXXFLAGS_SAVE
  LIBS=$LIBS_SAVE
  AC_LANG_RESTORE

  AC_SUBST(BOOST_THREAD_LIBS)
fi
])dnl
