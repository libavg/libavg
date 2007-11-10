if [[ x"${AVG_PATH}" == x"" ]]
then
    echo Please set AVG_PATH
else
    export PATH=${AVG_PATH}/bin:${PATH}
    export CPPFLAGS="-I${AVG_PATH}/include "$CPPFLAGS
    export CXXFLAGS="-O3 -g -Wall -Wextra -pipe "$CXXFLAGS
    export CFLAGS="-O3 -g -Wall -Wextra -pipe "$CFLAGS
    export LDFLAGS="-L${AVG_PATH}/lib "$LDFLAGS
    export PKG_CONFIG_PATH=${AVG_PATH}/lib/pkgconfig
    export PYTHONPATH=${AVG_PATH}/lib/python2.3/site-packages/:$PYTHONPATH
fi
