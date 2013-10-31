if [[ x"${AVG_PATH}" == x"" ]]
then
    echo Please set AVG_PATH
else
    DARWINVER=`uname -r`
    DARWINMAJORVER=${DARWINVER%%.*}
    if [[ "${DARWINMAJORVER}" == "13" ]]
    then
        DEBUGINFOFLAG="-g"
    else
        DEBUGINFOFLAG="-gstabs"
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
