@REM
@REM FILE: rtiddsgen.bat - Wrapper script to invoke rtiddsgen
@REM
@REM (c) Copyright, Real-Time Innovations, 2005-2025.
@REM 
@REM All rights reserved.
@REM
@REM No duplications, whole or partial, manual or electronic, may be made
@REM without express written permission.  Any such copies, or
@REM revisions thereof, must display this notice unaltered.
@REM This code contains trade secrets of Real-Time Innovations, Inc.
@REM
@REM =====================================================================

@echo off

setlocal enabledelayedexpansion

set parentDir=%~dp0
set me=%~n0
set parentDir_strip=%parentDir:~0,-1%

rem Use for loop to strip off the last folder
for %%I in ("%parentDir_strip%") do set "nddsgenDir=%%~dpI"

rem Get the parent directory name (e.g., "bin")
for %%F in ("%parentDir_strip%") do set last_dir_name=%%~nxF

if /i "%last_dir_name%"=="bin" (
    set nddsgenDir=%nddsgenDir%\rtiddsgen
)

set resourceDir=%nddsgenDir%\resource\rtiddsgen
set rtiddsgenJar=%nddsgenDir%\class\rtiddsgen2.jar

set argJre=-jre
set argPauseAtEnd=-pauseAtEnd

set invalidJrePath=some/made/up/path/that/likely/wont/exist
set jrePath=%invalidJrePath%

set jreMode=PATH
REM PATH            java from PATH
REM SHIPPED         RTI-shipped jre
REM JAVAHOME        JAVA_HOME
REM JREHOME         JREHOME
REM CMDLINE         -jre

set shipJre64=%nddsgenDir%\jre\x64Win64
set shipJre32=%nddsgenDir%\jre\i86Win32

set proShipJre64=%nddsgenDir%\..\..\resource\app\jre\x64Win64
set proShipJre32=%nddsgenDir%\..\..\resource\app\jre\i86Win32

:parseArgs

set pauseAtEnd=NO
set cmdLineArgs=

:parseArgsLoop
if "%~1"=="" goto :afterParse

REM Handle -jre by consuming the current flag AND the next argument
if /i "%~1"=="%argJre%" (
    set "jrePath=%~2"
    set jreMode=CMDLINE
    shift
    shift
    goto :parseArgsLoop
)

if /i "%~1"=="%argPauseAtEnd%" (
    set pauseAtEnd=YES
    shift
    goto :parseArgsLoop
)

REM Add the -micro flag back to the command line arguments later
if /i "%~1"=="-micro" (
    shift
    goto :parseArgsLoop
)

REM Accumulate arguments.
set "arg=%~1"
if "!arg:~-1!"=="\" (
    set "cmdLineArgs=!cmdLineArgs! "!arg!\""
) else (
    set "cmdLineArgs=!cmdLineArgs! "!arg!""
)
shift
goto :parseArgsLoop

:afterParse

if "%jreMode%" == "CMDLINE" (
    goto :runJava
)

if "%jrePath%" == "%invalidJrePath%" (
    if DEFINED JREHOME (
        set jreMode=JREHOME
        set jrePath="%JREHOME%"
        goto :runJava
    )

    if DEFINED JAVA_HOME (
        set jreMode=JAVAHOME
        set jrePath="%JAVA_HOME%"
        goto :runJava
    )

    if EXIST "%shipJre64%" (
        set jrePath="%shipJre64%"
        set jreMode=SHIPPED
        goto :runJava
    )

    if EXIST "%shipJre32%" (
        set jrePath="%shipJre32%"
        set jreMode=SHIPPED
        goto :runJava
    )

    if EXIST "%proShipJre64%" (
        set jrePath="%proShipJre64%"
        set jreMode=SHIPPED
        goto :runJava
    )

    if EXIST "%proShipJre32%" (
        set jrePath="%proShipJre32%"
        set jreMode=SHIPPED
        goto :runJava
    )

    set jreMode=PATH
    set jrePath=
    goto :runJava

)

:runJava

if "%jreMode%" == "PATH" (
    set javaBin="java"
) else (
    set javaBin=%jrePath%\bin\java
)

REM echo Using %javaBin% from %jreMode%

set javaBin=%javaBin:"=%

"%javaBin%" -DNDDS_RESOURCE_DIR="%resourceDir%" -jar "%rtiddsgenJar%" -micro %cmdLineArgs%

if "%pauseAtEnd%"=="YES" (
    pause
)

endlocal
