@echo off
echo Converting GLSL files to SPIR-V...
echo.

:: Define directory paths as variables
set INPUT_DIR=glsl
set OUTPUT_DIR=spirv
set COUNTER=0

:: Create output directory if it doesn't exist
if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

:: Process shaders
for %%f in (%INPUT_DIR%\*) do (
    echo Converting vertex shader: %%f
    glslang -G -g "%%f" -o "%OUTPUT_DIR%\%%~nf%%~xf.spv"
    if %ERRORLEVEL% EQU 0 (
        echo - Success: Created %OUTPUT_DIR%\%%~nf%%~xf.spv
        set /a COUNTER+=1
    ) else (
        echo - Error: Failed to convert %%f
    )
	echo.
)

echo.
echo Conversion complete. Processed %COUNTER% files.
pause