@echo off
REM MinGW Wrapper - Filters out MSVC warning flags

setlocal enabledelayedexpansion

REM Get the real MinGW compiler path
set GCC_BIN=C:\Qt\Tools\mingw1310_64\bin\g++.exe

REM Build arguments, filtering out /wd flags
set ARGS=
for %%A in (%*) do (
    set ARG=%%A
    if "!ARG:~0,3!" NEQ "/wd" (
        set ARGS=!ARGS! %%A
    )
)

REM Call the real compiler with filtered arguments
%GCC_BIN% %ARGS%
exit /b %errorlevel%
