@echo off
echo Converting GLSL files to SPIR-V...
echo.

:: Define output directory path as a variable
set OUTPUT_DIR=output\assets\shaders

set COUNTER=0

:: Process vertex shaders
for %%f in (*.vert) do (
    echo Converting vertex shader: %%f
    glslang -G -g "%%f" -o "%OUTPUT_DIR%\%%~nf.vert.spv"
    if %ERRORLEVEL% EQU 0 (
        echo - Success: Created %OUTPUT_DIR%\%%~nf.vert.spv
        set /a COUNTER+=1
    ) else (
        echo - Error: Failed to convert %%f
    )
)

:: Process fragment shaders
for %%f in (*.frag) do (
    echo Converting fragment shader: %%f
    glslang -G -g "%%f" -o "%OUTPUT_DIR%\%%~nf.frag.spv"
    if %ERRORLEVEL% EQU 0 (
        echo - Success: Created %OUTPUT_DIR%\%%~nf.frag.spv
        set /a COUNTER+=1
    ) else (
        echo - Error: Failed to convert %%f
    )
)

echo.
echo Conversion complete. Processed %COUNTER% files.
pause