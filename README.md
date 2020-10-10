 Drone Anarchy
==========================

Conceived as an experimental project used in exploring the basic and essential parts of [Urho3D](http://urho3d.github.io/), Drone Anarchy serves as Urho3D's Proof-Of-Concept for [PROJECT DARKDOVE](http://darkdove.proboards.com/).

You can either run Drone Anarchy directly as scripts through Urho3D's `Urho3DPlayer` or build the C++ source files and run.


To Run As Scripts
--------------------------
On the command line call up `Urho3DPlayer` as shown below
```
Urho3DPlayer Scripts/DroneAnarchy.as -pp path-to-Assets-folder -p CoreData;GameData
```
where `path-to-Assets-folder` is the path to the `Assets` folder

To Build and Run C++
-------------
Add the source files contained in `Source` folder to your Urho3D C++ project and build. Copy the `Assets` folder into the current working directory of the executable and run.


GAME PLAY
------------
- Move mouse to rotate
- Click to Shoot
- KEY P to toggle Pause
- ESC To Quit
