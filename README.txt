 DRONE ANARCHY
==========================

A simple experimental project inspired by NinjaSnowWars and intended to be used in exploring the basic and essential parts of Urho3D(http://urho3d.github.io/). Drone Anarchy also serves as Urho3D's Proof-Of-Concept for PROJECT DARKDOVE(http://darkdove.proboards.com/).

This work is Licensed under the MIT license, see License.txt for details.

Credits
----------
DARKDOVE team

APPRECIATION
---------------
Urho3D Developers

TO USE
--------
Copy the DroneAnarchy folder into the current working directory of the Drone Anarchy executable. 

For C++, add the source files contained in Source folder to your Urho3D C++ project and build

To run using Urho3DPlayer and AngelScript simply pass DroneAnarchy.as file, which is contained in the DroneAnarchy/Resources/Scripts folder, to the Urho3DPlayer.


GAME PLAY
------------
- Move mouse to rotate
- Mouse Click to Shoot
- KEY P to toggle Pause
- ESC To Quit




NOTE
-----
- DroneAnarchy is constantly upgraded to and tested against the latest master repo of Urho3D

- A method used to ensure better randomness of the Random() function involves tweaking
the code of Urho3DPlayer by adding the following line in its Setup() function

//C++	
srand(time(NULL) % 1000);
SetRandomSeed(rand());
//


CONTRIBUTORS
-------------
BlueMagnificent
jimmarlowe (Jim)