@echo off
setlocal enabledelayedexpansion

echo ========================================
echo  TI Sysconfig Prebuild
echo ========================================
echo.

:: Determine paths
set "SCRIPT_DIR=%~dp0"
for %%a in ("%SCRIPT_DIR%..") do set "PROJ_ROOT=%%~fa"

:: Read syscfg.ini
set "INI_FILE=%PROJ_ROOT%\scripts\config.ini"
if not exist "%INI_FILE%" (
    echo ERROR: config.ini not found at %INI_FILE%
    echo Run configure.bat first.
    pause
    exit /b 1
)

for /f "usebackq tokens=2 delims== " %%a in (`findstr /c:"SYSCFG_ROOT" "%INI_FILE%"`) do set "SYSCFG_ROOT=%%a"
for /f "usebackq tokens=2 delims== " %%a in (`findstr /c:"TI_SDK_ROOT" "%INI_FILE%"`) do set "SDK_ROOT=%%a"
for /f "usebackq tokens=2 delims== " %%a in (`findstr /c:"PROJECT_NAME" "%INI_FILE%"`) do set "PROJECT_NAME=%%a"

:: Validate
if "%SYSCFG_ROOT%"=="" (
    echo ERROR: SYSCFG_ROOT not found in syscfg.ini
    pause
    exit /b 1
)
if "%SDK_ROOT%"=="" (
    echo ERROR: TI_SDK_ROOT not found in syscfg.ini
    pause
    exit /b 1
)
if "%PROJECT_NAME%"=="" (
    echo ERROR: PROJECT_NAME not found in syscfg.ini
    pause
    exit /b 1
)

echo   Project:       %PROJECT_NAME%
echo   SysConfig:     %SYSCFG_ROOT%
echo   SDK:           %SDK_ROOT%
echo.

:: Find the .syscfg file
set "SYSCFG_FILE="
for %%f in ("%SCRIPT_DIR%*.syscfg") do set "SYSCFG_FILE=%%f"
if "%SYSCFG_FILE%"=="" (
    echo ERROR: No .syscfg file found in %PROJ_ROOT%
    pause
    exit /b 1
)
echo   .syscfg file:  %SYSCFG_FILE%
echo.

:: Find sysconfig_cli.bat
if not exist "%SYSCFG_ROOT%\sysconfig_cli.bat" (
    echo ERROR: sysconfig_cli.bat not found in %SYSCFG_ROOT%
    pause
    exit /b 1
)

:: Find SDK root by locating .metadata\product.json
set "SDK_CHECK=%SDK_ROOT%"
set "iter=0"
:sdk_search_loop
if exist "%SDK_CHECK%\.metadata\product.json" goto sdk_search_exit
if %iter% geq 5 (
    echo ERROR: .metadata\product.json not found near %SDK_ROOT%
    pause
    exit /b 1
)
set /a iter+=1
for %%a in ("%SDK_CHECK%") do set "SDK_CHECK=%%~dpa"
set "SDK_CHECK=%SDK_CHECK:~0,-1%"
goto sdk_search_loop
:sdk_search_exit
set "SDK_ROOT=%SDK_CHECK%"

:: Clean previous output
if exist "%PROJ_ROOT%\.ti\generate" rd /s /q "%PROJ_ROOT%\.ti\generate"

:: Run SysConfig
echo Running SysConfig...
call "%SYSCFG_ROOT%\sysconfig_cli.bat" ^
    -o "%PROJ_ROOT%\.ti\generate" ^
    -s "%SDK_ROOT%\.metadata\product.json" ^
    --compiler gcc ^
    "%SYSCFG_FILE%"

if not exist "%PROJ_ROOT%\.ti\generate" (
    echo ERROR: SysConfig failed to generate output
    pause
    exit /b 1
)
echo.

:: Move .c files to Core/Src
set "CORE_SRC_DIR=%PROJ_ROOT%\Core\Src"
if exist "%PROJ_ROOT%\.ti\generate\*.c" (
    if exist "%CORE_SRC_DIR%" rd /s /q "%CORE_SRC_DIR%"
    mkdir "%CORE_SRC_DIR%"
    move /Y "%PROJ_ROOT%\.ti\generate\*.c" "%CORE_SRC_DIR%\" >nul 2>&1
    echo   .c files moved to Core\Src\
) else (
    echo WARNING: No .c files in SysConfig output
)

:: Move .h files to Core/Inc
set "CORE_INC_DIR=%PROJ_ROOT%\Core\Inc"
if exist "%PROJ_ROOT%\.ti\generate\*.h" (
    if exist "%CORE_INC_DIR%" rd /s /q "%CORE_INC_DIR%"
    mkdir "%CORE_INC_DIR%"
    move /Y "%PROJ_ROOT%\.ti\generate\*.h" "%CORE_INC_DIR%\" >nul 2>&1
    echo   .h files moved to Core\Inc\
) else (
    echo WARNING: No .h files in SysConfig output
)

echo.
echo ========================================
echo  SysConfig Complete!
echo ========================================
echo.
exit /b 0
