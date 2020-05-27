@echo off
SET currentPath=%~dp0
SET projectPath=%currentPath%..\
SET tmpDIR=%currentPath%_generated\
SET workingDIR=%tmpDIR%cmake_win\
SET toolchain_file=%projectPath%..\external\win\vcpkg\scripts\buildsystems\vcpkg.cmake

echo "create dir"
echo "%projectPath%"

if exist %workingDIR% rd /s /Q %workingDIR%
if exist %tmpDIR% rd /s /Q %tmpDIR%
md %tmpDIR%
md %workingDIR%
cd %workingDIR%

echo %workingDIR%

cmake %projectPath% -G "Visual Studio 15 2017" -T v141_xp -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=%toolchain_file% -DPLATFORM=x86 -DARCH=i686

set build_config="Release"
set vsdev="C:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\Common7\Tools\VsDevCmd.bat"
call %vsdev%
set devenv="%VS150COMNTOOLS%../IDE/devenv.exe"    
set sln=%workingDIR%cicadaPlayer.sln

echo on
devenv %sln% /rebuild %build_config% /project ALL_BUILD
pause