@echo off
setlocal

rem RTIMEHOME = current workspace directory (script location)
set "RTIMEHOME=%~dp0"
if "%RTIMEHOME:~-1%"=="\" set "RTIMEHOME=%RTIMEHOME:~0,-1%"

rem Required environment variables
set "OSEK_PATH=G:\My Drive\Documents\01_Project\mobilgene\mobilgene_h"
set "GHS_COMPILER_ROOT=C:\ghs\comp_201914"
if exist "%GHS_COMPILER_ROOT%\ccthumb.exe" (
    set "GHS_COMPILER_PATH=%GHS_COMPILER_ROOT%"
) else if exist "%GHS_COMPILER_ROOT%\bin\ccthumb.exe" (
    set "GHS_COMPILER_PATH=%GHS_COMPILER_ROOT%\bin"
) else (
    set "GHS_COMPILER_PATH=%GHS_COMPILER_ROOT%"
)
set "OSEK_PATH=%OSEK_PATH:\=/%"
set "GHS_COMPILER_PATH=%GHS_COMPILER_PATH:\=/%"

echo RTIMEHOME=%RTIMEHOME%
echo OSEK_PATH=%OSEK_PATH%
echo GHS_COMPILER_PATH=%GHS_COMPILER_PATH%

set "TARGET_PDIO=armv7emleElfghs2019.1.4_PDIO-Mobilgene"
set "TARGET_PDIOCERT=armv7emleElfghs2019.1.4_PDIOCERT-Mobilgene"

echo [1/4] Build Debug (%TARGET_PDIO%)
call "%RTIMEHOME%\resource\scripts\rtime-make.bat" --config Debug --build --delete --target %TARGET_PDIO% --name %TARGET_PDIO% -G "Ninja" -DRTIME_INCLUDE_AUTOSAR_eq_FALSE
set "RC_DEBUG_PDIO=%ERRORLEVEL%"

if not "%RC_DEBUG_PDIO%"=="0" (
    echo Debug build failed for %TARGET_PDIO% with error code %RC_DEBUG_PDIO%.
    exit /b %RC_DEBUG_PDIO%
)

call :PACKAGE_PSL_LIBS Debug %TARGET_PDIO% zd
if not "%ERRORLEVEL%"=="0" (
    echo Artifact packaging failed for Debug %TARGET_PDIO%.
    exit /b %ERRORLEVEL%
)

echo [2/4] Build Release (%TARGET_PDIO%)
call "%RTIMEHOME%\resource\scripts\rtime-make.bat" --config Release --build --delete --target %TARGET_PDIO% --name %TARGET_PDIO% -G "Ninja" -DRTIME_INCLUDE_AUTOSAR_eq_FALSE
set "RC_RELEASE_PDIO=%ERRORLEVEL%"

if not "%RC_RELEASE_PDIO%"=="0" (
    echo Release build failed for %TARGET_PDIO% with error code %RC_RELEASE_PDIO%.
    exit /b %RC_RELEASE_PDIO%
)

call :PACKAGE_PSL_LIBS Release %TARGET_PDIO% z
if not "%ERRORLEVEL%"=="0" (
    echo Artifact packaging failed for Release %TARGET_PDIO%.
    exit /b %ERRORLEVEL%
)

echo [3/4] Build Debug (%TARGET_PDIOCERT%)
call "%RTIMEHOME%\resource\scripts\rtime-make.bat" --config Debug --build --delete --target %TARGET_PDIOCERT% --name %TARGET_PDIOCERT% -G "Ninja" -DRTIME_INCLUDE_AUTOSAR_eq_FALSE
set "RC_DEBUG_PDIOCERT=%ERRORLEVEL%"

if not "%RC_DEBUG_PDIOCERT%"=="0" (
    echo Debug build failed for %TARGET_PDIOCERT% with error code %RC_DEBUG_PDIOCERT%.
    exit /b %RC_DEBUG_PDIOCERT%
)

call :PACKAGE_PSL_LIBS Debug %TARGET_PDIOCERT% zd
if not "%ERRORLEVEL%"=="0" (
    echo Artifact packaging failed for Debug %TARGET_PDIOCERT%.
    exit /b %ERRORLEVEL%
)

echo [4/4] Build Release (%TARGET_PDIOCERT%)
call "%RTIMEHOME%\resource\scripts\rtime-make.bat" --config Release --build --delete --target %TARGET_PDIOCERT% --name %TARGET_PDIOCERT% -G "Ninja" -DRTIME_INCLUDE_AUTOSAR_eq_FALSE
set "RC_RELEASE_PDIOCERT=%ERRORLEVEL%"

if not "%RC_RELEASE_PDIOCERT%"=="0" (
    echo Release build failed for %TARGET_PDIOCERT% with error code %RC_RELEASE_PDIOCERT%.
    exit /b %RC_RELEASE_PDIOCERT%
)

call :PACKAGE_PSL_LIBS Release %TARGET_PDIOCERT% z
if not "%ERRORLEVEL%"=="0" (
    echo Artifact packaging failed for Release %TARGET_PDIOCERT%.
    exit /b %ERRORLEVEL%
)

echo Debug and Release builds completed successfully for %TARGET_PDIO% and %TARGET_PDIOCERT%.
exit /b 0

:PACKAGE_PSL_LIBS
set "PKG_CONFIG=%~1"
set "PKG_TARGET=%~2"
set "PKG_SUFFIX=%~3"
set "PKG_SRC_DIR=%RTIMEHOME%\build\cmake\%PKG_CONFIG%\%PKG_TARGET%"
set "PKG_DST_DIR=%RTIMEHOME%\lib\%PKG_TARGET%"

if not exist "%PKG_DST_DIR%" mkdir "%PKG_DST_DIR%"

for %%F in (
    librti_me_rti_me_psl%PKG_SUFFIX%.a
    librti_me_ospsl%PKG_SUFFIX%.a
    librti_me_netiopsl%PKG_SUFFIX%.a
) do (
    if not exist "%PKG_SRC_DIR%\%%F" (
        echo Missing expected artifact: "%PKG_SRC_DIR%\%%F"
        exit /b 1
    )
    copy /Y "%PKG_SRC_DIR%\%%F" "%PKG_DST_DIR%\%%F" >nul
)

echo Packaged PSL libs to "%PKG_DST_DIR%"
exit /b 0
