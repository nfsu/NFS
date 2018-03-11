@echo off

>NUL chcp 65001

if "%1"=="" goto :error
if "%2"=="" goto :error1

if exist %1 goto :error2
if not exist %2 goto :error1

mkdir "%1"
xcopy /y /F "%2/CMakeLists.child.txt" "%1/CMakeListse.txt"*
powershell -Command "(gc "%1/CMakeListse.txt") -replace '<PROJECT_NAME>', '%1' | Out-File "%1/CMakeListse.txt""
powershell -Command "Get-Content "%1/CMakeListse.txt" | Set-Content -Encoding utf8 "%1/CMakeLists.txt""
del /s "%1/../CMakeListse.txt"
mkdir "%1/include"
mkdir "%1/src"

echo #pragma once >> "%1/include/main.h"

echo #include "main.h" >> "%1/src/main.cpp"
echo int main(){ >> "%1/src/main.cpp"
echo 	return 0; >> "%1/src/main.cpp"
echo } >> "%1/src/main.cpp"

echo add_subdirectory(%1) >> CMakeLists.txt

reload

goto :eof

:error
	echo Please add a project name (no spaces and lowercase)
	echo Usage: addproject SomeTest nfs
	pause
	goto :eof
	
:error1
	echo Please add an existing project to inherrit from (no spaces and lowercase)
	echo Usage: addproject SomeTest nfs
	pause
	goto :eof
	
:error2
	echo That folder already exists
	pause
	goto :eof