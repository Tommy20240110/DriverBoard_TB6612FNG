@echo off
setlocal enabledelayedexpansion

echo ========================================
echo  Configuration Tool
echo ========================================
echo.

:: Determine paths
set "SCRIPT_DIR=%~dp0"
for %%a in ("%SCRIPT_DIR%..") do set "PROJECT_ROOT=%%~fa"
for %%a in ("%PROJECT_ROOT%") do set "PROJECT_NAME=%%~nxa"
set "CSV_FILE=%SCRIPT_DIR%mspm0_chip_db.csv"

:: Template paths
set "TMPL_CMAKE_PRESETS=%PROJECT_ROOT%\.cmake\template\CMakePresets.json.template"
set "TMPL_CMAKE_USER_PRESETS=%PROJECT_ROOT%\.cmake\template\CMakeUserPresets.json.template"
set "TMPL_TOOLCHAIN=%PROJECT_ROOT%\.cmake\template\toolchain.cmake.template"
set "TMPL_CPP_PROPERTIES=%PROJECT_ROOT%\.vscode\template\c_cpp_properties.json.template"
set "TMPL_LAUNCH=%PROJECT_ROOT%\.vscode\template\launch.json.template"
set "TMPL_TASKS=%PROJECT_ROOT%\.vscode\template\tasks.json.template"

:: Output paths
set "OUT_CMAKE_PRESETS=%PROJECT_ROOT%\CMakePresets.json"
set "OUT_CMAKE_USER_PRESETS=%PROJECT_ROOT%\CMakeUserPresets.json"
set "OUT_TOOLCHAIN=%PROJECT_ROOT%\.cmake\toolchain\toolchain.cmake"
set "OUT_CONFIG_INI=%SCRIPT_DIR%config.ini"

:: Ask for reconfigure
:ask_reconfig
set /p "RECONFIGURE=Do you want to reconfigure? (Y/N): "
if /i "!RECONFIGURE!"=="Y" (
    echo Reconfiguring all files...
    echo.
    set "RECONFIG=1"
) else if /i "!RECONFIGURE!"=="N" (
    echo Checking existing configuration...
    echo.
    set "RECONFIG=0"
) else (
    echo Please enter Y or N.
    goto ask_reconfig
)

:: Check existing files if not reconfiguring
if !RECONFIG!==0 (
    call :CheckFiles
    if errorlevel 1 (
        echo Some files are missing. Starting reconfiguration...
        echo.
        set "RECONFIG=1"
    )
)

:: Collect or load configuration
if !RECONFIG!==1 (
    call :CollectConfig
    if errorlevel 1 (
        pause
        exit /b 1
    )
) else (
    call :LoadExistingConfig
)

:: Generate configuration files
if !RECONFIG!==1 (
    call :GenerateCMakePresets
    call :GenerateCMakeUserPresets
    call :GenerateToolchain
    call :GenerateConfigIni
    call :CopyStartupFile
)

:: Ask for VS Code configuration
echo.
:ask_vscode
set /p "USE_VSCODE=Do you want to generate VS Code configuration? (Y/N): "
if /i "!USE_VSCODE!"=="Y" (
    call :GenerateVSCode
) else if /i "!USE_VSCODE!"=="N" (
    echo Skipping VS Code configuration.
) else (
    echo Please enter Y or N.
    goto ask_vscode
)

echo.
echo ========================================
echo  Configuration Complete!
echo ========================================
echo.
pause
exit /b 0


::  CheckFiles - Check if all required output files exist
:CheckFiles
set "MISSING=0"

if not exist "%OUT_CMAKE_PRESETS%" (
    echo MISSING: CMakePresets.json
    set "MISSING=1"
)
if not exist "%OUT_CMAKE_USER_PRESETS%" (
    echo MISSING: CMakeUserPresets.json
    set "MISSING=1"
)
if not exist "%OUT_TOOLCHAIN%" (
    echo MISSING: toolchain.cmake
    set "MISSING=1"
)
if not exist "%OUT_CONFIG_INI%" (
    echo MISSING: config.ini
    set "MISSING=1"
)

if !MISSING!==0 (
    echo All files found!
    exit /b 0
) else (
    echo Some files missing.
    exit /b 1
)


::  LoadExistingConfig - Load configuration from existing files
:LoadExistingConfig
echo Loading existing configuration...
echo.

:: Load from config.ini
if exist "%OUT_CONFIG_INI%" (
    for /f "usebackq tokens=2 delims== " %%a in (`findstr /c:"SYSCFG_ROOT" "%OUT_CONFIG_INI%"`) do set "SYSCFG_ROOT=%%a"
    for /f "usebackq tokens=2 delims== " %%a in (`findstr /c:"TI_SDK_ROOT" "%OUT_CONFIG_INI%"`) do set "TI_SDK_ROOT=%%a"
	for /f "usebackq tokens=2 delims== " %%a in (`findstr /c:"GCC_ROOT" "%OUT_CONFIG_INI%"`) do set "GCC_ROOT=%%a"
	for /f "usebackq tokens=2 delims== " %%a in (`findstr /c:"OPENOCD_ROOT" "%OUT_CONFIG_INI%"`) do set "OPENOCD_ROOT=%%a"
    for /f "usebackq tokens=2 delims== " %%a in (`findstr /c:"CHIP_MODEL" "%OUT_CONFIG_INI%"`)  do set "CHIP_MODEL=%%a"
    for /f "usebackq tokens=2 delims== " %%a in (`findstr /c:"PROJECT_NAME" "%OUT_CONFIG_INI%"`) do set "PROJECT_NAME=%%a"
    echo Loaded from config.ini
)

:: Load CHIP_MODEL from CMakePresets.json (backup)
if exist "%OUT_CMAKE_PRESETS%" (
    for /f "usebackq tokens=*" %%a in (`findstr /c:"\"CHIP_MODEL\"" "%OUT_CMAKE_PRESETS%"`) do set "line=%%a"
    for /f tokens^=4^ delims^=^" %%b in ("!line!") do (
        set "tmp=%%b"
        set "tmp=!tmp:,=!"
        set "tmp=!tmp: =!"
        if not "!tmp!"=="" set "CHIP_MODEL=!tmp!"
    )
    echo Loaded CHIP_MODEL from CMakePresets.json
)

:: Load TI_SDK_ROOT and GCC_ROOT from CMakeUserPresets.json (fallback only, do NOT overwrite config.ini values)
if exist "%OUT_CMAKE_USER_PRESETS%" (
    if "!TI_SDK_ROOT!"=="" (
        for /f "usebackq tokens=2,* delims=: " %%a in (`findstr "TI_SDK_ROOT" "%OUT_CMAKE_USER_PRESETS%" ^| findstr "value"`) do (
            set "tmp=%%b"
            set "tmp=!tmp:"=!"
            set "tmp=!tmp:,=!"
            set "tmp=!tmp: =!"
            if "!tmp!" neq "" if "!TI_SDK_ROOT!"=="" set "TI_SDK_ROOT=!tmp!"
        )
    )
    if "!GCC_ROOT!"=="" (
        for /f "usebackq tokens=2,* delims=: " %%a in (`findstr "GCC_ROOT" "%OUT_CMAKE_USER_PRESETS%" ^| findstr "value"`) do (
            set "tmp=%%b"
            set "tmp=!tmp:"=!"
            set "tmp=!tmp:,=!"
            set "tmp=!tmp: =!"
            if "!tmp!" neq "" if "!GCC_ROOT!"=="" set "GCC_ROOT=!tmp!"
        )
    )
    echo Loaded from CMakeUserPresets.json
)

echo.
echo Existing configuration loaded.
exit /b 0


::  CollectConfig - Prompt user for all configuration values
:CollectConfig
echo Press Enter to keep the existing value (shown in brackets).
echo.

:: Pre-load existing values from config.ini as defaults
if exist "%OUT_CONFIG_INI%" (
    for /f "usebackq tokens=2 delims== " %%a in (`findstr /c:"SYSCFG_ROOT" "%OUT_CONFIG_INI%"`) do set "SYSCFG_ROOT=%%a"
    for /f "usebackq tokens=2 delims== " %%a in (`findstr /c:"TI_SDK_ROOT" "%OUT_CONFIG_INI%"`) do set "TI_SDK_ROOT=%%a"
	for /f "usebackq tokens=2 delims== " %%a in (`findstr /c:"GCC_ROOT" "%OUT_CONFIG_INI%"`) do set "GCC_ROOT=%%a"
	for /f "usebackq tokens=2 delims== " %%a in (`findstr /c:"OPENOCD_ROOT" "%OUT_CONFIG_INI%"`) do set "OPENOCD_ROOT=%%a"
)

:: TI Sysconfig Root
set /p "SYSCFG_ROOT=TI Sysconfig path [%SYSCFG_ROOT%]: "
if "%SYSCFG_ROOT%"=="" (
    echo ERROR: TI Sysconfig path cannot be empty!
    exit /b 1
)
set "SYSCFG_ROOT=%SYSCFG_ROOT:\=/%"

:: TI MSPM0 SDK Root
set /p "TI_SDK_ROOT=TI MSPM0 SDK path [%TI_SDK_ROOT%]: "
if "%TI_SDK_ROOT%"=="" (
    echo ERROR: TI SDK path cannot be empty!
    exit /b 1
)
set "TI_SDK_ROOT=%TI_SDK_ROOT:\=/%"

:: GCC for Arm Path
set /p "GCC_ROOT=GCC path [%GCC_ROOT%]: "
if "%GCC_ROOT%"=="" (
    echo ERROR: GCC path cannot be empty!
    exit /b 1
)
set "GCC_ROOT=%GCC_ROOT:\=/%"

:: OpenOCD Path
set /p "OPENOCD_ROOT=OpenOCD path [%OPENOCD_ROOT%]: "
if "%OPENOCD_ROOT%"=="" (
    echo ERROR: OpenOCD path cannot be empty!
    exit /b 1
)
set "OPENOCD_ROOT=%OPENOCD_ROOT:\=/%"

:: Pre-load chip model from config.ini (may be empty)
for /f "usebackq tokens=2 delims== " %%a in (`findstr /c:"CHIP_MODEL" "%OUT_CONFIG_INI%" 2^>nul`) do set "CHIP_MODEL=%%a"

:: Chip Model
echo.
set /p "CHIP_MODEL_INPUT=Enter chip model [%CHIP_MODEL%]: "

:: If user pressed Enter, use pre-loaded value
if "!CHIP_MODEL_INPUT!"=="" set "CHIP_MODEL_INPUT=!CHIP_MODEL!"

if "!CHIP_MODEL_INPUT!"=="" (
    echo ERROR: Chip model cannot be empty!
    exit /b 1
)

if not exist "%CSV_FILE%" (
    echo ERROR: Chip database not found at %CSV_FILE%!
    exit /b 1
)

set "CHIP_FOUND=0"
for /f "usebackq skip=1 tokens=1-6 delims=," %%a in ("%CSV_FILE%") do (
    if /i "%%a"=="!CHIP_MODEL_INPUT!" (
        set "CHIP_MODEL=%%a"
        set "CHIP_FAMILY=%%b"
        set "CORE=%%c"
        set "ARCH=%%d"
        set "FLOAT_ABI=%%e"
        set "STARTUP_FILE=%%f"
        set "CHIP_FOUND=1"
    )
)

if !CHIP_FOUND!==0 (
    echo.
    echo ERROR: Chip model "!CHIP_MODEL_INPUT!" not found in database!
    echo Run the script again and enter a valid chip model.
    exit /b 1
)

echo.
echo Found chip configuration:
echo   CHIP_MODEL:    !CHIP_MODEL!
echo   CHIP_FAMILY:   !CHIP_FAMILY!
echo   CORE:          !CORE!
echo   ARCH:          !ARCH!
echo   FLOAT_ABI:     !FLOAT_ABI!
echo   STARTUP_FILE:  !STARTUP_FILE!

exit /b 0


::  GenerateCMakePresets
:GenerateCMakePresets
echo.
echo Generating CMakePresets.json...

if not exist "%TMPL_CMAKE_PRESETS%" (
    echo ERROR: Template not found: %TMPL_CMAKE_PRESETS%
    exit /b 1
)

call :SubFile "%TMPL_CMAKE_PRESETS%" "%OUT_CMAKE_PRESETS%"

if exist "%OUT_CMAKE_PRESETS%" (
    echo CMakePresets.json generated.
) else (
    echo ERROR: Failed to generate CMakePresets.json!
    exit /b 1
)
exit /b 0


::  GenerateCMakeUserPresets
:GenerateCMakeUserPresets
echo.
echo Generating CMakeUserPresets.json...

if not exist "%TMPL_CMAKE_USER_PRESETS%" (
    echo ERROR: Template not found: %TMPL_CMAKE_USER_PRESETS%
    exit /b 1
)

call :SubFile "%TMPL_CMAKE_USER_PRESETS%" "%OUT_CMAKE_USER_PRESETS%"

if exist "%OUT_CMAKE_USER_PRESETS%" (
    echo CMakeUserPresets.json generated.
) else (
    echo ERROR: Failed to generate CMakeUserPresets.json!
    exit /b 1
)
exit /b 0


::  GenerateToolchain
:GenerateToolchain
echo.
echo Generating toolchain.cmake...

if not exist "%TMPL_TOOLCHAIN%" (
    echo ERROR: Template not found: %TMPL_TOOLCHAIN%
    exit /b 1
)

set "TOOLCHAIN_DIR=%PROJECT_ROOT%\.cmake\toolchain"
if not exist "!TOOLCHAIN_DIR!" mkdir "!TOOLCHAIN_DIR!"

call :SubFile "%TMPL_TOOLCHAIN%" "%OUT_TOOLCHAIN%"

if exist "%OUT_TOOLCHAIN%" (
    echo toolchain.cmake generated.
) else (
    echo ERROR: Failed to generate toolchain.cmake!
    exit /b 1
)
exit /b 0


	::  GenerateConfigIni
	:GenerateConfigIni
	echo.
	echo Generating config.ini...
	
	> "%OUT_CONFIG_INI%" echo [Config]
	>>"%OUT_CONFIG_INI%" echo SYSCFG_ROOT = !SYSCFG_ROOT!
	>>"%OUT_CONFIG_INI%" echo TI_SDK_ROOT = !TI_SDK_ROOT!
	>>"%OUT_CONFIG_INI%" echo GCC_ROOT = !GCC_ROOT!
	>>"%OUT_CONFIG_INI%" echo OPENOCD_ROOT = !OPENOCD_ROOT!
	>>"%OUT_CONFIG_INI%" echo CHIP_MODEL = !CHIP_MODEL!
	>>"%OUT_CONFIG_INI%" echo PROJECT_NAME = !PROJECT_NAME!
	
	if exist "%OUT_CONFIG_INI%" (
	    echo config.ini generated.
	) else (
	    echo ERROR: Failed to generate config.ini!
	    exit /b 1
	)
	exit /b 0
	


::  CopyStartupFile - Copy startup file from SDK to Core/Startup/
:CopyStartupFile
echo.
echo Copying startup file...

:: Convert to backslashes for Windows file operations
set "SDK_BS=!TI_SDK_ROOT:/=\!"
set "STARTUP_SRC=!SDK_BS!\source\ti\devices\msp\m0p\startup_system_files\gcc\!STARTUP_FILE!.c"
set "STARTUP_DST=%PROJECT_ROOT%\Core\Startup\!STARTUP_FILE!.c"

echo   Source: !STARTUP_SRC!

if not exist "!STARTUP_SRC!" (
    echo WARNING: Startup file not found at:
    echo   !STARTUP_SRC!
    echo Please check that TI_SDK_ROOT points to a valid MSPM0 SDK installation.
    exit /b 0
)

set "STARTUP_DIR=%PROJECT_ROOT%\Core\Startup"
if not exist "!STARTUP_DIR!" mkdir "!STARTUP_DIR!"

:: Clean old startup files before copying new one
if exist "!STARTUP_DIR!\startup_*.c" del /Q "!STARTUP_DIR!\startup_*.c"

copy /Y "!STARTUP_SRC!" "!STARTUP_DST!" >nul
if exist "!STARTUP_DST!" (
    echo   Copied: !STARTUP_FILE!.c
) else (
    echo WARNING: Failed to copy startup file!
)
exit /b 0


::  GenerateVSCode - Generate .vscode/c_cpp_properties.json from template
:GenerateVSCode
echo.
echo Generating VS Code configuration...

set "VSCODE_DIR=%PROJECT_ROOT%\.vscode"
if not exist "!VSCODE_DIR!" mkdir "!VSCODE_DIR!"

:: Build device define (e.g., __MSPM0G3507__)
set "DEVICE_DEFINE_UPPER=!CHIP_MODEL!"
for %%c in (a-A b-B c-C d-D e-E f-F g-G h-H i-I j-J k-K l-L m-M n-N o-O p-P q-Q r-R s-S t-T u-U v-V w-W x-X y-Y z-Z) do (
    for /f "tokens=1,* delims=-" %%x in ("%%c") do (
        set "DEVICE_DEFINE_UPPER=!DEVICE_DEFINE_UPPER:%%x=%%y!"
    )
)
set "DEVICE_DEFINE=__!DEVICE_DEFINE_UPPER!__"

:: c_cpp_properties.json
echo   - c_cpp_properties.json
set "OUT_CPP=!VSCODE_DIR!\c_cpp_properties.json"
if not exist "%TMPL_CPP_PROPERTIES%" (
    echo ERROR: Template not found: %TMPL_CPP_PROPERTIES%
    exit /b 1
)

call :SubFile "%TMPL_CPP_PROPERTIES%" "%OUT_CPP%"

if not exist "%OUT_CPP%" (
    echo ERROR: Failed to generate c_cpp_properties.json!
    exit /b 1
)

:: launch.json
echo   - launch.json
set "OUT_LAUNCH=!VSCODE_DIR!\launch.json"
if not exist "%TMPL_LAUNCH%" (
    echo WARNING: Template not found: %TMPL_LAUNCH%
) else (
    call :SubFile "%TMPL_LAUNCH%" "%OUT_LAUNCH%"
)

:: tasks.json
echo   - tasks.json
set "OUT_TASKS=!VSCODE_DIR!\tasks.json"
if not exist "%TMPL_TASKS%" (
    echo WARNING: Template not found: %TMPL_TASKS%
) else (
    call :SubFile "%TMPL_TASKS%" "%OUT_TASKS%"
)

echo VS Code configuration generated.
exit /b 0


::  SubFile - Copy template, replacing @VAR@ tokens
::  %1 = template path, %2 = output path
:SubFile
set "tmpl=%~1"
set "out=%~2"

if exist "%out%" del "%out%"

for /f "usebackq tokens=1,* delims=:" %%a in (`findstr /n "^" "%tmpl%"`) do (
    set "line=%%b"
    if "!line!"=="" (
        echo.>> "%out%"
    ) else (
        set "line=!line:@CHIP_MODEL@=%CHIP_MODEL%!"
        set "line=!line:@CHIP_FAMILY@=%CHIP_FAMILY%!"
        set "line=!line:@CORE@=%CORE%!"
        set "line=!line:@ARCH@=%ARCH%!"
        set "line=!line:@FLOAT_ABI@=%FLOAT_ABI%!"
        set "line=!line:@TI_SDK_ROOT@=%TI_SDK_ROOT%!"
        set "line=!line:@GCC_ROOT@=%GCC_ROOT%!"
        set "line=!line:@OPENOCD_ROOT@=%OPENOCD_ROOT%!"
        set "line=!line:@PROJECT_NAME@=%PROJECT_NAME%!"
        set "line=!line:@STARTUP_FILE@=%STARTUP_FILE%!"
        set "line=!line:${config:ti.sdk.root}=%TI_SDK_ROOT%!"
        if defined DEVICE_DEFINE (
            set "line=!line:Please fill in your device macro definition such as __MSPM0G3507__=%DEVICE_DEFINE%!"
        )
        echo !line!>> "%out%"
    )
)
exit /b 0
