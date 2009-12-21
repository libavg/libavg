if [[ x"${AVG_PATH}" == x"" ]]
then
    echo Please set AVG_PATH
else
    export PATH=${AVG_PATH}/bin:${PATH}
    export CPPFLAGS="-I${AVG_PATH}/include "$CPPFLAGS
    export CXXFLAGS="-O3 -g -Wall -pipe "$CXXFLAGS
    export CFLAGS="-O3 -g -Wall -pipe "$CFLAGS
    export LDFLAGS="-L${AVG_PATH}/lib "$LDFLAGS
    export PKG_CONFIG_PATH=${AVG_PATH}/lib/pkgconfig
    DARWINVER=`uname -r`
    DARWINMAJORVER=${DARWINVER%%.*}
    if [[ "${DARWINMAJORVER}" == "10" ]]
    then
        export PYTHONPATH=${AVG_PATH}/lib/python2.6/site-packages/:$PYTHONPATH
    else
        export PYTHONPATH=${AVG_PATH}/lib/python/2.5/site-packages/:$PYTHONPATH
    export AVG_MAC_ENV_SET=1
fi
