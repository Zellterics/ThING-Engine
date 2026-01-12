@echo off
setlocal

set GLSLC=glslc
set SHADERS_DIR=.

:: ===== BASIC =====
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

:: ===== POST ===== I didn't test shit on windows, srry I'll check it when I have windows
set VERT=%SHADERS_DIR%\post.vert
set FRAG=%SHADERS_DIR%\post.frag
set VERT_OUT=%SHADERS_DIR%\postVert.spv
set FRAG_OUT=%SHADERS_DIR%\postFrag.spv

echo Compilando post vertex shader...
%GLSLC% "%VERT%" -o "%VERT_OUT%"
if errorlevel 1 goto :error

echo Compilando post fragment shader...
%GLSLC% "%FRAG%" -o "%FRAG_OUT%"
if errorlevel 1 goto :error


:: ===== JFA (COMPUTE) =====
set COMP=%SHADERS_DIR%\jfa.comp
set COMP_OUT=%SHADERS_DIR%\jfaComp.spv

echo Compilando JFA compute shader...
%GLSLC% "%COMP%" -o "%COMP_OUT%"
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
