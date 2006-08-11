if [[ x"${AVG_PATH}" == x"" ]]
then
    echo Please set AVG_PATH
else
    export PATH=${AVG_PATH}/bin:${PATH}
    export CPPFLAGS="-I${AVG_PATH}/include "$CPPFLAGS
    export CXXFLAGS="-g "$CXXFLAGS
    export LDFLAGS="-L${AVG_PATH}/lib "$LDFLAGS
    export PKG_CONFIG_PATH=${AVG_PATH}/lib/pkgconfig
    export PYTHONPATH=${AVG_PATH}/lib/python2.3/site-packages/libavg/:$PYTHONPATH
fi
export CFLAGS=-g
