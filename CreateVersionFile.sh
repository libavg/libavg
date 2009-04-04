#!/bin/sh

if [ -z "$1" ]
then
    echo "$0 <outpath>"
elif which -s svn
then
    LC_ALL=C svn info >"$1/version.txt"
fi
