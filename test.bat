@echo off
cd C:\Users\Administrator\source\repos\Super-Slo-Mo-Team\Super-Slow-Motion-Video\out\build\x64-Release
echo Beginning Slow Motion Generation...
SET target=.\Super_Slow_Motion_Video.exe %1 out_%2X.mp4 %2 25
%target%
