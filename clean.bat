@echo off

rd /s /q builds

if "%~1"=="" goto :eof

reload.bat "%~1"