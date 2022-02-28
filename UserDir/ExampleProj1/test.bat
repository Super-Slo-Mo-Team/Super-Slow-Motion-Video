@echo off

SET target=%~dp0..\..\Source\PySrc\slowmotion.py --input akiyo_cif.y4m --output Outputs\akiyo_cif.mkv --FPS 25 --slowdown 4
SET environment=%~dp0..\..\PythonEnvironment\Scripts
%environment%\activate.bat && python %target% && deactivate