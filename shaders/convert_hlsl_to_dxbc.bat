@echo off
setlocal EnableDelayedExpansion


echo Compiling HLSL files to DXBC...
echo.

:: Define directory paths as variables
set INPUT_DIR=hlsl
set OUTPUT_DIR=dxbc
set COUNTER=0

:: Create output directory if it doesn't exist
if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

:: Process HLSL files to DXBC
for %%f in (%INPUT_DIR%\*.hlsl) do (
    set "filename=%%~nxf"
    set "basename=%%~nf"
    set "target="

    :: Detect shader type by extension
    if /i "!filename:~-10!"==".vert.hlsl" (
        set "target=vs_5_0"
    ) else if /i "!filename:~-10!"==".frag.hlsl" (
        set "target=ps_5_0"
    ) else if /i "!filename:~-10!"==".comp.hlsl" (
        set "target=cs_5_0"
    ) else if /i "!filename:~-10!"==".geom.hlsl" (
        set "target=gs_5_0"
    ) else if /i "!filename:~-10!"==".tesc.hlsl" (
        set "target=hs_5_0"
    ) else if /i "!filename:~-10!"==".tese.hlsl" (
        set "target=ds_5_0"
    )

    if defined target (
        echo Compiling %%f with target !target!
        fxc /T !target! /E main /Fo "%OUTPUT_DIR%\%%~nf.dxbc" "%%f"

        if !ERRORLEVEL! EQU 0 (
            echo - Success: Created %OUTPUT_DIR%\%%~nf.dxbc
            set /a COUNTER+=1
        ) else (
            echo - Error: Failed to compile %%f
        )
    ) else (
        echo Warning: Skipping unknown shader type: %%f
		pause
    )
    echo.
)

echo.
echo Compilation complete. Compiled %COUNTER% files from HLSL to DXBC.