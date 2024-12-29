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

/// Console Game Controller handling 

// Additional enumerations for controller features
const int DA_LEFT_TRIGGER = 15;
const int DA_RIGHT_TRIGGER = 16;
const int DA_HAT_UP = 17;
const int DA_HAT_DOWN = 18;
const int DA_HAT_LEFT = 19;
const int DA_HAT_RIGHT = 20;
const int DA_LAST = 21;

// Contain the game controller functionality 
class virtualController
{
	virtualController()  // xbox 360 button mapping is default
	{
        updatecounter_ = 0; 
        updatevalue_ = 0;
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
	void remap_button ( int index, int value )
	{
		if ( index >= CONTROLLER_BUTTON_A && index < DA_LAST )
		   lookup[index] = value;
	}
	
	// return actual controllers button value
	int button ( int index )
	{
		if ( index >= CONTROLLER_BUTTON_A && index < DA_LAST )
		  return lookup[index];
		return -1;
	}
	
	// clear button settings to be invalid, we'll fix them later
	void clearAll()
	{	
		for(uint nn=0; nn < DA_LAST; nn++)
           remap_button(nn ,-1);
	}
	
	// A PS3 usb controller
	void make_ps3()
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
    void make_2in1()
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
    void load_user_settings()
    { 
		if ( !cache.Exists ("Settings/dajoystick.xml") ) return; // no file, no error in console
		
        XMLFile@ ujoy = cache.GetResource("XMLFile", "Settings/dajoystick.xml");
		if ( ujoy !is null )
		{
			clearAll();	// start clean
			XMLElement Elem = ujoy.root; // read out settings
			if (Elem.isNull) return; 
			
			// look thru all the elements, we'll read the ones that are defined.	
			XMLElement ElemN = Elem.GetChild("CONTROLLER_BUTTON_A"); 
			if (ElemN.notNull)remap_button ( CONTROLLER_BUTTON_A, ElemN.GetValue().ToInt() );
			ElemN = Elem.GetChild("CONTROLLER_BUTTON_B");
			if (ElemN.notNull) remap_button ( CONTROLLER_BUTTON_B, ElemN.GetValue().ToInt() );
			ElemN = Elem.GetChild("CONTROLLER_BUTTON_X");
			if (ElemN.notNull) remap_button ( CONTROLLER_BUTTON_X, ElemN.GetValue().ToInt() );
			ElemN = Elem.GetChild("CONTROLLER_BUTTON_Y");
			if (ElemN.notNull) remap_button ( CONTROLLER_BUTTON_Y, ElemN.GetValue().ToInt() );
			ElemN = Elem.GetChild("CONTROLLER_BUTTON_BACK");
			if (ElemN.notNull) remap_button ( CONTROLLER_BUTTON_BACK, ElemN.GetValue().ToInt() );
			ElemN = Elem.GetChild("CONTROLLER_BUTTON_GUIDE");
			if (ElemN.notNull) remap_button ( CONTROLLER_BUTTON_GUIDE, ElemN.GetValue().ToInt() );
			ElemN = Elem.GetChild("CONTROLLER_BUTTON_START");
			if (ElemN.notNull) remap_button ( CONTROLLER_BUTTON_START, ElemN.GetValue().ToInt() );
			ElemN = Elem.GetChild("CONTROLLER_BUTTON_LEFTSTICK");
			if (ElemN.notNull) remap_button ( CONTROLLER_BUTTON_LEFTSTICK, ElemN.GetValue().ToInt() );
			ElemN = Elem.GetChild("CONTROLLER_BUTTON_RIGHTSTICK");
			if (ElemN.notNull) remap_button ( CONTROLLER_BUTTON_RIGHTSTICK, ElemN.GetValue().ToInt() );
			ElemN = Elem.GetChild("CONTROLLER_BUTTON_LEFTSHOULDER");
			if (ElemN.notNull) remap_button ( CONTROLLER_BUTTON_LEFTSHOULDER, ElemN.GetValue().ToInt() );
			ElemN = Elem.GetChild("CONTROLLER_BUTTON_RIGHTSHOULDER");
			if (ElemN.notNull) remap_button ( CONTROLLER_BUTTON_RIGHTSHOULDER, ElemN.GetValue().ToInt() );
			ElemN = Elem.GetChild("CONTROLLER_BUTTON_DPAD_UP");
			if (ElemN.notNull) remap_button ( CONTROLLER_BUTTON_DPAD_UP, ElemN.GetValue().ToInt() );
			ElemN = Elem.GetChild("CONTROLLER_BUTTON_DPAD_DOWN");
			if (ElemN.notNull) remap_button ( CONTROLLER_BUTTON_DPAD_DOWN, ElemN.GetValue().ToInt() );
			ElemN = Elem.GetChild("CONTROLLER_BUTTON_DPAD_LEFT");
			if (ElemN.notNull) remap_button ( CONTROLLER_BUTTON_DPAD_LEFT, ElemN.GetValue().ToInt() );
			ElemN = Elem.GetChild("CONTROLLER_BUTTON_DPAD_RIGHT");
			if (ElemN.notNull) remap_button ( CONTROLLER_BUTTON_DPAD_RIGHT, ElemN.GetValue().ToInt() );
			ElemN = Elem.GetChild("DA_LEFT_TRIGGER");
			if (ElemN.notNull) remap_button ( DA_LEFT_TRIGGER, ElemN.GetValue().ToInt() );
			ElemN = Elem.GetChild("DA_RIGHT_TRIGGER");
			if (ElemN.notNull) remap_button ( DA_RIGHT_TRIGGER, ElemN.GetValue().ToInt() );
			ElemN = Elem.GetChild("DA_HAT_UP");
			if (ElemN.notNull) remap_button ( DA_HAT_UP, ElemN.GetValue().ToInt() );
			ElemN = Elem.GetChild("DA_HAT_DOWN");
			if (ElemN.notNull) remap_button ( DA_HAT_DOWN, ElemN.GetValue().ToInt() );
			ElemN = Elem.GetChild("DA_HAT_LEFT");
			if (ElemN.notNull) remap_button ( DA_HAT_LEFT, ElemN.GetValue().ToInt() );
			ElemN = Elem.GetChild("DA_HAT_RIGHT");
			if (ElemN.notNull) remap_button ( DA_HAT_RIGHT, ElemN.GetValue().ToInt() );
		}
    }
    
	private int [] lookup (DA_LAST);  // local button database
    int updatecounter_; // used for pseudo easing movement
    int updatevalue_;  // used for pseudo easing movement
}
