/// Console Game Controller handling 

// Additional enumerations for controller features
const int DA_LEFT_TRIGGER = 15;
const int DA_RIGHT_TRIGGER = 16;
const int DA_HAT_UP = 17;
const int DA_HAT_DOWN = 18;
const int DA_HAT_LEFT = 19;
const int DA_HAT_RIGHT = 20;
const int DA_LAST = 21;

virtualController@ myjoystick_ = virtualController();  // make a controller

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
		if ( !cache.Exists ("Resources/Settings/dajoystick.xml") ) return; // no file, no error in console
		
        XMLFile@ ujoy = cache.GetResource("XMLFile", "Resources/Settings/dajoystick.xml");
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


// look for a config file to normalize the controller button functions.
// and set up the event handlers in case 1 joystick is connected
void CreateGameControllers()
{
 	if ( input.numJoysticks > 0 )  // is there a game controller plugged in?
    {
       myjoystick_.load_user_settings();
       SubscribeToEvent("JoystickButtonDown", "HandleButtonDown");
       SubscribeToEvent("JoystickButtonUp", "HandleButtonUp");
       if ( myjoystick_.button(DA_HAT_UP) > -1 ) // xbox uses both dpad and hat, (and analoge)!
         SubscribeToEvent("JoystickHatMove", "HandleHatMove");
    }
}

// continuous action function. This is needed because unlike keyboard keys,
// the joystick buttons do not auto-repeat. That not being bad enough, if
// a linear function is used to move around, it wont be fast enough to get
// to whats behind you, so the longer you hold down a movement button, the
// faster you move, up unto a point. You also want fine control to dial in
// aiming, so the first couple of updates after a button is held tries to 
// limit the number of steps moved, hopefully to improve the aiming.
void joystickUpdate ( int position )
{
	if (input.numJoysticks == 0 || position == -1 || ( myjoystick_ is null ) ) return;

    // reset the counter if the controller emits 0, or the button pressed changes
    if ( position == 0 || myjoystick_.updatevalue_ != position )
       myjoystick_.updatecounter_ = 0;

    int move = 1; // start out with 1 clik of movement
	int dx = 0;
	int dy = 0;

	// we need a real easing function...
	
	// fine control starting out, step 0, 4, 7, 9 on are let thru.
	if ( myjoystick_.updatecounter_ == 1 ) move = 0;
	if ( myjoystick_.updatecounter_ == 2 ) move = 0;
	if ( myjoystick_.updatecounter_ == 3 ) move = 0;
	if ( myjoystick_.updatecounter_ == 5 ) move = 0;
	if ( myjoystick_.updatecounter_ == 6 ) move = 0;
	if ( myjoystick_.updatecounter_ == 8 ) move = 0;
	
	// values to speed up, if the same button is held
	if ( myjoystick_.updatecounter_ > 20 ) move = 2;
	if ( myjoystick_.updatecounter_ > 80 ) move = 3;
	if ( myjoystick_.updatecounter_ > 170 ) move = 5;
	if ( myjoystick_.updatecounter_ > 223 ) move = 7;
	if ( myjoystick_.updatecounter_ > 666 ) move = 10;
	if ( myjoystick_.updatecounter_ > 999 ) move = 12;
	
	// find out where to go for direction buttons
	if ( position == myjoystick_.button(CONTROLLER_BUTTON_DPAD_UP)) {  dx= 0; dy =-move; }
	else if ( position == myjoystick_.button(CONTROLLER_BUTTON_DPAD_LEFT) ) { dx= -move; dy =0; }
	else if ( position == myjoystick_.button(CONTROLLER_BUTTON_DPAD_DOWN) ) { dx= 0; dy =move; }
	else if ( position == myjoystick_.button(CONTROLLER_BUTTON_DPAD_RIGHT) ) { dx= move; dy =0; }
	// or hat direction
	else if ( position == myjoystick_.button(DA_HAT_UP)) {  dx= 0; dy =-move; }
	else if ( position == myjoystick_.button(DA_HAT_LEFT) ) { dx= -move; dy =0; }
	else if ( position == myjoystick_.button(DA_HAT_DOWN) ) { dx= 0; dy =move; }
	else if ( position == myjoystick_.button(DA_HAT_RIGHT) ) { dx= move; dy =0; }

	if ( move > 0 )  // adjust the camera and hud settings
	{
       float camYaw = cameraNode_.rotation.yaw + (dx * 0.25f);
	   float camPitch = cameraNode_.rotation.pitch + (dy * 0.25f);
	   camPitch = Clamp(camPitch, -20.0f, 70.0f);
	   cameraNode_.rotation = Quaternion(camPitch, camYaw, 0.0f);
	   radarScreenBase_.rotation = -cameraNode_.worldRotation.yaw;
    }
    
    // keep track of pseudo easing values 
    myjoystick_.updatevalue_ = position;  
    myjoystick_.updatecounter_ ++;
}

// Some types of joysticks handle the hat, which is the direction pad,
// as a separate unit from the buttons. Why, I dont know. If the hat is
// used then the controller will not emit DPAD button presses. The hat
// presses will be handled separately because the numbers it produces 
// conflict with button numbers.
void HandleHatMove(StringHash eventType, VariantMap& eventData)
{	    
	if(gameState_ != GS_INGAME) // if we are not playing, dont get new positions
	{
		return;
	}
	
	joydirection_ = eventData["Position"].GetInt();

    // for hats that can do both x + y directions, make it do x direction instead, otherwise 
    // the continuous action studders.  
	if ( joydirection_ == 9 || joydirection_ == 12 ) joydirection_ = myjoystick_.button(DA_HAT_LEFT);  
	else if ( joydirection_ == 3 || joydirection_ == 6) joydirection_ = myjoystick_.button(DA_HAT_RIGHT);

}

// Get the joystick button presses. For controllers without the hat, the direction
// values will go thru here, as does fire control, and pause, exit.
void HandleButtonDown(StringHash eventType, VariantMap& eventData)
{
	int jsButton = eventData["Button"].GetInt();

    if ( myjoystick_ is null ) return; 
     
	if ( jsButton == myjoystick_.button(CONTROLLER_BUTTON_DPAD_UP)
	|| jsButton == myjoystick_.button(CONTROLLER_BUTTON_DPAD_LEFT) 
	|| jsButton == myjoystick_.button(CONTROLLER_BUTTON_DPAD_DOWN) 
	|| jsButton == myjoystick_.button(CONTROLLER_BUTTON_DPAD_RIGHT) )
	{
		joydirection_ = jsButton;  // feed the direction and bolt
		return; 
	}

    // select/back exits always
	if( jsButton == myjoystick_.button(CONTROLLER_BUTTON_BACK) ) 
	{
		onQuit_ = true;
	}

    // start in game pauses, unpauses
	if( jsButton == myjoystick_.button(CONTROLLER_BUTTON_START) && (gameState_ == GS_INGAME || gameState_ == GS_PAUSED) ) 
	{
        PauseGame();
	}

	//  fire only in game
	if( jsButton == myjoystick_.button(CONTROLLER_BUTTON_X) && ( gameState_ == GS_INGAME)) 
	{
		Fire();
	}
}

// To make the continuous action work with the DPAD buttons, we have
// to detect a DPAD button up event and send a 0 for the direction,
// to enable a stop in direction.
void HandleButtonUp(StringHash eventType, VariantMap& eventData)
{
	int jsButton = eventData["Button"].GetInt();

    if ( myjoystick_ is null ) return;
     
    // when one of these buttons goes up, set the direction to 0 so the
    // continuous control stops. 
	if ( jsButton == myjoystick_.button(CONTROLLER_BUTTON_DPAD_UP)
	|| jsButton == myjoystick_.button(CONTROLLER_BUTTON_DPAD_LEFT) 
	|| jsButton == myjoystick_.button(CONTROLLER_BUTTON_DPAD_DOWN) 
	|| jsButton == myjoystick_.button(CONTROLLER_BUTTON_DPAD_RIGHT) )
	{
		joydirection_ = 0;  // stop the spinning
	}
}
