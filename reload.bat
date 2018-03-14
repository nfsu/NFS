@echo off

if "%~1"=="" goto :reg

cmake . -G "%~1"
goto :eof

:reg
cmake .