@echo off
setlocal EnableDelayedExpansion

echo Converting SPIR-V files to HLSL...
echo.

:: Define directory paths as variables
set INPUT_DIR=spirv
set OUTPUT_DIR=hlsl
set COUNTER=0

:: Create output directory if it doesn't exist
if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

:: Process SPIR-V files to HLSL
for %%f in (%INPUT_DIR%\*.spv) do (
    echo Converting SPIR-V file: %%f
    spirv-cross "%%f" --hlsl --entry main --shader-model 50 --output "%OUTPUT_DIR%\%%~nf.hlsl"
	
    if !ERRORLEVEL! EQU 0 (
        echo - Success: Created %OUTPUT_DIR%\%%~nf.hlsl
        set /a COUNTER+=1
    ) else (
        echo - Error: Failed to convert %%f to HLSL
		pause
    )
    echo.
)

echo.
echo Conversion complete. Converted %COUNTER% files from SPIR-V to HLSL.