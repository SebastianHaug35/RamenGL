#!/bin/sh

PROGRAM_SRC_FOLDER=$1

if [[ ! $PROGRAM_SRC_FOLDER ]]; then
    echo Please provide the directory to your sourcefiles.
    exit 1
fi

if [[ ! -d $PROGRAM_SRC_FOLDER ]]; then
    echo Directory does not exist!
    exit 1
fi

if [[ ! -d release_build ]]; then
    mkdir release_build
fi

PROGRAM_NAME=$(basename $PROGRAM_SRC_FOLDER)
echo CMake configure...
cmake -DCMAKE_BUILD_TYPE=Release -DRAMEN_BUILD_PROGRAM_NAME=$PROGRAM_NAME -B release_build -S .
echo CMake build...
cmake --build release_build --config Release -j$(nproc)
echo done.

echo Copy data
# PROGRAM_NAME_DIR=${PROGRAM_NAME}"_DIR"
if [[ -d release_build/$PROGRAM_NAME ]]; then
    echo Deleting old release dir...
    rm -rf release_build/$PROGRAM_NAME
fi
echo Create new release dir...
mkdir release_build/$PROGRAM_NAME

if [[ -f release_build/${PROGRAM_NAME}.zip ]]; then
    echo ZIP file exists. I am deleting it to start fresh.
    rm release_build/${PROGRAM_NAME}.zip
fi


cp release_build/Release/$PROGRAM_NAME release_build/$PROGRAM_NAME/
rsync -avz release_build/Release/lib* release_build/$PROGRAM_NAME/
rsync -avz ./assets release_build/$PROGRAM_NAME/

# mv release_build/$PROGRAM_NAME_DIR release_build/$PROGRAM_NAME

echo ZIP files...
cd release_build
zip ${PROGRAM_NAME}.zip -r $PROGRAM_NAME/
cd ..
echo done.


