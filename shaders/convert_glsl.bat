@echo off
echo ===================================================
echo Converting GLSL to all other shader targets...
echo ===================================================
echo.

call convert_glsl_to_spirv.bat
echo.

call convert_spirv_to_hlsl.bat
echo.

call convert_hlsl_to_dxbc.bat
echo.

call convert_hlsl_to_dxil.bat
echo.

echo ===================================================
echo All shader conversion processes completed!
echo ===================================================