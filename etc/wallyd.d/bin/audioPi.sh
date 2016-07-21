#!/bin/sh

if [ -z $1 ]; then
    echo usage : $0 [0|1|2] 
    echo 0 = Auto
    echo 1 = Jack
    echo 2 = HDMI
    exit
fi

amixer cset numid=3 $1

