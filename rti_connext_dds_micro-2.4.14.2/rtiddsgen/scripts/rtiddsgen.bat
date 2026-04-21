@REM
@REM FILE: rtiddsgen.bat - Wrapper script to invoke rtiddsgen
@REM
@REM (c) Copyright, Real-Time Innovations, 2005-2024.
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

set nddsgenDir=%parentDir%\..
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

set parseJrePath=NO
set excludeArg=NO
set pauseAtEnd=NO

set cmdLineArgs=

for %%A in (%*) do (

    if "!parseJrePath!" == "YES" (
        set parseJrePath=NO
        set jrePath=%%A
        set excludeArg=YES
    )

    if "%%~A" == "%argJre%" (
        set parseJrePath=YES
        set jreMode=CMDLINE
        set excludeArg=YES
    )

    if "%%~A" == "%argPauseAtEnd%" (
        set pauseAtEnd=YES
        set excludeArg=YES
    )

    REM Ensure that -micro is the first option
    if "%%~A" == "-micro" (
        set excludeArg=YES
    )

    if "!excludeArg!" == "NO" (
        set cmdLineArgs=!cmdLineArgs! %%A
    )

    set excludeArg=NO
)

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
