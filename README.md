# CG_Experiments

This project renders a bird flying over a forest scene using OpenGL and GLUT.

## Build and run

### macOS

From the repo root:

    make
    make run

This uses the macOS OpenGL and GLUT frameworks.

### Linux / MSYS2 / MinGW

Install a GLUT/OpenGL implementation such as FreeGLUT.

For MSYS2/Mingw-w64:

    pacman -Syu
    pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-freeglut

Build with:

    make

Direct MSYS2 MinGW-w64 command:

    g++ -std=c++17 -Wall -Wno-deprecated main.cpp OBJImporter.cpp -o bird_scene.exe -lopengl32 -lglu32 -lfreeglut

If Windows cannot find `freeglut.dll`, copy it next to `bird_scene.exe` before running:

    ./bird_scene.exe

Or use CMake:

    mkdir build && cd build
    cmake ..
    cmake --build .

### Windows (Visual Studio)

1. Install FreeGLUT and configure include/library paths.
2. Add `main.cpp`, `OBJImporter.cpp`, `OBJImporter.h`, and `gl_headers.h` to the project.
3. Link with `opengl32.lib`, `glu32.lib`, and `freeglut.lib`.
4. Run the generated executable.

### Windows (MSYS2 quick start)

1. Install MSYS2 from `https://www.msys2.org`.
2. Open the `MSYS2 MinGW x64` shell.
3. Install packages:

       pacman -Syu
       pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-freeglut

4. Build:

       g++ -std=c++17 -Wall -Wno-deprecated main.cpp OBJImporter.cpp -o bird_scene.exe -lopengl32 -lglu32 -lfreeglut

5. Run:

       ./bird_scene.exe

## Compatibility changes

- Added `gl_headers.h` to select `GLUT/glut.h` on macOS and `GL/glut.h` on other platforms.
- Updated the Makefile to link with macOS frameworks on Darwin, `freeglut` on MSYS/MinGW, and `GLUT` on Linux.
- Added a CMake build file for cross-platform builds.
