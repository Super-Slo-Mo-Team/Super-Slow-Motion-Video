#!/bin/bash

INPUT="IO/input.y4m"
OUTPUT="IO/output.y4m"
SLOWDOWN=2
FPS=50

TMP="IO/temp.yuv"

rm build/yuv/*

cd build
./Super_Slow_Motion_Video "../$INPUT" "../$OUTPUT" $SLOWDOWN $FPS
cd ..

cat build/tmp/yuv/*.yuv > $TMP
ffmpeg -f rawvideo -pix_fmt yuv420p -s:v 352x288 -r $FPS -i $TMP -c:v libx264 $OUTPUT
rm IO/temp.yuv
