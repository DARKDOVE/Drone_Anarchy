//
// Copyright (c) 2014 - 2021 Drone Anarchy.
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

//Level Status
const int LSTATUS_NORMAL = 0;
const int LSTATUS_QUIT = 1;
const int LSTATUS_SUSPEND = 2;

//Definitions
const int BULLET_COLLISION_LAYER = 1;
const int PLAYER_COLLISION_LAYER = 2;
const int DRONE_COLLISION_LAYER = 3;
const int FLOOR_COLLISION_LAYER = 5;

//Level Manager Event IDs
const int EVT_UPDATE = 1;
const int EVT_KEYDOWN = 2;
const int EVT_MOUSECLICK = 3;
const int EVT_MOUSEMOVE = 4;
const int EVT_SOUNDFINISH = 5;
const int EVT_JOYSTICK_BUTTONDOWN = 6;
const int EVT_JOYSTICK_BUTTONUP = 7;
const int EVT_JOYSTICK_HATMOVE = 8;

//Application Event IDs
const int EVT_WEB_WINDOW_RESIZED = 9;

//Custom Events
URHO3D_EVENT(E_PLAYERHIT, PlayerHit)
{

}

URHO3D_EVENT(E_PLAYERHEALTHUPDATE, PlayerHealthUpdate)
{
    URHO3D_PARAM(P_CURRENTHEALTHFRACTION,CurrentHealthFraction );
}

URHO3D_EVENT(E_PLAYERDESTROYED, PlayerDestroyed)
{
    URHO3D_PARAM(P_CAMPOSITION,CamPosition );
    URHO3D_PARAM(P_CAMROTATION,CamRotation );
    URHO3D_PARAM(P_PLAYERPOSITION,PlayerPosition );
    URHO3D_PARAM(P_PLAYERROTATION,PlayerRotation );
}

URHO3D_EVENT(E_DRONEDESTROYED, DroneDestroyed)
{
    URHO3D_PARAM(P_DRONEPOINT, DronePoint);
}


URHO3D_EVENT(E_SOUNDGENERATED, SoundGenerated)
{
    URHO3D_PARAM(P_SOUNDNODE, SoundNode);
    URHO3D_PARAM(P_SOUNDNAME, SoundName);
}

URHO3D_EVENT(E_PLAYERROTATION, PlayerRotation)
{
    URHO3D_PARAM(P_DX, DX);
    URHO3D_PARAM(P_DY, DY);
}

URHO3D_EVENT(E_COUNTFINISHED, CountFinished)
{
}

URHO3D_EVENT(E_ACTIVATEWEAPON, ActivateWeapon)
{

}

#endif // EVENTS_AND_DEFS_H
