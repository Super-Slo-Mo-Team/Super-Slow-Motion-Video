#!/bin/bash

cd build
./Super_Slow_Motion_Video ../IO/input.y4m ../IO/output.y4m 4 30
cd ..

cat build/tmp/yuv/*.yuv > IO/temp.yuv
ffmpeg -f rawvideo -pix_fmt yuv420p -s:v 352x288 -r 30 -i IO/temp.yuv -c:v libx264 IO/res.mp4
rm IO/temp.yuv
