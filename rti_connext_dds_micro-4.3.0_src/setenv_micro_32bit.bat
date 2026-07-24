@echo off
:: Use setlocal to prevent unintended environment variable leakage in some contexts
:: but note that variables intended for the current session will remain.

:: 1. Define RTIMEHOME and remove the trailing backslash safely
set "RTIMEHOME=%~dp0"
if "%RTIMEHOME:~-1%"=="\" set "RTIMEHOME=%RTIMEHOME:~0,-1%"

:: 2. Set NDDSHOME (Used in the current session)
set "NDDSHOME=%RTIMEHOME%"

:: 4. Define Architecture-specific path
set "RTIMEARCH=i86lePEvs2017-MICROSAR4"

set "PATH=%PATH:C:\Program Files\CMake\bin;%RTIMEHOME%\lib\i86lePEvs2017;=%"
:: set "JREHOME=%RTIMEHOME%\rtiddsgen\jre\x64Win64"
set "RTIME_DIST=%RTIMEHOME%"

:: 6. Display the final configuration
echo ---------------------------------------------------
echo RTI DDS Environment Set Successfully (Current Session)
echo NDDSHOME : %NDDSHOME%
echo RTIMEHOME : %RTIMEHOME%
echo RTIMEARCH: %RTIMEARCH%
echo ---------------------------------------------------

@echo off