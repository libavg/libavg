#!/bin/sh
for fname in *.h *.cpp
do
    echo $fname
    tr -d '\015\010' < $fname > $fname.lf
    cp $fname.lf $fname
    rm $fname.lf
done
