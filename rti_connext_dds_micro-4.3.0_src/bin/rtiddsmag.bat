@REM =========================================================================
@REM (c) 2005-2014 Copyright, Real-Time Innovations. All rights reserved.
@REM
@REM
@REM No duplications, whole or partial, manual or electronic, may be made
@REM without express written permission.  Any such copies, or
@REM revisions thereof, must display this notice unaltered.
@REM This code contains trade secrets of Real-Time Innovations, Inc.
@REM =========================================================================

@echo off
setlocal EnableDelayedExpansion

set dir=%~dp0

set parentDir_strip=%dir:~0,-1%

for %%I in ("%parentDir_strip%") do set "base_dir=%%~dpI"
for %%F in ("%parentDir_strip%") do set last_dir_name=%%~nxF

if /i "%last_dir_name%"=="bin" (
    set dir=%base_dir%rtiddsmag\scripts\
)

set appName=rtiddsmag
set scriptVersion=

set argJre=-jre
set invalidJrePath=some/made/up/path/that/likely/wont/exist
set jrePath=%invalidJrePath%

set jreMode=PATH
REM PATH            java from PATH
REM SHIPPED         RTI-shipped jre
REM JAVAHOME        JAVA_HOME
REM JREHOME         JREHOME
REM CMDLINE         -jre

set shipJre64=%dir%\..\..\resource\app\jre\x64Win64
set shipJre32=%dir%\..\..\resource\app\jre\i86Win32

set proShipJre64=%dir%\..\..\..\resource\app\jre\x64Win64
set proShipJre32=%dir%\..\..\..\resource\app\jre\i86Win32

:parseArgs

set parseJrePath=NO
set excludeArg=NO

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

set javaBin=%javaBin:"=%

REM echo Using %javaBin% from %jreMode%

set "appSupportDir=%dir%..\resource"
set "rtiddsmagJar=%dir%..\class\rtiddsmag.jar"
set "javaPrefsWarning=--add-opens=java.base/java.lang=ALL-UNNAMED"

if "%jreMode%" == "PATH" (
    where /q "%javaBin%"
    if errorlevel 1 (
        echo Cannot find or execute '%javaBin%'. Be sure Java is installed and available on PATH.
        exit /b 2
    )
) else (
    if not exist "%javaBin%.exe" if not exist "%javaBin%" (
        echo Cannot find or execute '%javaBin%'. Be sure to specify the correct JRE to use.
        exit /b 2
    )
)

set "javaVersion="
set "javaMajorVersion="
for /f "tokens=3" %%j in ('%javaBin% -version 2^>^&1 ^| findstr /i "version"') do set "javaVersion=%%~j"
for /f "tokens=1 delims=." %%j in ("!javaVersion!") do set "javaMajorVersion=%%j"
if defined javaMajorVersion if !javaMajorVersion! NEQ 17 (
    @echo on
    echo "warning: Micro Application Generator has not been tested with Java !javaMajorVersion!. Unexpected behavior may occur."
    @echo off
)

"%javaBin%" -DMAG_RESOURCE_DIR="%appSupportDir%" -DSCHEMA_DIR="%appSupportDir%\schema"^
    %javaPrefsWarning% -jar "%rtiddsmagJar%" %cmdLineArgs%

endlocal
