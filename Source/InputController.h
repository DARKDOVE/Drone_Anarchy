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



#ifndef __INPUTCONTROLLER_H_
#define __INPUTCONTROLLER_H_


// Additional enumerations for controller features
const int DA_LEFT_TRIGGER = 15;
const int DA_RIGHT_TRIGGER = 16;
const int DA_HAT_UP = 17;
const int DA_HAT_DOWN = 18;
const int DA_HAT_LEFT = 19;
const int DA_HAT_RIGHT = 20;
const int DA_LAST = 21;

using namespace Urho3D;

// Contain the game controller functionality
class virtualController
{


   public :
    virtualController();  // xbox 360 button mapping is default
    int button ( int index ) const; // return actual controllers button value
    void load_user_settings( ResourceCache* rcache ); // read settings out of a user prepared configuration file

    int updatecounter_; // used for pseudo easing movement
    int updatevalue_;  // used for pseudo easing movement

   private:
     int lookup[DA_LAST];  // local button database
     void remap_button ( int index, int value ); // overlay an actual controllers button, hat (if it's got one) mapping
     void clearAll(); // clear button settings to be invalid, we'll fix them later
     void make_ps3(); // A PS3 usb controller
     void make_2in1();  // MYPOWER 2in1 (cheap) usb controller

};


#endif // #ifndef __INPUTCONTROLLER_H_
