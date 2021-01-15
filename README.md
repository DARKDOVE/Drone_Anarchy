 Drone Anarchy
==========================
An earlier conceived [Urho3D](https://github.com/urho3d/Urho3D) Proof-Of-Concept for [PROJECT DARKDOVE](http://darkdove.proboards.com/) 


To Build and Run <a name="build-run"></a>
----------------

_(**Note:** You need [Urho3D](https://github.com/urho3d/Urho3D) to be able to build Drone Anarchy. You can build Urho3D from [source](https://urho3d.github.io/documentation/HEAD/_building.html) or you can download the [prebuilt](https://sourceforge.net/projects/urho3d/files/Urho3D/) library)_

To build Drone Anarchy, add the source files contained in `Source` folder to your Urho3D C++ project and build. To run the built executable, copy the `Assets` folder into the current working directory of the executable and run.

Build For Web Assembly (WASM)
---------------
First you should be fairly familiar with [emscripten](https://emscripten.org/index.html) and [building](https://emscripten.org/docs/compiling/Building-Projects.html) projects with it.
Also, obviously, you would need a WASM build of Urho3D (you can build from source or download already built version)

This [half-baked makefile](wasm-build-assets/makefile-web.example) can be of help in setting up makefile for WASM build.


Game Play
------------
- Move mouse to rotate
- Click to Shoot
- KEY P to toggle Pause
- ESC To Quit

Credits
--------
- Joystick integration by [Jim](https://discourse.urho3d.io/u/jimmarlowe/summary)

ToDO
------
- Make build instruction more detailed