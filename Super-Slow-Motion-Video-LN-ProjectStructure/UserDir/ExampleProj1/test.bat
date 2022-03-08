@echo off
cd Super-Slow-Motion-Video-LN-ProjectStructure\UserDir\ExampleProj1 
echo Beginning Slow Motion Generation...
SET target=%~dp0....\Source\PySrc\slowmotion.py --input %1 --output Outputs\Output.mkv --FPS 25 --slowdown %2 
SET environment=%~dp0....\PythonEnvironment\Scripts
%environment%\activate.bat && python %target% && deactivate

:: right now this just takes two arguements, the input video(%1) and the slowdown amount (%2)
:: im just naming the output video as "Output.mkv" to keep things simple, and this output vvideo will be written to the Output\ directory
:: Should call it with  "/k "%CD%\Super-Slow-Motion-Video-LN-ProjectStructure\UserDir\ExampleProj1\runWithArguements.bat $input video$ $slowdown$""