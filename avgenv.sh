#!/bin/bash
# This shell script finds the mozilla components on different setups 
# and sets shell variables accordingly.

export MOZ_BASEDIR=`mozilla-config --prefix`

if [[ -e $MOZ_BASEDIR/lib/xpidl ]]
then
    # This is SuSE
    export MOZ_LIBDIR=$MOZ_BASEDIR/lib
    export MOZ_IDLDIR=$MOZ_BASEDIR/share/idl
elif [[ -e $MOZ_BASEDIR/lib/mozilla/xpidl ]]
then
    # This is debian
    export MOZ_LIBDIR=$MOZ_BASEDIR/lib/mozilla
    export MOZ_IDLDIR=$MOZ_BASEDIR/share/idl/mozilla
else
    # This is mozilla installed from source
    export MOZ_VERSION=`mozilla-config --version`
    TEST_MOZ_LIBDIR=$MOZ_BASEDIR/lib/mozilla-$MOZ_VERSION
    if [[ -e $TEST_MOZ_LIBDIR/xpidl ]]
    then
        export MOZ_LIBDIR = $TEST_MOZ_LIBDIR
        export MOZ_IDLDIR = $MOZ_BASEDIR/share/idl/mozilla-$MOZ_VERSION
    fi
fi

# General
export MOZ_COMPONENTSDIR=$MOZ_LIBDIR/components
export LD_LIBRARY_PATH=$MOZ_LIBDIR:$LD_LIBRARY_PATH
export AVG_INCLUDE_PATH=`libavg-config --includedir`

#echo
#echo "libavg variables: "
#echo "    "'$MOZ_BASEDIR'" = $MOZ_BASEDIR"
#echo "    "'$MOZ_LIBDIR'" = $MOZ_LIBDIR"
#echo "    "'$MOZ_IDLDIR'" = $MOZ_IDLDIR"
#echo "    "'$MOZ_COMPONENTSDIR'" = $MOZ_COMPONENTSDIR"
#echo "    "'$AVG_INCLUDE_PATH'" = $AVG_INCLUDE_PATH"

