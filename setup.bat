@echo off

echo executing from: %~DP0
cd %~DP0

REM #############
REM SDL3
REM #############
if exist dependencies\SDL (
	echo SDL dir already exists. I remove it to start over from a clean slate.
	rd /Q /S dependencies\SDL
)
echo Downloading and inflating SDL3...
curl -L -o SDL.zip https://github.com/libsdl-org/SDL/archive/refs/tags/release-3.4.2.zip
powershell -Command "Expand-Archive -Force SDL.zip -DestinationPath dependencies/"
REM rename unzipped SDL-* folder to just SDL
for /d %%f in (dependencies\SDL-*) do ren "%%f" SDL 


REM #############
REM PhysFS
REM #############
if exist dependencies\physfs (
	echo physfs dir already exists. I remove it to start over from a clean slate.
	rd /Q /S dependencies\physfs
)
REM Download PhysFS
echo Downloading and inflating PhysFS...
curl -L -o physfs.zip https://github.com/icculus/physfs/archive/refs/tags/release-3.2.0.zip
powershell -Command "Expand-Archive -Force physfs.zip -DestinationPath dependencies/"
REM rename unzipped physfs-* folder to just physfs
for /d %%f in (dependencies\physfs-*) do ren "%%f" physfs
REM PhysFS 3.2.0 still declares an old CMake minimum that current CMake rejects.
powershell -Command "(Get-Content dependencies/physfs/CMakeLists.txt) -replace 'cmake_minimum_required\(VERSION 3\.0\)', 'cmake_minimum_required(VERSION 3.10)' | Set-Content dependencies/physfs/CMakeLists.txt"


REM #############
REM Assets
REM #############
if not exist assets\models (
	mkdir assets\models
)
echo Downloading and inflating model files...
curl -L -o models.zip https://syncandshare.lrz.de/dl/fiXxtfc7YgH8Dg6u2paF4G/.dir
powershell -Command "Expand-Archive -Force models.zip -DestinationPath assets/models"

echo Downloading and inflating texture files...
curl -L -o textures.zip https://syncandshare.lrz.de/dl/fiEhKjgAgYCKjUSYTxyyy6/.dir
powershell -Command "Expand-Archive -Force textures.zip -DestinationPath assets/textures"

