# CG_Experiments

This project renders a stylized bird flying through a polished OpenGL showcase scene using GLUT.

## Showcase features

- Cartoon bird with a custom colorful look.
- Dynamic sky with time-of-day lighting.
- Theme presets: meadow, tropical, sunset, and dusk.
- Weather presets: clear, breezy, rain, and magic particles.
- Animated clouds, floating islands, distant flock silhouettes, and wind-reactive trees.
- Collectible rings and a lightweight score/distance HUD.
- Title card, pause flow, demo camera orbit, and multiple camera modes.

## Build and run

### macOS

From the repo root:

    make
    make run

This uses the macOS OpenGL and GLUT frameworks.

### Best way to share with a Windows professor

Your class submission instructions require one compressed `.zip` or `.rar`
package named:

    studentNo+yourName+"projects"

Minimum expected contents:

1. `bird_scene.exe`
2. Required non-system `.dll` files such as `freeglut.dll` if your Windows build needs them
3. A demo video with audio showing the project running
4. Source code files are optional, but including them is a good fallback

The project searches for `models/bird.obj` from both the current working
directory and the executable directory, which makes Windows launches more
reliable.

Recommended zip layout:

    studentNo_yourName_projects/
    ├── WINDOWS_RUN_ME.txt
    ├── README_project.md
    ├── build_windows.bat
    ├── bird_scene.exe
    ├── freeglut.dll          (if your Windows build needs it)
    ├── demo_video.mp4
    └── models/
        └── bird.obj

To prepare that folder on macOS or Linux:

    chmod +x prepare_submission.sh
    ./prepare_submission.sh studentNo_yourName_projects

This creates a submission folder you can zip or rar after adding the final
Windows executable, required DLL files, and demo video.

### Windows (recommended: CMake)

If CMake and a compiler are already installed:

1. Open the project folder.
2. Double-click `build_windows.bat`

Or run manually:

    cmake -S . -B build
    cmake --build build --config Release

Common output locations:

    build\Release\bird_scene.exe
    build\bird_scene.exe

### Windows (Visual Studio)

Recommended if your professor uses Visual Studio already.

1. Install Visual Studio with Desktop development for C++.
2. Install FreeGLUT, or use an environment where GLUT is already available.
3. Open a Developer Command Prompt in the repo folder.
4. Run:

       cmake -S . -B build
       cmake --build build --config Release

5. Run `build\Release\bird_scene.exe`.

If the program reports a missing `freeglut.dll`, copy that DLL next to the executable.

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

If the app starts but cannot open the bird model, run it from the repo root or keep the `models` folder beside the executable.

### Submission checklist

Before uploading, confirm all of these:

- The program was built for Windows and produced `bird_scene.exe`
- The `.exe` starts on Windows
- `models/bird.obj` is included
- Any required non-system DLLs are next to the `.exe`
- The demo video with audio is included
- The archive name matches `studentNo+yourName+"projects"`

## Compatibility changes

- Added `gl_headers.h` to select `GLUT/glut.h` on macOS and `GL/glut.h` on other platforms.
- Updated the Makefile to link with macOS frameworks on Darwin, `freeglut` on MSYS/MinGW, and `GLUT` on Linux.
- Added a CMake build file for cross-platform builds.
- Added `build_windows.bat` for a simpler Windows build path.
- Added `prepare_submission.sh` and `WINDOWS_RUN_ME.txt` to help create a professor-friendly Windows submission folder.
- Added safer runtime lookup for `models/bird.obj` so Windows launches are less dependent on the current working directory.

## Controls

- `Enter`: start from the title card
- `P`: pause or resume
- `G`: toggle auto-flight
- `T`: cycle visual theme
- `Y`: cycle weather mode
- `M`: toggle cinematic auto-showcase camera
- `H`: toggle HUD
- `K`: toggle detailed controls text
- `B`: cycle bird models
- `F`: toggle follow camera
- `0` to `6`: switch camera views
- `W`, `A`, `S`, `D`, arrow keys, `Q`, `E`, `U`, `O`: fly and pose the bird
- Mouse drag: rotate the bird
- `Ctrl` + drag: roll
- `Shift` + drag or mouse wheel: zoom


