#!/bin/sh

SDL_DIR=./dependencies/SDL/
PHYSFS_DIR=./dependencies/physfs/

# Download and extract SDL. Rename to "SDL"
curl -L -o SDL.zip https://github.com/libsdl-org/SDL/archive/refs/tags/release-3.4.2.zip
if [[ -d "$SDL_DIR" ]]; then
    echo "$SDL_DIR exists. I remove it..."
    rm -rf "$SDL_DIR"
    echo "Done."
fi
unzip -q SDL.zip -d dependencies
mv dependencies/SDL* dependencies/SDL


# Download and extract PhysFS. Rename to "physfs"
curl -L -o physfs.zip https://github.com/icculus/physfs/archive/refs/tags/release-3.2.0.zip
if [[ -d "$PHYSFS_DIR" ]]; then
    echo "$PHYSFS_DIR exists. I remove it..."
    rm -rf "$PHYSFS_DIR"
    echo "Done."
fi
unzip -q physfs.zip -d dependencies
mv dependencies/physfs* dependencies/physfs


# Download model files
curl -L -o models.zip https://syncandshare.lrz.de/dl/fiXxtfc7YgH8Dg6u2paF4G/.dir
unzip -q -o models.zip -d assets/models

# Download texture files
curl -L -o textures.zip https://syncandshare.lrz.de/dl/fiEhKjgAgYCKjUSYTxyyy6/.dir
unzip -q -o textures.zip -d assets/textures
