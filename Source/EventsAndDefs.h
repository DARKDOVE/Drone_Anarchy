//
// Copyright (c) 2014 - 2015 Drone Anarchy.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//



#ifndef EVENTS_AND_DEFS_H
#define EVENTS_AND_DEFS_H

#include <Urho3D/Core/Object.h>

//Definitions
const int BULLET_COLLISION_LAYER = 1;
const int PLAYER_COLLISION_LAYER = 2;
const int DRONE_COLLISION_LAYER = 3;
const int FLOOR_COLLISION_LAYER = 5;

//Custom Events
EVENT(E_PLAYERHIT, PlayerHit)
{
    PARAM(P_CURRENTHEALTHFRACTION,CurrentHealthFraction );
}

EVENT(E_PLAYERDESTROYED, PlayerDestroyed)
{
    PARAM(P_CAMPOSITION,CamPosition );
    PARAM(P_CAMROTATION,CamRotation );
    PARAM(P_PLAYERPOSITION,PlayerPosition );
    PARAM(P_PLAYERROTATION,PlayerRotation );
}

EVENT(E_DRONEDESTROYED, DroneDestroyed)
{
    PARAM(P_DRONEPOINT, DronePoint);
}


EVENT(E_SOUNDGENERATED, SoundGenerated)
{
    PARAM(P_SOUNDNODE, SoundNode);
    PARAM(P_SOUNDNAME, SoundName);
}

EVENT(E_PLAYERROTATION, PlayerRotation)
{
    PARAM(P_DX, DX);
    PARAM(P_DY, DY);
}

EVENT(E_COUNTFINISHED, CountFinished)
{
}

EVENT(E_ACTIVATEWEAPON, ActivateWeapon)
{

}

#endif // EVENTS_AND_DEFS_H
