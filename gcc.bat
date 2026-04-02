@echo off
REM MinGW C Compiler Wrapper for MuseScore
setlocal
setlocal enabledelayedexpansion

REM Real compiler path
set REAL_GCC=C:\Qt\Tools\mingw1310_64\bin\gcc.exe

REM Filter out /wd flags and rebuild argument list
set GCC_ARGS=
for %%A in (%*) do (
    set ARG=%%A
    if "!ARG:~0,3!" NEQ "/wd" (
        set "GCC_ARGS=!GCC_ARGS! %%A"
    )
)

REM Execute real compiler
%REAL_GCC% %GCC_ARGS%
exit /b %errorlevel%


