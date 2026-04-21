@echo off
setlocal EnableDelayedExpansion
set BUILD_TYPE=Debug
set CMAKE_OPTIONS=
set CBUILD_OPTIONS=
set CMAKE_G=
set SCRIPT_NAME=%~nx0
set ARCH=
set GUI=
set DO_BUILD=
set DO_DELETE=
set NAME=
set CMAKE_PATH=
set GIT_BRANCH=
set BRANCH=
set DO_HELP=0
set NO_TEST=0
set TOOLCHAIN_FILE=
set SYSTEM_FILE=
REM Full script path
set SDIR=%~dp0
set ENABLE_QUACK=

if EXIST "%SDIR%\..\..\resource\scripts\rtime-make.bat" (
    set DEVTREE=0
    set RESOURCE_DIR=%SDIR%\..\..\resource\cmake
    set MODULE_PATH=!RESOURCE_DIR:\=\\!
) else (
   if EXIST "%SDIR%\..\..\resource.1.0\scripts\rtime-make.bat" (
      set DEVTREE=1
      set RESOURCE_DIR=%SDIR%..\..\resource.1.0\cmake
   ) else (
       call :print_error "Unknown source-tree"
       goto :eof
   )

   set MODULE_PATH=!RESOURCE_DIR:\=\\!
)

:parse_args
if not "%1"=="" (
    set found_arg=0
    if "%1" == "--help" (
        set DO_HELP=1
        set found_arg=1 
    )
    if "%1" == "-h" (
        call :help
        goto :eof
    )
    if "%1" == "--no-test" (
       if "%DEVTREE%" == "1" (
           set NO_TEST=1
        ) else (
           call :print_error "--no-test can only be used for the development tree"
           goto :eof
        )
    )
    if "%1" == "--git-branch" (
       if "%DEVTREE%" == "1" (
           set GIT_BRANCH=-DRTI_BUILD_BRANCH=%2
           set BRANCH=%2
           shift
           set found_arg=1 
        ) else (
           call :print_error "--git-branch can only be used for the development tree"
           goto :eof
        )
    )
    if "%1" == "--enable-quack" (
       if "%DEVTREE%" == "1" (
           set ENABLE_QUACK=-DRTIME_UTEST_QUACK_ENABLE=true
           set found_arg=1
        ) else (
           call :print_error "--enable-quack can only be used for the development tree"
           goto :eof
        )
    )
    if "%1" == "--cmake-path" (
        set CMAKE_PATH=%2
        shift
        set found_arg=1
    )
    if "%1" == "--list" (
        call :listarch
        goto :eof)
    if "%1" == "-l" (
        call :listarch
        goto :eof)
    if "%1" == "-G" (
        set CMAKE_G=%2
        shift
        set found_arg=1
    )
    if "%1" == "--source-dir" (
        set SOURCE_DIR=%2
        shift
        set found_arg=1
    )
    if "%1" == "--srcdir" (
        set SOURCE_DIR=%2
        shift
        set found_arg=1
    )
    if "%1" == "--gui" (
        set GUI=yes
        set found_arg=1
    )
    if "%1" == "--config" (
        set BUILD_TYPE=%2
        shift
        set found_arg=1
    )
    if "%1" == "--build" (
        set DO_BUILD=yes
        set found_arg=1
    )
    if "%1" == "--dry-run" (
        set CMD_ECHO=echo
        set found_arg=1
    )
    if "%1" == "--tcfile" (
        set TOOLCHAIN_FILE=%2
        shift
        set found_arg=1
    )
    if "%1" == "--sysfile" (
        set SYSTEM_FILE=%2
        shift
        set found_arg=1
    )
    if "%1" == "--target" (
        set ARCH=%2
        shift
        set found_arg=1
    )
    if "%1" == "--name" (
        set NAME=%2
        shift
        set found_arg=1
    )
    if "%1" == "--delete" (
        set DO_DELETE=yes
        set found_arg=1
    )
    if "%1" == "--cmake-target" (
        :loop
            set NEXT_ARG=%2
            if "!NEXT_ARG!" == "" (goto :end_loop)
            echo !NEXT_ARG! | findstr /R "^-" >nul 2>&1
            if not errorlevel 1 (goto :end_loop)
            set CBUILD_OPTIONS=!CBUILD_OPTIONS! --target !NEXT_ARG!
            shift
            goto :loop
        :end_loop
        set found_arg=1
    )
    if "%1" == "--" (
:rest_args
        if not "%1" == "" (
            shift
            set CBUILD_OPTIONS=!CBUILD_OPTIONS! %1
            goto :rest_args
        )
        set found_arg=1
    )

    if "!found_arg!" == "0" (
        set CMAKE_OPTIONS=!CMAKE_OPTIONS! %1
    )

    shift
    goto :parse_args
)

if not "x%ARCH%" == "x" (
    if not "x%TOOLCHAIN_FILE%" == "x" (
        call :print_error Only one of --target or --toolchain-file can be specified.
        exit /b
    )
)

if NOT "%GIT_BRANCH%_" == "_" (
    set GIT_BRANCH=%GIT_BRANCH:/=\%
    set BRANCH=%BRANCH:/=\%
)

if NOT "%CMAKE_OPTIONS%_" == "_" (
    set CMAKE_OPTIONS=!CMAKE_OPTIONS:_eq_==!
)

if NOT "%CBUILD_OPTIONS%_" == "_" (
    set CBUILD_OPTIONS=!CBUILD_OPTIONS:_eq_==!
)

if %CMAKE_PATH%_ == _ (
    set CMAKE_BIN=cmake
    set CMAKE_GUI=cmake-gui
) else (
    set CMAKE_BIN=%CMAKE_PATH%/bin/cmake
    set CMAKE_GUI=%CMAKE_PATH%/bin/cmake-gui
)

set CMAKE_BIN=%CMAKE_BIN:"=%
set CMAKE_GUI=%CMAKE_GUI:"=%

if "%DO_HELP%" == "1" (
    if "%ARCH%" == "" (
        call :help
        goto :eof
    ) else (
        "%CMAKE_BIN%" -DTARGET_HELP:BOOLEAN=1 -DCMAKE_MODULE_PATH="%MODULE_PATH%"\\architectures -DRTIME_TARGET_NAME=%NAME% -P "%RESOURCE_DIR%/architectures/%ARCH%.tc"
        goto :eof
    )
    exit /b
)

if "x%ARCH%" == "x" (
    if "x%TOOLCHAIN_FILE%" == "x" (
        call :print_error The target must be specified with -t or --target or 
        call :print_error a toolchain file must be specified with --toolchain-file
        call :print_error 
        call :print_error --target target
        call :print_error 
        call :print_error Use --list for available targets
        call :print_error 
        call :print_error 
        call :print_error --toolchain-file file
        exit /b
    ) else (
        if not exist "%TOOLCHAIN_FILE%" (
            call :print_error The toolchain-file %TOOLCHAIN_FILE% does not exist.
            exit /b
        ) else (
            set ARCH=%TOOLCHAIN_FILE%
            set TOOLCHAIN_FILE=-DCMAKE_TOOLCHAIN_FILE="%TOOLCHAIN_FILE%"
        )
        if "x%NAME%" == "x" (
            call :print_error The name of the target must be specified with --name \<name\> when --toolchain-file is used.
            exit /b
        )
    )
) else (
    if NOT "%ARCH%" == "self" (
        if not exist "%RESOURCE_DIR%\architectures\%ARCH%.tc" (
            call :print_error "Unknown target %ARCH% (%RESOURCE_DIR%\architectures\%ARCH%.tc) listed. use --list for a list of available targets."
            exit /b
        ) else (
            set TOOLCHAIN_FILE=-DCMAKE_TOOLCHAIN_FILE="%RESOURCE_DIR%\architectures\%ARCH%.tc"
        )   
    )
    
    if "x%NAME%" == "x" (
        if NOT "%ARCH%" == "self" (
            set NAME=%ARCH%
        ) else (
            call :print_error "The name of the target must be specified with --name <name> when --target self is used"
            exit /b
        )
    )
)

if NOT "%SYSTEM_FILE%"_ == ""_ (
    if NOT "%SYSTEM_FILE%" == "" (
        if not exist "%SYSTEM_FILE%" (
            call :print_error The system file "%SYSTEM_FILE%" does not exist.
            exit /b
        )
     )
    set SYSTEM_FILE=-DRTIME_SYSTEM_FILE="%SYSTEM_FILE%"
)

if "x%SOURCE_DIR%" == "x" (
    set SOURCE_DIR=%SDIR%..\..
    if "%DEVTREE%" == "1" (
        set CMAKE_ROOT=%SDIR%..\..\..\build\%BRANCH%\cmake\cmake\%BUILD_TYPE%\%NAME%
    ) else (
        set CMAKE_ROOT=%SDIR%..\..\build\cmake\%BUILD_TYPE%\%NAME%
    )

    if "%NO_TEST%" == "1" (
        set CMAKE_OPTIONS=!CMAKE_OPTIONS! -DRTI_BUILD_UNITTESTS=FALSE
    ) else (
        set CMAKE_OPTIONS=!CMAKE_OPTIONS! -DRTI_BUILD_UNITTESTS=TRUE
    )

    set CMAKE_OPTIONS=!CMAKE_OPTIONS! -DRTIME_TARGET=%ARCH%
) else (
    set CMAKE_ROOT=%SOURCE_DIR%\build\cmake\%BUILD_TYPE%\%NAME%
)

set CMAKE_ROOT=%CMAKE_ROOT:"=%
set CMAKE_ROOT="%CMAKE_ROOT%"

set SOURCE_DIR=%SOURCE_DIR:"=%
set SOURCE_DIR="%SOURCE_DIR%"

if not exist %SOURCE_DIR% (
    echo %SOURCE_DIR% does not exist
    exit /b
)

if "%DO_DELETE%" == "yes" (
    if exist %CMAKE_ROOT% (
        rmdir /Q /S %CMAKE_ROOT%
    )
)

if not exist %CMAKE_ROOT% (
    mkdir %CMAKE_ROOT%
)

if %CMAKE_G%_ == _ (
    %CMD_ECHO% "%CMAKE_BIN%" -DCMAKE_MODULE_PATH="%MODULE_PATH%"\\architectures %TOOLCHAIN_FILE% %SYSTEM_FILE% %CMAKE_OPTIONS% -DCMAKE_BUILD_TYPE=%BUILD_TYPE% %GIT_BRANCH% -DRTIME_TARGET_NAME=%NAME% -B%CMAKE_ROOT% %ENABLE_QUACK% -H%SOURCE_DIR%
) else (
    %CMD_ECHO% "%CMAKE_BIN%" -DCMAKE_MODULE_PATH="%MODULE_PATH%"\\architectures %TOOLCHAIN_FILE%  %SYSTEM_FILE% -G %CMAKE_G% %CMAKE_OPTIONS% -DCMAKE_BUILD_TYPE=%BUILD_TYPE% %GIT_BRANCH% %ENABLE_QUACK% -DRTIME_TARGET_NAME=%NAME% -B%CMAKE_ROOT% -H%SOURCE_DIR%
)
if not defined CMD_ECHO (
    if %ERRORLEVEL% NEQ 0 (
        echo ERROR: CMake configure step failed
        exit /b %ERRORLEVEL%
    )
)

if "%GUI%" == "yes" (
    %CMD_ECHO% "%CMAKE_GUI%" -B%CMAKE_ROOT% -H%SOURCE_DIR%
)

if "%DO_BUILD%" == "yes" (
    %CMD_ECHO% "%CMAKE_BIN%" --build %CMAKE_ROOT% --config %BUILD_TYPE% %CBUILD_OPTIONS%
    if not defined CMD_ECHO (
        if %ERRORLEVEL% NEQ 0 (
            echo ERROR: CMake build step failed
            exit /b %ERRORLEVEL%
        )
    )
)

exit /b

:print_error
echo ERROR: %*
exit /b

:print_warning
echo WARN: %*
exit /b

:print_info
echo INFO: %*
exit /b

:print_debug
echo DEBUG: %*
exit /b

:listarch
    echo.
    echo The following architectures are available:
    echo.
    for %%f in ("%RESOURCE_DIR%"\architectures\*.tc) do echo %%~nf
exit /b

:help

echo  NAME
echo.
echo  %SCRIPT_NAME%
echo.
echo  SYNOPSIS
echo.
echo  %SCRIPT_NAME% [options] [ -- cmake build options ] 
echo.        
echo  Options:
echo  -h ^| --help        Print this help and exit.
echo.
echo  --gui               Launch the CMake GUI.
echo.
echo  --build             Build the target. 
echo.
echo  --list              List available targets and exit.
echo.
echo  --config            Which type of build (Default: Debug):
echo                        - Debug   - Create a debug build.
echo                        - Release - Create a release build.
echo.
echo   --enable-quack     Enable Quack test bundles creation
echo.
echo  --delete            Delete existing, if any, target specific build directories
echo.
echo  --target <target>   Which target to compile for
echo                      The special target self is a self-hosted build with no
echo                      configuration.
echo.
echo  --name <name>       The name of the target. This is used as part of the 
echo                      directory paths, as well as compiled into the target. If
echo                      --name is not specified, the --target is used as the name
echo.      
echo  -G "generator"      Which CMake generator to use. Use cmake --help for a list
echo                      of available generators.
echo.  
echo  --no-test           Do not build the unit-tests (only valid in the devtree)
echo.
echo  --git-branch        The Git branch to build (only valid in the devtree)
echo.
echo  --cmake-path        Path to cmake installation
echo.
echo  --source-dir        Path to CMakeLists.txt
echo.
echo  --srcdir            Path to CMakeLists.txt
echo.
echo  --dry-run           Only print the cmake commands that will be executed
echo.
echo  --tcfile file       Use the specified tool-chain file directly, ignoring
echo                      --target
echo.
echo  --sysfile file      Include this file as the system-file to modify flags
echo                      set by the toolchain (either with --target or 
echo                      --tcfile).
echo.
echo  --cmake-target      Use the specified space separated list as the cmake
echo                      targets.
echo.
echo  Unknown arguments before -- are concatenated and passed to cmake.
echo  All arguments following -- are passed to cmake build.
echo.
echo  DESCRIPTION
echo.  
echo  %SCRIPT_NAME% is used to create architecture specific build files for 
echo  RTI Connext Micro and can optionally invoke the build.
echo.
echo  EXAMPLES
echo.
echo  Build Debug version on Windows, self hosted:
echo.  
echo  %SCRIPT_NAME% --target Windows --name i86Win32VS2010 --config Debug \
echo        --delete -G "Visual Studio 10 2010" --build
echo.
echo  Note the -- . The -- is used to indicate that all the following 
echo  arguments are to be passed to cmake build. The -- -j 8 tells cmake 
echo  build to pass -j 8 to ninja.

exit /b
