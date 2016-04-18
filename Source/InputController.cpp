//
// Copyright (c) 2014 - 2016 Drone Anarchy.
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
/// Console Game Controller handling

#include <Urho3D/Urho3D.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Input/InputEvents.h>

#include "InputController.h"


virtualController::virtualController() :  // xbox 360 button mapping is default
    lookup(),
    updatecounter_(0),
    updatevalue_(0)
{
    lookup[CONTROLLER_BUTTON_A] = CONTROLLER_BUTTON_A;
    lookup[CONTROLLER_BUTTON_B] = CONTROLLER_BUTTON_B;
    lookup[CONTROLLER_BUTTON_X] = CONTROLLER_BUTTON_X;
    lookup[CONTROLLER_BUTTON_Y] = CONTROLLER_BUTTON_Y;
    lookup[CONTROLLER_BUTTON_BACK] = CONTROLLER_BUTTON_BACK;
    lookup[CONTROLLER_BUTTON_GUIDE] = CONTROLLER_BUTTON_GUIDE;
    lookup[CONTROLLER_BUTTON_START] = CONTROLLER_BUTTON_START;
    lookup[CONTROLLER_BUTTON_LEFTSTICK] = CONTROLLER_BUTTON_LEFTSTICK;
    lookup[CONTROLLER_BUTTON_RIGHTSTICK] = CONTROLLER_BUTTON_RIGHTSTICK;
    lookup[CONTROLLER_BUTTON_LEFTSHOULDER] = CONTROLLER_BUTTON_LEFTSHOULDER;
    lookup[CONTROLLER_BUTTON_RIGHTSHOULDER] = CONTROLLER_BUTTON_RIGHTSHOULDER;
    lookup[CONTROLLER_BUTTON_DPAD_UP] = CONTROLLER_BUTTON_DPAD_UP;
    lookup[CONTROLLER_BUTTON_DPAD_DOWN] = CONTROLLER_BUTTON_DPAD_DOWN;
    lookup[CONTROLLER_BUTTON_DPAD_LEFT] = CONTROLLER_BUTTON_DPAD_LEFT;
    lookup[CONTROLLER_BUTTON_DPAD_RIGHT] = CONTROLLER_BUTTON_DPAD_RIGHT;
    lookup[DA_LEFT_TRIGGER] = DA_LEFT_TRIGGER;
    lookup[DA_RIGHT_TRIGGER] = DA_RIGHT_TRIGGER;
    lookup[DA_HAT_UP] = -1;
    lookup[DA_HAT_DOWN] = -1;
    lookup[DA_HAT_LEFT] = -1;
    lookup[DA_HAT_RIGHT] = -1;
}

// overlay an actual controllers button, hat (if it's got one) mapping
void virtualController::remap_button ( int index, int value )
{
    if ( index >= CONTROLLER_BUTTON_A && index < DA_LAST )
        lookup[index] = value;
}

// return actual controllers button value
int virtualController::button ( int index ) const
{
    if ( index >= CONTROLLER_BUTTON_A && index < DA_LAST )
        return lookup[index];
    return -1;
}

// clear button settings to be invalid, we'll fix them later
void virtualController::clearAll()
{
    for(unsigned int nn=0; nn < DA_LAST; nn++)
        remap_button(nn ,-1);
}

// A PS3 usb controller
void virtualController::make_ps3()
{
    clearAll();
    remap_button ( CONTROLLER_BUTTON_X, 2 );
    remap_button ( CONTROLLER_BUTTON_DPAD_UP, 11 );
    remap_button ( CONTROLLER_BUTTON_DPAD_DOWN, 12);
    remap_button ( CONTROLLER_BUTTON_DPAD_LEFT, 13);
    remap_button ( CONTROLLER_BUTTON_DPAD_RIGHT, 14);
    remap_button ( CONTROLLER_BUTTON_BACK, 4);
    remap_button ( CONTROLLER_BUTTON_START, 6 );
}

// MYPOWER 2in1 (cheap) usb controller
void virtualController::make_2in1()
{
    clearAll();
    remap_button ( CONTROLLER_BUTTON_X, 3 );
    remap_button ( DA_HAT_UP, 1 );
    remap_button ( DA_HAT_DOWN, 4 );
    remap_button ( DA_HAT_LEFT, 8 );
    remap_button ( DA_HAT_RIGHT, 2 );
    remap_button ( CONTROLLER_BUTTON_BACK, 8 );
    remap_button ( CONTROLLER_BUTTON_START, 9);
}

// read settings out of a user prepared configuration file
void virtualController::load_user_settings(  ResourceCache* rcache )
{
    if ( !rcache->Exists ("Resources/Settings/dajoystick.xml") ) return; // no file, no error in console

    XMLFile* ujoy = rcache->GetResource<XMLFile>("Resources/Settings/dajoystick.xml");
    if ( ujoy != NULL )
    {
        clearAll();	// start clean
        XMLElement Elem = ujoy->GetRoot("DroneAnarchyJoystick"); // read out settings
        if (Elem.IsNull() ) return;

        // look thru all the elements, we'll read the ones that are defined.
        XMLElement ElemN = Elem.GetChild("CONTROLLER_BUTTON_A");
        if (ElemN.NotNull()) remap_button ( CONTROLLER_BUTTON_A, atoi( ElemN.GetValue().CString() ) );
        ElemN = Elem.GetChild("CONTROLLER_BUTTON_B");
        if (ElemN.NotNull()) remap_button ( CONTROLLER_BUTTON_B, atoi( ElemN.GetValue().CString() ) );
        ElemN = Elem.GetChild("CONTROLLER_BUTTON_X");
        if (ElemN.NotNull()) remap_button ( CONTROLLER_BUTTON_X, atoi( ElemN.GetValue().CString() ) );
        ElemN = Elem.GetChild("CONTROLLER_BUTTON_Y");
        if (ElemN.NotNull()) remap_button ( CONTROLLER_BUTTON_Y, atoi( ElemN.GetValue().CString() ) );
        ElemN = Elem.GetChild("CONTROLLER_BUTTON_BACK");
        if (ElemN.NotNull()) remap_button ( CONTROLLER_BUTTON_BACK, atoi( ElemN.GetValue().CString() ) );
        ElemN = Elem.GetChild("CONTROLLER_BUTTON_GUIDE");
        if (ElemN.NotNull()) remap_button ( CONTROLLER_BUTTON_GUIDE, atoi( ElemN.GetValue().CString() ) );
        ElemN = Elem.GetChild("CONTROLLER_BUTTON_START");
        if (ElemN.NotNull()) remap_button ( CONTROLLER_BUTTON_START, atoi( ElemN.GetValue().CString() ) );
        ElemN = Elem.GetChild("CONTROLLER_BUTTON_LEFTSTICK");
        if (ElemN.NotNull()) remap_button ( CONTROLLER_BUTTON_LEFTSTICK,atoi( ElemN.GetValue().CString() ) );
        ElemN = Elem.GetChild("CONTROLLER_BUTTON_RIGHTSTICK");
        if (ElemN.NotNull()) remap_button ( CONTROLLER_BUTTON_RIGHTSTICK, atoi( ElemN.GetValue().CString() ));
        ElemN = Elem.GetChild("CONTROLLER_BUTTON_LEFTSHOULDER");
        if (ElemN.NotNull()) remap_button ( CONTROLLER_BUTTON_LEFTSHOULDER, atoi( ElemN.GetValue().CString() ) );
        ElemN = Elem.GetChild("CONTROLLER_BUTTON_RIGHTSHOULDER");
        if (ElemN.NotNull()) remap_button ( CONTROLLER_BUTTON_RIGHTSHOULDER, atoi( ElemN.GetValue().CString() ));
        ElemN = Elem.GetChild("CONTROLLER_BUTTON_DPAD_UP");
        if (ElemN.NotNull()) remap_button ( CONTROLLER_BUTTON_DPAD_UP, atoi( ElemN.GetValue().CString() ) );
        ElemN = Elem.GetChild("CONTROLLER_BUTTON_DPAD_DOWN");
        if (ElemN.NotNull()) remap_button ( CONTROLLER_BUTTON_DPAD_DOWN, atoi( ElemN.GetValue().CString() ) );
        ElemN = Elem.GetChild("CONTROLLER_BUTTON_DPAD_LEFT");
        if (ElemN.NotNull()) remap_button ( CONTROLLER_BUTTON_DPAD_LEFT, atoi( ElemN.GetValue().CString() ) );
        ElemN = Elem.GetChild("CONTROLLER_BUTTON_DPAD_RIGHT");
        if (ElemN.NotNull()) remap_button ( CONTROLLER_BUTTON_DPAD_RIGHT, atoi( ElemN.GetValue().CString() ));
        ElemN = Elem.GetChild("DA_LEFT_TRIGGER");
        if (ElemN.NotNull()) remap_button ( DA_LEFT_TRIGGER, atoi( ElemN.GetValue().CString() ) );
        ElemN = Elem.GetChild("DA_RIGHT_TRIGGER");
        if (ElemN.NotNull()) remap_button ( DA_RIGHT_TRIGGER, atoi( ElemN.GetValue().CString() ) );
        ElemN = Elem.GetChild("DA_HAT_UP");
        if (ElemN.NotNull()) remap_button ( DA_HAT_UP, atoi( ElemN.GetValue().CString() ) );
        ElemN = Elem.GetChild("DA_HAT_DOWN");
        if (ElemN.NotNull()) remap_button ( DA_HAT_DOWN, atoi( ElemN.GetValue().CString() ) );
        ElemN = Elem.GetChild("DA_HAT_LEFT");
        if (ElemN.NotNull()) remap_button ( DA_HAT_LEFT, atoi( ElemN.GetValue().CString() ) );
        ElemN = Elem.GetChild("DA_HAT_RIGHT");
        if (ElemN.NotNull()) remap_button ( DA_HAT_RIGHT, atoi( ElemN.GetValue().CString() ) );
    }
}
