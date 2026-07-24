@echo off
setlocal EnableExtensions EnableDelayedExpansion

set "SCRIPT_DIR=%~dp0"
pushd "%SCRIPT_DIR%" >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Failed to switch to script directory: %SCRIPT_DIR%
    exit /b 90
)

set "MODE=all"
set "CONFIG=Debug"
set "VERIFY=verify"

set "HAS_MODE="
set "HAS_CONFIG="
set "HAS_VERIFY="

if /I "%~1"=="-h" goto :usage
if /I "%~1"=="--help" goto :usage
if /I "%~1"=="/h" goto :usage
if /I "%~1"=="/?" goto :usage

:parse_args
if "%~1"=="" goto :args_done

set "ARG=%~1"

if "%ARG%"=="MODE" (
    if "%~2"=="" (
        echo [ERROR] Missing MODE value.
        goto :usage_error
    )
    set "MODE=%~2"
    set "HAS_MODE=1"
    shift
) else if "%ARG%"=="CONFIG" (
    if "%~2"=="" (
        echo [ERROR] Missing CONFIG value.
        goto :usage_error
    )
    set "CONFIG=%~2"
    set "HAS_CONFIG=1"
    shift
) else if "%ARG%"=="VERIFY" (
    if "%~2"=="" (
        echo [ERROR] Missing VERIFY value.
        goto :usage_error
    )
    set "VERIFY=%~2"
    set "HAS_VERIFY=1"
    shift
) else if /I "%ARG:~0,5%"=="MODE=" (
    set "MODE=%ARG:~5%"
    set "HAS_MODE=1"
) else if /I "%ARG:~0,7%"=="CONFIG=" (
    set "CONFIG=%ARG:~7%"
    set "HAS_CONFIG=1"
) else if /I "%ARG:~0,7%"=="VERIFY=" (
    set "VERIFY=%ARG:~7%"
    set "HAS_VERIFY=1"
) else (
    if not defined HAS_MODE (
        set "MODE=%ARG%"
        set "HAS_MODE=1"
    ) else if not defined HAS_CONFIG (
        set "CONFIG=%ARG%"
        set "HAS_CONFIG=1"
    ) else if not defined HAS_VERIFY (
        set "VERIFY=%ARG%"
        set "HAS_VERIFY=1"
    ) else (
        echo [ERROR] Too many arguments: %ARG%
        goto :usage_error
    )
)

shift
goto :parse_args

:args_done

call :normalize_mode
call :normalize_config
call :normalize_verify

if errorlevel 1 goto :end_error

call :validate_mode
if errorlevel 1 goto :usage_error

call :validate_config
if errorlevel 1 goto :usage_error

call :validate_verify
if errorlevel 1 goto :usage_error

echo [INFO] MODE=%MODE% CONFIG=%CONFIG% VERIFY=%VERIFY%

if not defined OSEK_PATH (
    echo [ERROR] OSEK_PATH is not defined.
    goto :end_error
)
if not exist "%OSEK_PATH%" (
    echo [ERROR] OSEK_PATH does not exist: %OSEK_PATH%
    goto :end_error
)

if not exist "%SCRIPT_DIR%resource\scripts\rtime-make.bat" (
    echo [ERROR] Missing resource\scripts\rtime-make.bat in workspace root.
    goto :end_error
)

call :setup_env
if errorlevel 1 goto :end_error

if /I "%MODE%"=="all" goto :build_all
if /I "%MODE%"=="pil" goto :build_pil_mode
if /I "%MODE%"=="psl" goto :build_psl_mode
goto :usage_error

:build_all
call :build_target i86lePEvs2017
if errorlevel 1 goto :end_error
call :build_target i86lePEvs2017-MICROSAR4
if errorlevel 1 goto :end_error
goto :post_build

:build_pil_mode
call :build_target i86lePEvs2017
if errorlevel 1 goto :end_error
goto :post_build

:build_psl_mode
call :build_target i86lePEvs2017-MICROSAR4
if errorlevel 1 goto :end_error

:post_build

if /I "%VERIFY%"=="verify" goto :do_verify
echo [INFO] Verification skipped (VERIFY=noverify).
goto :build_success

:do_verify
call :run_verification
if errorlevel 1 goto :end_error

:build_success
echo [OK] build_micro4_vtt completed successfully.
popd >nul 2>&1
exit /b 0

:build_target
set "TARGET=%~1"
if "%TARGET%"=="" (
    echo [ERROR] Internal: empty target passed to build_target.
    exit /b 71
)

if /I "%TARGET%"=="i86lePEvs2017" set "RTIMEARCH=i86lePEvs2017"
if /I "%TARGET%"=="i86lePEvs2017-MICROSAR4" set "RTIMEARCH=i86lePEvs2017-MICROSAR4"

echo [INFO] Building target: %TARGET% (CONFIG=%CONFIG%)
call rtimemake --config %CONFIG% --build --target %TARGET% --name %TARGET% -G "Visual Studio 15 2017" -DRTIME_EXCLUDE_CPP_eq_TRUE -DRTI_BUILD_UNITTESTS_eq_FALSE
if errorlevel 1 (
    echo [ERROR] Build failed for target: %TARGET%
    exit /b 72
)

call :sync_output_libs %TARGET%
if errorlevel 1 (
    echo [ERROR] Failed to synchronize output archives for target: %TARGET%
    exit /b 73
)

exit /b 0

:setup_env
set "RTIMEHOME=%SCRIPT_DIR:~0,-1%"
set "NDDSHOME=%RTIMEHOME%"
set "RTIME_DIST=%RTIMEHOME%"
set "RTIMEARCH=i86lePEvs2017-MICROSAR4"

call :prepend_path "%RTIMEHOME%"
call :prepend_path "%RTIMEHOME%\resource\scripts"
call :prepend_path "%RTIMEHOME%\bin"

if exist "%ProgramFiles%\CMake\bin\cmake.exe" call :prepend_path "%ProgramFiles%\CMake\bin"
if exist "%ProgramFiles(x86)%\CMake\bin\cmake.exe" call :prepend_path "%ProgramFiles(x86)%\CMake\bin"

where rtimemake >nul 2>&1
if errorlevel 1 (
    echo [ERROR] rtimemake is not available after environment setup.
    exit /b 1
)

exit /b 0

:prepend_path
set "ENTRY=%~1"
if "%ENTRY%"=="" exit /b 0
echo ;%PATH%; | findstr /I /C:";%ENTRY%;" >nul
if errorlevel 1 set "PATH=%ENTRY%;%PATH%"
exit /b 0

:sync_output_libs
set "TARGET=%~1"
if "%TARGET%"=="" exit /b 1

set "BUILD_ARCHIVE_DIR=%SCRIPT_DIR%build\cmake\%CONFIG%\%TARGET%\%CONFIG%"
set "OUTPUT_LIB_DIR=%SCRIPT_DIR%lib\%TARGET%"

if not exist "%BUILD_ARCHIVE_DIR%" (
    echo [ERROR] Build archive directory not found: %BUILD_ARCHIVE_DIR%
    exit /b 1
)

if not exist "%OUTPUT_LIB_DIR%" mkdir "%OUTPUT_LIB_DIR%"

copy /Y "%BUILD_ARCHIVE_DIR%\*.a" "%OUTPUT_LIB_DIR%\" >nul 2>&1
copy /Y "%BUILD_ARCHIVE_DIR%\*.lib" "%OUTPUT_LIB_DIR%\" >nul 2>&1

set "ARCHIVE_COUNT=0"
for /f %%N in ('dir /b /a:-d "%OUTPUT_LIB_DIR%\*.a" "%OUTPUT_LIB_DIR%\*.lib" 2^>nul ^| find /c /v ""') do set "ARCHIVE_COUNT=%%N"
if "%ARCHIVE_COUNT%"=="0" (
    echo [ERROR] No archives were synchronized to %OUTPUT_LIB_DIR%
    exit /b 1
)

echo [INFO] Synchronized %ARCHIVE_COUNT% archive(s) to %OUTPUT_LIB_DIR%
exit /b 0

:run_verification
if /I "%MODE%"=="all" goto :verify_all
if /I "%MODE%"=="pil" goto :verify_pil_only
if /I "%MODE%"=="psl" goto :verify_psl_only

exit /b 83

:verify_all
call :verify_pil
if errorlevel 1 exit /b 81
call :verify_psl
if errorlevel 1 exit /b 82
exit /b 0

:verify_pil_only
call :verify_pil
if errorlevel 1 exit /b 81
exit /b 0

:verify_psl_only
call :verify_psl
if errorlevel 1 exit /b 82
exit /b 0

:verify_pil
set "PIL_DIR=%SCRIPT_DIR%lib\i86lePEvs2017"
if not exist "%PIL_DIR%" (
    echo [ERROR] PIL output directory not found: %PIL_DIR%
    exit /b 1
)

set "PIL_COUNT=0"
for /f %%N in ('dir /b /a:-d "%PIL_DIR%\*.a" "%PIL_DIR%\*.lib" 2^>nul ^| find /c /v ""') do set "PIL_COUNT=%%N"
if "%PIL_COUNT%"=="0" (
    echo [ERROR] No archives found in PIL output directory: %PIL_DIR%
    exit /b 1
)

echo [OK] PIL verification passed. Found %PIL_COUNT% archive(s) in %PIL_DIR%
exit /b 0

:verify_psl
set "PSL_DIR=%SCRIPT_DIR%lib\i86lePEvs2017-MICROSAR4"
set "PSL_VERIFY=%SCRIPT_DIR%playbooks\microsar-pil-psl\verify_psl_symbols.ps1"

if not exist "%PSL_DIR%" (
    echo [ERROR] PSL output directory not found: %PSL_DIR%
    exit /b 1
)

set "PSL_COUNT=0"
for /f %%N in ('dir /b /a:-d "%PSL_DIR%\*.a" "%PSL_DIR%\*.lib" 2^>nul ^| find /c /v ""') do set "PSL_COUNT=%%N"
if "%PSL_COUNT%"=="0" (
    echo [ERROR] No archives found in PSL output directory: %PSL_DIR%
    exit /b 1
)

if not exist "%PSL_VERIFY%" (
    echo [ERROR] Missing verification script: %PSL_VERIFY%
    exit /b 1
)

echo [INFO] Running PSL symbol verification script.
powershell -NoProfile -ExecutionPolicy Bypass -File "%PSL_VERIFY%"
if errorlevel 1 (
    echo [ERROR] PSL symbol verification failed.
    exit /b 1
)

echo [OK] PSL verification passed. Found %PSL_COUNT% archive(s) in %PSL_DIR%
exit /b 0

:normalize_mode
if /I "%MODE%"=="ALL" set "MODE=all"
if /I "%MODE%"=="PIL" set "MODE=pil"
if /I "%MODE%"=="PSL" set "MODE=psl"
exit /b 0

:normalize_config
if /I "%CONFIG%"=="DEBUG" set "CONFIG=Debug"
if /I "%CONFIG%"=="RELEASE" set "CONFIG=Release"
exit /b 0

:normalize_verify
if /I "%VERIFY%"=="VERIFY" set "VERIFY=verify"
if /I "%VERIFY%"=="NOVERIFY" set "VERIFY=noverify"
exit /b 0

:validate_mode
if /I "%MODE%"=="all" exit /b 0
if /I "%MODE%"=="pil" exit /b 0
if /I "%MODE%"=="psl" exit /b 0
echo [ERROR] Invalid MODE: %MODE%
exit /b 1

:validate_config
if /I "%CONFIG%"=="Debug" exit /b 0
if /I "%CONFIG%"=="Release" exit /b 0
echo [ERROR] Invalid CONFIG: %CONFIG%
exit /b 1

:validate_verify
if /I "%VERIFY%"=="verify" exit /b 0
if /I "%VERIFY%"=="noverify" exit /b 0
echo [ERROR] Invalid VERIFY: %VERIFY%
exit /b 1

:usage
@echo Usage:
@echo   build_micro4_vtt.bat [MODE] [CONFIG] [VERIFY]
@echo   build_micro4_vtt.bat MODE^=all^|pil^|psl CONFIG^=Debug^|Release VERIFY^=verify^|noverify
@echo.
@echo Examples:
@echo   build_micro4_vtt.bat
@echo   build_micro4_vtt.bat pil Debug verify
@echo   build_micro4_vtt.bat MODE^=psl CONFIG^=Release VERIFY^=noverify
popd >nul 2>&1
exit /b 0

:usage_error
call :usage
exit /b 2

:end_error
popd >nul 2>&1
exit /b 1