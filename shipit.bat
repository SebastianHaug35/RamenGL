@echo off

echo executing from: %~DP0
cd %~DP0

if "%1" equ "" (
	echo Please provide the directory to your sourcefiles.
	pause /B 1
)

set PROGRAM_SRC_FOLDER=%~1
if not exist %PROGRAM_SRC_FOLDER% (
	echo %PROGRAM_SRC_FOLDER% does not exist! Abort.
	pause
	exit /B 1
)

:: Replace forward slashes with backslashes
set PROGRAM_NAME=%PROGRAM_SRC_FOLDER%
set PROGRAM_NAME=%PROGRAM_NAME:/=\%

:: Remove trailing backslash if present
if "%PROGRAM_NAME:~-1%"=="\" (	
	set PROGRAM_NAME=%PROGRAM_NAME:~0,-1%
)
:: Extract last folder name (good lord, that syntax!)
for %%A in ("%PROGRAM_NAME%") do set "PROGRAM_NAME=%%~nxA"

echo Create release build of %PROGRAM_NAME%
if not exist release_build (
	mkdir release_build
)

echo CMake configure...
cmake -DCMAKE_BUILD_TYPE=Release -DRAMEN_BUILD_PROGRAM_NAME=%PROGRAM_NAME% -B release_build -S .
echo CMake build...
cmake --build release_build --config Release -j %NUMBER_OF_PROCESSORS%
echo done.

echo Create ZIP
if not exist release_build\%PROGRAM_NAME% (
	mkdir release_build\%PROGRAM_NAME%
)
xcopy release_build\Release\%PROGRAM_NAME%.exe release_build\%PROGRAM_NAME%\ /Y
xcopy release_build\Release\SDL3.dll           release_build\%PROGRAM_NAME%\ /Y
xcopy release_build\Release\physfs.dll         release_build\%PROGRAM_NAME%\ /Y
xcopy release_build\Release\tinyobjloader.dll  release_build\%PROGRAM_NAME%\ /Y
robocopy assets                                release_build/%PROGRAM_NAME%/assets/ /E

powershell -Command Compress-Archive -Force -Path release_build/%PROGRAM_NAME%/ -DestinationPath release_build/%PROGRAM_NAME%-Win-x64.zip
echo Done.
pause
