dnl a macro to check for ability to create python extensions
dnl  AM_CHECK_PYTHON_HEADERS([ACTION-IF-POSSIBLE], [ACTION-IF-NOT-POSSIBLE])
dnl function also defines PYTHON_INCLUDES
AC_DEFUN([AM_CHECK_PYTHON_HEADERS],
[AC_REQUIRE([AM_PATH_PYTHON])
AC_MSG_CHECKING(for headers required to compile python extensions)
dnl deduce PYTHON_INCLUDES
py_prefix=`$PYTHON -c "import sys; print sys.prefix"`
py_exec_prefix=`$PYTHON -c "import sys; print sys.exec_prefix"`
PYTHON_INCLUDES="-I${py_prefix}/include/python${PYTHON_VERSION}"
if test "$py_prefix" != "$py_exec_prefix"; then
  PYTHON_INCLUDES="$PYTHON_INCLUDES -I${py_exec_prefix}/include/python${PYTHON_VERSION}"
fi
AC_SUBST(PYTHON_INCLUDES)
dnl check if the headers exist:
save_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $PYTHON_INCLUDES"
AC_TRY_CPP([#include <Python.h>],dnl
[AC_MSG_RESULT(found)
$1],dnl
[AC_MSG_RESULT(not found)
$2])
CPPFLAGS="$save_CPPFLAGS"
])

AC_DEFUN([AM_CHECK_PYTHON_LIB],
[AC_REQUIRE([AM_PATH_PYTHON])
AC_REQUIRE([AM_CHECK_PYTHON_HEADERS])

AC_MSG_CHECKING(for libpython${PYTHON_VERSION})

py_config_dir=${py_exec_prefix}/lib/python${PYTHON_VERSION}/config
py_makefile="${py_config_dir}/Makefile"
if test -f "$py_makefile"; then
dnl extra required libs
  py_localmodlibs=`sed -n -e 's/^LOCALMODLIBS=\(.*\)/\1/p' $py_makefile`
  py_basemodlibs=`sed -n -e 's/^BASEMODLIBS=\(.*\)/\1/p' $py_makefile`
  py_other_libs=`sed -n -e 's/^LIBS=\(.*\)/\1/p' $py_makefile`

dnl now the actual libpython
  if test -e "${py_config_dir}/libpython${PYTHON_VERSION}.a"; then
    PYTHON_LIBS="-L${py_config_dir} -lpython${PYTHON_VERSION} $py_localmodlibs $py_basemodlibs $py_other_libs"
    AC_MSG_RESULT(found)
  else
    if test -e "/usr/lib/libpython${PYTHON_VERSION}.dylib"; then
      PYTHON_LIBS="-lpython${PYTHON_VERSION} $py_localmodlibs $py_basemodlibs $py_other_libs"
      AC_MSG_RESULT(found)
    else
     AC_MSG_RESULT(not found)
    fi
  fi
fi
AC_SUBST(PYTHON_LIBS)])

