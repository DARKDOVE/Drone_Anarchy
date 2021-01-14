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


#include "InputController.as"

//Level Status
const int LSTATUS_NORMAL = 0;
const int LSTATUS_QUIT = 1;
const int LSTATUS_SUSPEND = 2;


//Level Manager Events
const int EVT_UPDATE = 1;
const int EVT_KEYDOWN = 2;
const int EVT_MOUSECLICK = 3;
const int EVT_MOUSEMOVE = 4;
const int EVT_SOUNDFINISH = 5;
const int EVT_JOYSTICK_BUTTONDOWN = 6;
const int EVT_JOYSTICK_BUTTONUP = 7;
const int EVT_JOYSTICK_HATMOVE = 8;
const int EVT_WEB_WINDOW_RESIZED = 9;

	
//Bullet Physics Mask
const int BULLET_COLLISION_LAYER = 1;
const int PLAYER_COLLISION_LAYER = 2;
const int DRONE_COLLISION_LAYER = 3;
const int FLOOR_COLLISION_LAYER = 5;
const int SCORE_ADDITION_RATE = 1;

enum LevelState
{
	LS_INGAME = 101,
	LS_OUTGAME,
	LS_PAUSED,
    LS_FIRSTRUN,
    LS_COUNTDOWN
}


abstract class LevelManager : ScriptObject
{
	protected bool isInitialised = false;
	protected bool isActivated = false;
	protected SoundSource@ backgroundMusicSource_;
	
		
	bool initialised
	{
		get const
		{
			return isInitialised;
		}
	}
	
	
	bool activated
	{
		get const
		{
			return isActivated;
		}
	}
	
	
	void Initialise()
	{
		if(isInitialised)
		{
			return;
		}
		CreateAudioSystem();
		isInitialised = true;
	}
	
	void Activate()
	{		
		if(!isInitialised)
		{
			return;
		}
		
		isActivated = true;
	}

	
	void Deactivate()
	{
		if(!isInitialised)
		{
			return;
		}
		
		isActivated = false;
	}
	
		
	void InitialiseAndActivate()
	{
		Initialise();
		Activate();
	}
	
	
	void SetupLevel(){}
	
	void HandleLevelEvent(VariantMap& eventData)
	{
		int eventId = eventData["ID"].GetInt();
		
		switch(eventId)
		{
		case EVT_UPDATE:
			HandleUpdate(eventData);
			break;
		case EVT_KEYDOWN:
			HandleKeyDown(eventData);
			break;
		case EVT_MOUSEMOVE:
			HandleMouseMove(eventData);
			break;	
		case EVT_SOUNDFINISH:
			HandleSoundFinish(eventData);
			break;	
		case EVT_JOYSTICK_BUTTONDOWN:
			HandleJoystickButtonDown(eventData);
			break;	
		case EVT_JOYSTICK_BUTTONUP:
			HandleJoystickButtonUp(eventData);
			break;		
		case EVT_JOYSTICK_HATMOVE:
			HandleHatMove(eventData);
			break;		
		case EVT_WEB_WINDOW_RESIZED:
			HandleWebWindowResized(eventData);
			break;
		}
	}
	
	void HandleUpdate(VariantMap& eventData){}
	void HandleKeyDown(VariantMap& eventData){}
	void HandleMouseMove(VariantMap& eventData){}
	void HandleSoundFinish(VariantMap& eventData){}
	void HandleJoystickButtonDown(VariantMap& eventData){}
	void HandleJoystickButtonUp(VariantMap& eventData){}
	void HandleHatMove(VariantMap& eventData){}
    void HandleWebWindowResized(VariantMap& eventData){}
	
	
	protected void SetViewportCamera(Camera@ viewCamera)
	{
		renderer.viewports[0] = Viewport(scene, viewCamera);
	}
	
	
	private void CreateAudioSystem()
	{
		
		Node@ backgroundMusicNode = scene.CreateChild();
		backgroundMusicSource_ = backgroundMusicNode.CreateComponent("SoundSource");
		backgroundMusicSource_.soundType = SOUND_MUSIC;
		
	}
	
	
	protected void SetSoundListener(Node@ listenerNode)
	{
		if(listenerNode.GetComponent("SoundListener") is null)
		{
			return;
		}
		audio.listener = listenerNode.GetComponent("SoundListener");
	}
	
}


//=========================== LEVEL ONE MANAGER ==========================================


class LevelOneManager : LevelManager
{

	uint MAX_DRONE_COUNT = 15;

	float EASY_PHASE = 0;
	float MODERATE_PHASE = 60;
	float CRITICAL_PHASE = 120;
	float EASY_PHASE_RATE = 3.5;
	float MODERATE_PHASE_RATE = 2.5;
	float CRITICAL_PHASE_RATE = 1;
	float SCENE_TO_UI_SCALE = 1.6f;
	float SPRITE_UPDATE_TIME = 0.04f;

	String NORMAL_DRONE_SPRITE = "Textures/drone_sprite.png";
	String ALTERNATE_DRONE_SPRITE = "Textures/alt_drone_sprite.png";
	
	int playerScore_ = 0;

	float spriteUpdateCounter_ = 0.0f;
	float droneSpawnCounter_ = 0.0f;
	float gamePhaseCounter_ = 0.0f;
    float tempCounterSpeed_ = 0.0f;
	
	
	bool playerDestroyed_ = false;

	int joydirection_ = -1;  // which way the game controller wants to go

	String optionsMessage_ = "<SPACE> To Replay | <ESC> To Quit";

	LevelState levelState_ = LS_FIRSTRUN;

    bool isWeb_ = GetPlatform() == "Web";

	Node@ cameraNode_;
	Node@ playerNode_;

	Viewport@ viewport_ = renderer.viewports[0];


	ValueAnimation@ damageAnimation_;
	ValueAnimation@ textAnimation_;

	Sprite@ radarScreenBase_;
	Sprite@ healthFillSprite_;
	Sprite@ targetSprite_;

	Text@ enemyCounterText_;
	Text@ playerScoreText_;
	Text@ statusText_;
	Text@ playerScoreMessageText_;
	Text@ optionsInfoText_;

    Sound@ backgroundMusic_ = cache.GetResource("Sound","Sounds/cyber_dance.ogg");
    Sound@ defeatMusic_ = cache.GetResource("Sound","Sounds/defeated.ogg");

    UIElement@ displayRoot_;
	
	virtualController@ myjoystick_ = virtualController();
	void Activate()
	{
		LevelManager::Activate();
	}
	
	void Deactivate()
	{
		LevelManager::Deactivate();

        if( levelState_ == LS_INGAME )
        {
            ToggleGamePause();
        }
        else if( levelState_ == LS_COUNTDOWN)
        {
            tempCounterSpeed_ = statusText_.GetAttributeAnimationSpeed("Text");
            statusText_.SetAttributeAnimationSpeed("Text", 0.0f);
        }
        else
        {
		    scene.updateEnabled = false;
        }

        if( displayRoot_ !is null )
        {
            displayRoot_.visible = false;
        }

        backgroundMusicSource_.Stop();
	}
	

    void StartOrResumeLevel()
    {
        if( levelState_ == LS_FIRSTRUN )
        {
            levelState_ = LS_OUTGAME;

		    SetupLevel();
        }
        else
        {
            Activate();
            renderer.viewports[0] = viewport_;

            if( levelState_ == LS_OUTGAME)
            {
                PlayDefeatMusic();
            }
            else
            {
                if(levelState_ == LS_COUNTDOWN)
                    statusText_.SetAttributeAnimationSpeed("Text", tempCounterSpeed_);

                PlayBackgroundMusic();
            }

            if( displayRoot_ !is null )
            {
                displayRoot_.visible = true;
            }
            
        }
    }

	void SetupLevel()
	{
		LoadDisplayInterface();
		LoadBackgroundResources();
		LoadAttributeAnimations();
		CreateGameControllers();
		SetupScene();
        CreateSkyBox();
		CreateCameraAndLight();
		SubscribeToEvents();
		StartGame();
	}
	
	private void SetupScene()
	{
		scene.updateEnabled = false;
	}

    private void CreateSkyBox()
    {
            
        Node@ skyNode = scene.CreateChild("Sky");
        Skybox@ skybox = skyNode.CreateComponent("Skybox");

        skybox.model = cache.GetResource("Model", "Models/box.mdl");
        skybox.material = cache.GetResource("Material", "Materials/level_one_sky_box.xml");
    }
	
	void LoadDisplayInterface()
	{
		displayRoot_ = ui.root.CreateChild("UIElement");
		
		displayRoot_.LoadXML(cache.GetFile("UI/ScreenDisplay.xml"));
		
		//Load the various UI Elements
		healthFillSprite_ = displayRoot_.GetChild("HealthFill", true);
		radarScreenBase_ = displayRoot_.GetChild("RadarScreenBase");
		
		targetSprite_ = displayRoot_.GetChild("Target");
		
		enemyCounterText_ = displayRoot_.GetChild("EnemyCounter");
		playerScoreText_ = displayRoot_.GetChild("PlayerScore");
		
		
		statusText_ = displayRoot_.GetChild("StatusText");
		playerScoreMessageText_ = displayRoot_.GetChild("ScoreMessage");
		optionsInfoText_ = displayRoot_.GetChild("OptionInfo");

        IntVector2 rect = graphics.size;
		displayRoot_.SetSize(rect.x, rect.y);


	}
	
	
	void LoadBackgroundResources()
	{
		cache.BackgroundLoadResource("Model","Models/drone_body.mdl");
		cache.BackgroundLoadResource("Model","Models/drone_arm.mdl");
		cache.BackgroundLoadResource("Animation","Models/open_arm.ani");
		cache.BackgroundLoadResource("Animation","Models/close_arm.ani");
		
		cache.BackgroundLoadResource("Texture2D", "Textures/explosion.png");
		
		cache.BackgroundLoadResource("ParticleEffect", "Particles/bullet_particle.xml");
		cache.BackgroundLoadResource("ParticleEffect", "Particles/explosion.xml");
		
		cache.BackgroundLoadResource("Material","Materials/drone_arm.xml");
		cache.BackgroundLoadResource("Material","Materials/drone_body.xml");
		cache.BackgroundLoadResource("Material", "Materials/bullet_particle.xml");
		cache.BackgroundLoadResource("Material", "Materials/explosion.xml");
		
		cache.BackgroundLoadResource("Texture2D", "Textures/drone_sprite.png");
		cache.BackgroundLoadResource("Texture2D", "Textures/health_bar_green.png");
		cache.BackgroundLoadResource("Texture2D", "Textures/health_bar_red.png");
		cache.BackgroundLoadResource("Texture2D", "Textures/health_bar_yellow.png");
		
		cache.BackgroundLoadResource("Sound", "Sounds/boom1.wav");
		
		
	}


	void LoadAttributeAnimations()
	{
		textAnimation_ = cache.GetResource("ValueAnimation", "AttributeAnimations/GameStartCounterAnimation.xml");
		damageAnimation_ = cache.GetResource("ValueAnimation", "AttributeAnimations/DamageWarningAnimation.xml");
	}
	
	
	void CreateCameraAndLight()
	{
		cameraNode_ = scene.CreateChild();
		cameraNode_.CreateComponent("Camera");
		cameraNode_.Translate(Vector3(0,1.7,0));
		
		Node@ lightNode = cameraNode_.CreateChild("DirectionalLight");
		lightNode.direction = Vector3(0.6f, -1.0f, 0.8f);
		Light@ light = lightNode.CreateComponent("Light");
		light.lightType = LIGHT_DIRECTIONAL;	
		
		cameraNode_.CreateComponent("SoundListener");
		
        viewport_ = Viewport(scene, cameraNode_.GetComponent("Camera"));

        renderer.viewports[0] = viewport_;

		if ( !isWeb_ )
        {
            RenderPath@ rPath = viewport_.renderPath;
            rPath.Append(cache.GetResource("XMLFile", "PostProcess/Blur.xml"));
            rPath.SetEnabled("Blur",true);
        }
	}
	
	
	void CreatePlayer()
	{
		
		playerNode_ = scene.CreateChild("PlayerNode");
		Node@ playerCameraNode = playerNode_.CreateChild("CameraNode");
		playerCameraNode.CreateComponent("Camera");
		playerCameraNode.Translate(Vector3(0,1.7,0));
		
		ScriptFile@ sFile = cache.GetResource("ScriptFile","Scripts/GameObjects.as");
		playerNode_.CreateScriptObject(sFile,"PlayerObject");
		
		playerNode_.AddTag("player");

		playerCameraNode.CreateComponent("SoundListener");
		SetSoundListener(playerCameraNode);

        viewport_.camera =  playerCameraNode.GetComponent("Camera") ;
		
		playerDestroyed_ = false;

	}
	
	

	void SubscribeToEvents()
	{
		SubscribeToEvent("PlayerHit","HandlePlayerHit");
		SubscribeToEvent("DroneDestroyed", "HandleDroneDestroyed");
		SubscribeToEvent("CountFinished", "HandleCountFinished");
		SubscribeToEvent("SoundGenerated", "HandleSoundGenerated");
		SubscribeToEvent("PlayerDestroyed", "HandlePlayerDestroyed");
		SubscribeToEvent("PlayerHealthUpdate", "HandlePlayerHealthUpdate");
		SubscribeToEvent(scene.physicsWorld, "PhysicsPreStep", "HandleFixedUpdate");
		
	}

	
	
	void StartCounterToGame()
	{
        levelState_ = LS_COUNTDOWN;
		statusText_.SetAttributeAnimation("Text", textAnimation_,WM_ONCE);
	}
	
	void StartGame()
	{
		playerScoreMessageText_.text = "";
		optionsInfoText_.text = "";
		gamePhaseCounter_ = 0.0f;
		droneSpawnCounter_ = 0.0f;
		playerScore_ = 0;
		
		SetSoundListener(cameraNode_);

		PlayBackgroundMusic();
		StartCounterToGame();
	}

	
	void InitiateGameOver()
	{
		scene.updateEnabled = false;
		levelState_ = LS_OUTGAME;
		
		CleanupScene();
		
		if ( !isWeb_ )
        {
		    renderer.viewports[0].renderPath.SetEnabled("Blur",true);
        }
		
		PlayDefeatMusic();
		
		targetSprite_.visible = false;
		statusText_.text = "YOU FAILED";
		playerScoreMessageText_.text = "Score : " + String(playerScore_);
		optionsInfoText_.text = optionsMessage_ ;

	}
	
	
	void CleanupScene()
	{
		//Remove All Nodes with script object : Drones, Bullets and even the player
		Array<Node@> scriptedNodes = scene.GetChildrenWithScript(true);
		for(uint i=0; i < scriptedNodes.length ; i++)
		{
			Node@ scriptNode = scriptedNodes[i];
			Sprite@ nodeSprite = scriptNode.vars["Sprite"].GetPtr();
			
			if(nodeSprite !is null)
			{
				nodeSprite.Remove();
			}
			
			scriptNode.Remove();
		}
		
		
		//Hide the enemy counter and player score texts
		enemyCounterText_.text = "";
		playerScoreText_.text = "";

	}
	
	

	void HandlePlayerHit()
	{
		//Show Warning
		radarScreenBase_.SetAttributeAnimation("Color", damageAnimation_, WM_ONCE);
		PlaySoundFX(cameraNode_,"Sounds/boom5.ogg");
	}

	
	void HandleDroneDestroyed(StringHash eventType, VariantMap& eventData)
	{
		playerScore_ += eventData["DronePoint"].GetInt();
		UpdateScoreDisplay();
	}
	
	
	void HandleCountFinished()
	{
		CreatePlayer();
		
		cameraNode_.GetChild("DirectionalLight").enabled = false;
		
		scene.updateEnabled = true;
		levelState_ = LS_INGAME;
		
		targetSprite_.visible = true;
		enemyCounterText_.text = 0;
		playerScoreText_.text = 0;
		
		if ( !isWeb_ )
        {
		    renderer.viewports[0].renderPath.SetEnabled("Blur",false);
        }
	}
	
	
	void HandleSoundGenerated(StringHash eventType, VariantMap& eventData)
	{
		Node@ soundNode = eventData["SoundNode"].GetPtr();
		String soundName = eventData["SoundName"].GetString();
		
		PlaySoundFX(soundNode, soundName);
	}
	
	
	void HandlePlayerDestroyed(StringHash eventType, VariantMap& eventData)
	{
		playerDestroyed_ = true;
		Vector3 camPosition = eventData["CamPosition"].GetVector3();
		Quaternion camRotation = eventData["CamRotation"].GetQuaternion();
		cameraNode_.worldRotation = camRotation;
		cameraNode_.worldPosition = camPosition;
		
		SetViewportCamera(cameraNode_.GetComponent("Camera"));
		cameraNode_.GetChild("DirectionalLight").enabled = true;
		SetSoundListener(cameraNode_);
	}
	
	
	void HandlePlayerHealthUpdate(StringHash eventType, VariantMap& eventData)
	{
		//Update Health
		float playerHealthFraction = eventData["CurrentHealthFraction"].GetFloat();
		
		int range = 512 - int( 512 * playerHealthFraction);
		healthFillSprite_.imageRect = IntRect(range, 0, 512 + range, 64);
		UpdateHealthTexture(playerHealthFraction);
	}
	
	
	void HandleFixedUpdate(StringHash eventType, VariantMap& eventData)
	{
		float timeStep = eventData["TimeStep"].GetFloat();
		float droneSpawnRate = 0.0;
			
		gamePhaseCounter_ += timeStep;
		if(gamePhaseCounter_ >= CRITICAL_PHASE)
		{
			droneSpawnRate = CRITICAL_PHASE_RATE;
			gamePhaseCounter_ = CRITICAL_PHASE;		
		}
		else if(gamePhaseCounter_ >= MODERATE_PHASE)
		{
			droneSpawnRate = MODERATE_PHASE_RATE;
		}
		else
		{
			droneSpawnRate = EASY_PHASE_RATE;
		}
		
		
		
		droneSpawnCounter_ +=timeStep;
		if(droneSpawnCounter_ >= droneSpawnRate)
		{
			if(GetDroneCount() < MAX_DRONE_COUNT)
			{
				SpawnDrone();
				UpdateDroneSprites();
				droneSpawnCounter_ = 0;
			}
		}
		
		
		spriteUpdateCounter_ += timeStep;
		
		if(spriteUpdateCounter_ >= SPRITE_UPDATE_TIME)
		{
			UpdateDroneSprites();
			spriteUpdateCounter_ = 0;
		}
		

	}
	
	private void HandleUpdate(VariantMap& eventData)
	{
		if(playerDestroyed_ && levelState_ == LS_INGAME)
		{
			InitiateGameOver();
        }
		else if(levelState_ == LS_INGAME)
		{
            HandleMouseClick();
			joystickUpdate(joydirection_);
		}
	}
	
	void HandleKeyDown(VariantMap& eventData)
	{
		int key = eventData["Key"].GetInt();	
		
        //only handle escape key for non web platforms since
        //for web platform escape key will trigger losing pointer lock
		if(key == KEY_ESCAPE && !isWeb_)
			globalVars["STATUS_ID"] = LSTATUS_QUIT;
		else if(levelState_ == LS_OUTGAME)
		{
			HandleKeyOnOutGame(key);
		}
		else 
		{
			HandleKeyOnInGame(key);
		}
	}
	
	void HandleWebWindowResized(VariantMap& eventData)
	{
		IntVector2 rect = eventData["CurrentWebWindowSize"].GetIntVector2();	
		displayRoot_.SetSize(rect.x, rect.y);
	}
	
	
	void HandleKeyOnOutGame(int key)
	{
		if(key == KEY_SPACE)
		{
			StartGame();
		}
	}

	void HandleKeyOnInGame(int key)
	{
		if(key == KEY_P)
		{
			ToggleGamePause();
		}
	}
	
	
	void HandleMouseMove(VariantMap& eventData)
	{
		if(levelState_ != LS_INGAME)
		{
			return;
		}
		
		int dx = eventData["DX"].GetInt();
		int dy = eventData["DY"].GetInt();

		RotatePlayer(dx, dy);
	}
	
	
	
	void HandleMouseClick()
	{
		if(levelState_ != LS_INGAME)
		{
			return;
		}
		
		//int mouseButton = eventData["Button"].GetInt();
		
		if(input.mouseButtonPress[MOUSEB_LEFT])
		{
			Fire();
		}
	}
	
	
	void HandleSoundFinish(VariantMap& eventData)
	{	
		Node@ soundNode = eventData["Node"].GetPtr();
		soundNode.RemoveComponent("SoundSource3D");
	}
	

	void PlayBackgroundMusic()
	{
		if(backgroundMusic_ is null)
			return;
			
		backgroundMusic_.looped = true;
		backgroundMusicSource_.Play(backgroundMusic_);
	}


	void PlayDefeatMusic()
	{
		if(defeatMusic_ is null)
			return;
			
		defeatMusic_.looped = true;
		backgroundMusicSource_.Play(defeatMusic_);
	}


    void StopBackgroundMusic()
    {
        backgroundMusicSource_.Stop();
    }
	
	
	void PlaySoundFX(Node@ soundNode, String soundName )
	{
		SoundSource3D@ source = soundNode.CreateComponent("SoundSource3D");
		
		Sound@ sound = cache.GetResource("Sound", soundName);
		source.SetDistanceAttenuation(0.2, 120, 0.1);
		source.soundType = SOUND_EFFECT;
		source.Play(sound);
	}
	
	
	void SpawnDrone()
	{
		Node@ droneNode = scene.CreateChild();
		
		XMLFile@ file = cache.GetResource("XMLFile", "Objects/LowLevelDrone.xml");
		droneNode.LoadXML(file.root);
		
		droneNode.vars["Sprite"] = CreateDroneSprite(NORMAL_DRONE_SPRITE);
	}


	Sprite@ CreateDroneSprite(String spriteTexture)
	{
		Texture2D@ droneSpriteTex = cache.GetResource("Texture2D", spriteTexture);
		Sprite@ droneSprite = radarScreenBase_.CreateChild("Sprite");
		
		droneSprite.texture = droneSpriteTex;
		droneSprite.SetSize(6,6);
		droneSprite.SetAlignment(HA_CENTER, VA_CENTER);
		droneSprite.SetHotSpot(3,3);
		droneSprite.blendMode = BLEND_ALPHA;
		droneSprite.priority = 1;
		
		return droneSprite;
	}
	

	
	void UpdateHealthTexture(float healthFraction)
	{
		if(healthFraction > 0.5)
		{
			healthFillSprite_.texture = cache.GetResource("Texture2D", "Textures/health_bar_green.png");
		}
		else if(healthFraction > 0.2)
		{
			healthFillSprite_.texture = cache.GetResource("Texture2D", "Textures/health_bar_yellow.png");
		}
		else
		{
			healthFillSprite_.texture = cache.GetResource("Texture2D", "Textures/health_bar_red.png");
		}
	}
	
	
	
	void UpdateDroneSprites()
	{
		Array<Node@> scriptNodes = scene.GetChildrenWithTag("drone",true);
		
		for(uint i=0; i < scriptNodes.length ; i++)
		{
			Node@ droneNode = scriptNodes[i];
			Sprite@ nodeSprite = droneNode.vars["Sprite"].GetPtr();
			
			if(nodeSprite !is null)
			{
				Vector3 relativePos = droneNode.worldPosition - playerNode_.worldPosition ;
				nodeSprite.position = Vector2(relativePos.x, -(relativePos.z))* SCENE_TO_UI_SCALE;
			}
		
		}
		
		enemyCounterText_.text = scriptNodes.length;;
		
	}
	

	void UpdateScoreDisplay()
	{
		playerScoreText_.text = playerScore_;
	}
	
	 
	void RotatePlayer(int dx, int dy)
	{
		VariantMap eventData;
		eventData["DX"] = dx;
		eventData["DY"] = dy;
		SendEvent("PlayerRotation", eventData);
		
		radarScreenBase_.rotation = -playerNode_.worldRotation.yaw;
	}

	
	void Fire()
	{	
		SendEvent("ActivateWeapon");
	}
	
	
	void ToggleGamePause()
	{
		scene.updateEnabled = !scene.updateEnabled;
		
		if(scene.updateEnabled)
		{
			statusText_.text = "";
			levelState_ = LS_INGAME;
		}
		else
		{
			statusText_.text = "PAUSED";
			levelState_ = LS_PAUSED;
		}
		
		targetSprite_.visible = scene.updateEnabled;
	}
	
	int GetDroneCount()
	{
		Array<Node@> scriptNodes = scene.GetChildrenWithTag("drone",true);
		return scriptNodes.length;
	}

	
	
	
	// look for a config file to normalize the controller button functions.
	// and set up the event handlers in case 1 joystick is connected
	void CreateGameControllers()
	{
		if ( input.numJoysticks > 0 )  // is there a game controller plugged in?
		{
		   myjoystick_.load_user_settings();
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

		if ( move > 0 )  // Adjust the player view
		{
		   RotatePlayer(dx, dy);
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
	void HandleHatMove(VariantMap& eventData)
	{	    
	
		if ( myjoystick_.button(DA_HAT_UP) <= -1 )
			return;
		
			
		if(levelState_ != LS_INGAME) // if we are not playing, dont get new positions
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
	void HandleJoystickButtonDown(VariantMap& eventData)
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
			globalVars["STATUS_ID"] = LSTATUS_QUIT;
		}

		// start in game pauses, unpauses
		if( jsButton == myjoystick_.button(CONTROLLER_BUTTON_START) && (levelState_ == LS_INGAME || levelState_ == LS_PAUSED) ) 
		{
			ToggleGamePause();
		}

		//  fire only in game
		if( jsButton == myjoystick_.button(CONTROLLER_BUTTON_X) && ( levelState_ == LS_INGAME)) 
		{
			Fire();
		}
	}

	// To make the continuous action work with the DPAD buttons, we have
	// to detect a DPAD button up event and send a 0 for the direction,
	// to enable a stop in direction.
	void HandleJoystickButtonUp(VariantMap& eventData)
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
	
}