@echo off
cd C:\Users\Administrator\source\repos\Super-Slo-Mo-Team\Super-Slow-Motion-Video\out\build\x64-Release
::set incoming=%1
::set name=%incoming:~0,-4%
echo Beginning Slow Motion Generation...
SET target=.\Super_Slow_Motion_Video.exe ..\%1 out.mp4 %2 25
::echo %target%
%target%
