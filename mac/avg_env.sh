if [[ x"${AVG_PATH}" == x"" ]]
then
    echo Please set AVG_PATH
else
    # Set the debug info flag to use depending on whether clang is used as compiler.
    # Is there an easier way to do this?
    if [[ "${CXX}" == "" ]]
    then
        CXX=gcc
    fi
    IS_CLANG=`${CXX} --version | grep clang`
    if [[ "${IS_CLANG}" == "" ]]
    then
        DEBUGINFOFLAG="-gstabs"
    else
        DEBUGINFOFLAG="-g"
    fi
    export PATH=${AVG_PATH}/bin:${PATH}
    export CPPFLAGS="-I${AVG_PATH}/include "$CPPFLAGS
    export CXXFLAGS="-O3 ${DEBUGINFOFLAG} -Wall -pipe "$CXXFLAGS
    export CFLAGS="-O3 ${DEBUGINFOFLAG} -Wall -pipe "$CFLAGS
    export LDFLAGS="-L${AVG_PATH}/lib "$LDFLAGS
    export PKG_CONFIG_PATH=${PKG_CONFIG_PATH}:${AVG_PATH}/lib/pkgconfig
    DARWINVER=`uname -r`
    DARWINMAJORVER=${DARWINVER%%.*}
    if [[ "${DARWINMAJORVER}" == "10" ]]
    then
        export PYTHONPATH=/usr/local/lib/python2.6/site-packages/:$PYTHONPATH
    else
        export PYTHONPATH=${AVG_PATH}/lib/python/2.5/site-packages/:$PYTHONPATH
    fi
    export AVG_MAC_ENV_SET=1
fi
