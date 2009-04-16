#!/bin/sh

if [ -z "$1" ]
then
    echo "$0 <outpath>"
elif which svn >/dev/null
then
    if [ -d .svn ]
    then
        LC_ALL=C svn info >"$1/version.txt"
    else
        REL=$(basename $PWD)
        echo "Version: $REL" >$1/version.txt
    fi
fi
