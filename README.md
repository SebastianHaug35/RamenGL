# How to build.

## Get dependencies:
Download SDL3 sources
- https://github.com/libsdl-org/SDL/archive/refs/tags/release-3.4.2.zip
```bash
mkdir dependencies
```
unpack SDL3 and rename to `SDL` and place into `dependencies`.  

## Build:
```bash
cmake -B build -S .
cmake --build build/
```

Executables will be in `build`.

