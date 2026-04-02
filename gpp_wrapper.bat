@echo off
REM MinGW C++ Compiler Wrapper for MuseScore
setlocal
setlocal enabledelayedexpansion

REM Real compiler path
set REAL_GXX=C:\Qt\Tools\mingw1310_64\bin\g++.exe

REM Filter out /wd flags and rebuild argument list  
set GXX_ARGS=
for %%A in (%*) do (
    set ARG=%%A
    if "!ARG:~0,3!" NEQ "/wd" (
        set "GXX_ARGS=!GXX_ARGS! %%A"
    )
)

REM Execute real compiler
%REAL_GXX% %GXX_ARGS%
exit /b %errorlevel%

