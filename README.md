# How to build.

## Get dependencies and assets:

**Automatic setup UNIX. curl must be installed.**  
call
```bash
./setup.sh
```

**Automatic setup Windows.**  
call or double-click
```bash
setup.bat
```

**Manual setup**  
### SDL
Download SDL3 sources
- https://github.com/libsdl-org/SDL/archive/refs/tags/release-3.4.2.zip
```bash
mkdir dependencies
```
unpack SDL3 and rename to `SDL` and place into `dependencies`.  

### PhysFS
Download PhysFS sources
- https://github.com/icculus/physfs/archive/refs/tags/release-3.2.0.zip
```bash
mkdir dependencies
```
unpack PhysFS and rename to `physfs` and place into `dependencies`.  

### Models/Textures
Download from Sync&Share
- https://syncandshare.lrz.de/dl/fiXxtfc7YgH8Dg6u2paF4G/.dir
Make new directory called `assets/`.
Extract the obj files to `assets/models`.


## Build:
```bash
cmake -B build -S .
cmake --build build/
```

Executables will be in `build`. On Windows in `build/Debug` (and `build/Release` for a release build).


## How to load assets in your code.
Ramen uses PhysFS which provides a virtual filesystem. That is, you **mount**
a system directory which will become your **root**-directory. You then
load all files relative to that **root** directory. Note that you cannot
escape the root directory! All you have to do for this to work is call
```c
Filesystem* pFS = Filesystem::Init(argc, argv, "assets");
```
At the beginning of your program. This will mount the `assets/` directory
**relative to your executable** as root. So if your exe lives in
`/home/myhomedir/computergraphics/` PhysFS expects the folder `assets` to be
at location (`/home/myhomedir/computergraphics/assets/`). Now you can
load eg. models via:
```c
model.Load("models/Skull.obj");
```
This loads `Skull.obj` from location `assets/models`.

You can also provide the directory to mount via command-line to your program:
```bash
./myprogram /home/me/somefolder/
```
Relative paths work as well:
```bash
./myprogram somefolder/relative/to/cwd/
```
If you provide a path via command line, then the last argument of `Filesystem::Init` will
be ignored as it has lower precedence.  

If you neither provide a path via command line nor an argument to `Filesystem::Init`
then the directory of your executable will be mounted as root.

## Ship It.
**UNIX**:  
```bash
./shipit.sh <path-to-target-folder>
```

**Windows**:
```bash
shipit.bat <path-to-target-folder>
```

The script will create a Release configuration via CMake, compile
and zip the resulting libraries and executable together with the
`assets/` folder to a new folder called `release_build/`.
So in the end you will have your program ready to ship as
`release_build/<target>-Win-x64.zip` on Windows
and `release_build/<target>.zip` on UNIX.


## Known issues
- CMake refuses to generate Makefile/VS-Solution because CMake minimum required in PhysFS.
  **Solution**: Go into `./dependencies/physfs/` and change the line
  ```cmake
  cmake_minimum_required(VERSION 3.0)
  ```
  to
  ```cmake
  cmake_minimum_required(VERSION 3.10)
  ```


