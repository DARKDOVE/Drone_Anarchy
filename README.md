 Drone Anarchy
==========================
An earlier conceived [Urho3D](https://github.com/urho3d/Urho3D) Proof-Of-Concept for [PROJECT DARKDOVE](http://darkdove.proboards.com/) now updated to use [U3D](https://github.com/u3d-community/U3D).


## Build
DroneAnarchy makes use of [U3D](https://github.com/u3d-community/U3D) CMake build system (`/cmake` and `/script`) to achieve ease of build on multiple platform. This has been confirmed for windows and web, and should work on other platforms supported by U3D with little or no change at all.

### Prerequisite
- C++ compiler with support for at least C++17.
- CMake 3.10.2 or higher.
- [U3D](https://github.com/u3d-community/U3D) library for the respective platform you are targeting ([You would need to build U3D from source](https://u3d.io/docs/_building.html)).

### Steps
- Clone this repository or download the source code as zip.
- Navigate to the root of the project directory.
- For desktop, setup the project with CMake by calling the respective cmake batch script of the platform you are building for, passing your intended build directory and the install or build directory of U3D. These batch scripts are found in the `/script` directory.
    ```shell
    # script/cmake_{platform} {build directory} -D URHO3D_HOME={U3D directory}

    # Example for windows (mingw)
    script/cmake_mingw.bat build\desktop -D URHO3D_HOME=C:\u3d\install

    # Example for windows (msvc)
    script/cmake_vs2022.bat build\desktop -D URHO3D_HOME=C:\u3d\install

    # Example for linux
    script/cmake_generic.sh build/desktop -D URHO3D_HOME=/home/u3d/install
    ```

    For web, ensure you have the emscripten environment setup and activated. Then similarly setup the project like in the case of desktop but with the below command.
    ```shell
    # emcmake script\cmake_emscripten.bat {build directory} -D URHO3D_HOME={U3D directory}
    emcmake script\cmake_emscripten.bat build\web -D URHO3D_HOME=C:\u3d\install
    ```
- Build the project using the generated build files.
    ```shell
    # For Desktop
    cmake --build build/desktop

    # For Web
    cmake --build build/web
    ```
    The built executable or generated WASM file for web will be found in `{build directory}/bin`, for example `build/desktop/bin` for desktop and `build/web/bin` for web.


## Game Play
- Move mouse to rotate
- Click to Shoot
- KEY P to toggle Pause
- ESC To Quit

Credits
--------
- Joystick integration by [Jim](https://discourse.urho3d.io/u/jimmarlowe/summary)