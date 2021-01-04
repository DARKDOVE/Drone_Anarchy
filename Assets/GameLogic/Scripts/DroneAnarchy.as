#include "LevelManager.as"

LevelManager@ levelManager_;
Scene@ scene_;

void Start()
{
	
	SetWindowTitleAndIcon();

	CreateDebugHud();
	
	SubscribeToEvents();
	CreateLevel();
	
}


void CreateDebugHud()
{
	// Get default style
	XMLFile@ xmlFile = cache.GetResource("XMLFile", "UI/DefaultStyle.xml");
	if (xmlFile is null)
		return;
			
	// Create debug HUD
	DebugHud@ debugHud = engine.CreateDebugHud();
	debugHud.defaultStyle = xmlFile;
}


void SetWindowTitleAndIcon()
{
    graphics.windowIcon = cache.GetResource("Image","Textures/drone_anarchy_icon.png");
    graphics.windowTitle = "Drone Anarchy";
}


void CreateLevel()
{

	XMLFile@ file = cache.GetResource("XMLFile", "Objects/Scene.xml");
	scene_ = Scene();
	scene_.LoadXML(file.root);
	@levelManager_ = cast<LevelManager>(scene_.CreateScriptObject(scriptFile, "LevelOneManager"));
	levelManager_.InitialiseAndActivate();
	
}

void SubscribeToEvents()
{
	SubscribeToEvent("KeyDown","HandleKeyDown");
	SubscribeToEvent("Update", "HandleUpdate");
	SubscribeToEvent("MouseMove", "HandleMouseMove");
	SubscribeToEvent("MouseButtonDown", "HandleMouseClick");
	SubscribeToEvent("SoundFinished", "HandleSoundFinished");
	
	if ( input.numJoysticks > 0 )  // is there a game controller plugged in?
	{
	   SubscribeToEvent("JoystickButtonDown", "HandleJoystickButtonDown");
	   SubscribeToEvent("JoystickButtonUp", "HandleJoystickButtonUp");
	   SubscribeToEvent("JoystickHatMove", "HandleHatMove");
	}
	
}


void HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
	int key = eventData["Key"].GetInt();
	
	if(key == KEY_F2)
		debugHud.ToggleAll();
	else 
	{
		eventData["ID"] = EVT_KEYDOWN;
		levelManager_.HandleLevelEvent(eventData);
	}
		
}


void HandleUpdate(StringHash eventType, VariantMap& eventData)
{
	int statusId = globalVars["STATUS_ID"].GetInt();
	
	//Quickly reset the status variable to normal
    globalVars["STATUS_ID"] = LSTATUS_NORMAL;
	
	if(statusId == LSTATUS_QUIT)
	{
		engine.Exit();
	}
	else
	{
		eventData["ID"] = EVT_UPDATE;
		levelManager_.HandleLevelEvent(eventData);
		
	}
}

void HandleMouseMove(StringHash eventType, VariantMap& eventData)
{
	eventData["ID"] = EVT_MOUSEMOVE;
	levelManager_.HandleLevelEvent(eventData);
}


void HandleMouseClick(StringHash eventType, VariantMap& eventData)
{
	eventData["ID"] = EVT_MOUSECLICK;
	levelManager_.HandleLevelEvent(eventData);
}

void HandleSoundFinished(StringHash eventType, VariantMap& eventData)
{
	eventData["ID"] = EVT_SOUNDFINISH;
	levelManager_.HandleLevelEvent(eventData);
}

void HandleJoystickButtonDown(StringHash eventType, VariantMap& eventData)
{
	eventData["ID"] = EVT_JOYSTICK_BUTTONDOWN;
	levelManager_.HandleJoystickButtonDown(eventData);
}

void HandleJoystickButtonUp(StringHash eventType, VariantMap& eventData)
{
	eventData["ID"] = EVT_JOYSTICK_BUTTONUP;
	levelManager_.HandleJoystickButtonUp(eventData);
}

void HandleHatMove(StringHash eventType, VariantMap& eventData)
{
	eventData["ID"] = EVT_JOYSTICK_HATMOVE;
	levelManager_.HandleHatMove(eventData);
}


