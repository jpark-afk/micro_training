@echo off
:: Use setlocal to prevent unintended environment variable leakage in some contexts
:: but note that variables intended for the current session will remain.

:: 1. Define RTIMEHOME and remove the trailing backslash safely
set "RTIMEHOME=%~dp0"
if "%RTIMEHOME:~-1%"=="\" set "RTIMEHOME=%RTIMEHOME:~0,-1%"

:: 2. Set NDDSHOME (Used in the current session)
set "NDDSHOME=%RTIMEHOME%"

:: 3. Permanently set NDDSHOME for the user (setx)
:: Using double quotes carefully to avoid trailing quote bugs in Windows
:: setx NDDSHOME "%RTIMEHOME%" >nul

:: 4. Define Architecture-specific path
set "RTIMEARCH=x86_64lePEvs2017-Win10"

:: 5. Update PATH only if the directory is not already present
:: This prevents the PATH variable from growing indefinitely and hitting the length limit
:: echo %PATH% | findstr /C:"%RTIMEHOME%\rtiddsgen\scripts" >nul
:: if errorlevel 1 (
::     set "PATH=%RTIMEHOME%\rtiddsgen\scripts;%RTIMEHOME%\lib\%RTIMEARCH%;%PATH%"
:: )

:: 6. Display the final configuration
echo ---------------------------------------------------
echo RTI DDS Environment Set Successfully (Current Session)
echo NDDSHOME : %NDDSHOME%
echo RTIMEHOME : %RTIMEHOME%
echo RTIMEARCH: %RTIMEARCH%
echo ---------------------------------------------------

@echo off