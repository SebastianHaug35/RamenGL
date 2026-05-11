#!/bin/sh

# Download models
curl -L -o models.zip https://syncandshare.lrz.de/dl/fiXxtfc7YgH8Dg6u2paF4G/.dir
unzip -q -o models.zip -d assets/models

# Download textures
curl -L -o textures.zip https://syncandshare.lrz.de/dl/fiEhKjgAgYCKjUSYTxyyy6/.dir
unzip -q -o textures.zip -d assets/textures
