 DRONE ANARCHY
==========================

A simple test project inspired by NinjaSnowWars and intended to be used in exploring the basic and essential parts of Urho3D(http://urho3d.github.io/). Drone Anarchy is an experimental project of PROJECT DARKDOVE(http://darkdove.proboards.com/).

This work is Licensed under the MIT license, see License.txt for details.

Credits
----------
DARKDOVE team

APPRECIATION
---------------
Urho3D Developers

TO USE
--------
Copy the DroneAnarchy folder into one of your Urho3D ResourcePaths folder,
that will be any of "Data" or "CoreData" folders if using the default setting. 

For C++, add the source files contained in Source folder to your Urho3D C++ project and build

To run using Urho3DPlayer and AngelScript simply pass DroneAnarchy.as file, which is contained in the DroneAnarchy folder, to the Urho3DPlayer.


GAME PLAY
------------
- Move mouse to rotate
- Mouse Click to Shoot
- KEY P to toggle Pause
- ESC To Quit




NOTE
-----
- Due to a bugfix relating to text attribute animation of the Text UI, a master branch build later
than 4th Dec 2014 is required ( to be precise Urho3D-1.32.91 Snapshot and upward)

- A method used to ensure better randomness of the Random() function involves tweaking
the code of Urho3DPlayer by adding the following line in its Setup() function

//C++	
srand(time(NULL) % 1000);
SetRandomSeed(rand());
//

- This application has only been tested on the Windows Platform but should also work on others
