@echo off
del /s CMakeCache.txt
del /s Makefile
del /s cmake_install.cmake
del /s *.sln
del /s *.vcxproj
del /s *.filters
rd /s /q CMakeFiles
rd /s /q nfs\CMakeFiles
rd /s /q nfsu\CMakeFiles
rd /s /q nfs\nfs.dir
rd /s /q nfsu\nfsu.dir

if "%~1"=="" goto :eof

reload.bat "%~1"