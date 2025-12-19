@echo off
setlocal

set GLSLC=glslc
set SHADERS_DIR=.
set VERT=%SHADERS_DIR%\basic.vert
set FRAG=%SHADERS_DIR%\basic.frag
set VERT_OUT=%SHADERS_DIR%\basicVert.spv
set FRAG_OUT=%SHADERS_DIR%\basicFrag.spv

echo Compilando vertex shader...
%GLSLC% "%VERT%" -o "%VERT_OUT%"
if errorlevel 1 goto :error

echo Compilando fragment shader...
%GLSLC% "%FRAG%" -o "%FRAG_OUT%"
if errorlevel 1 goto :error


set VERT=%SHADERS_DIR%\circle.vert
set FRAG=%SHADERS_DIR%\circle.frag
set VERT_OUT=%SHADERS_DIR%\circleVert.spv
set FRAG_OUT=%SHADERS_DIR%\circleFrag.spv
:: UNTESTED :p
echo Compilando vertex shader...
%GLSLC% "%VERT%" -o "%VERT_OUT%"
if errorlevel 1 goto :error

echo Compilando fragment shader...
%GLSLC% "%FRAG%" -o "%FRAG_OUT%"
if errorlevel 1 goto :error


set VERT=%SHADERS_DIR%\outline.vert
set FRAG=%SHADERS_DIR%\outline.frag
set VERT_OUT=%SHADERS_DIR%\outlineVert.spv
set FRAG_OUT=%SHADERS_DIR%\outlineFrag.spv

echo Compilando vertex shader...
%GLSLC% "%VERT%" -o "%VERT_OUT%"
if errorlevel 1 goto :error

echo Compilando fragment shader...
%GLSLC% "%FRAG%" -o "%FRAG_OUT%"
if errorlevel 1 goto :error

echo.
echo ✅ Compilación exitosa.
goto :end

:error
echo.
echo ❌ Error durante la compilación.

:end
pause
endlocal
