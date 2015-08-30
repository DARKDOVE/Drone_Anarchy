#include "InputController.as"

enum GameState
{
	GS_INGAME = 101,
	GS_OUTGAME,
	GS_PAUSED
}


//Bullet Physics Mask
const int BULLET_COLLISION_LAYER = 1;
const int PLAYER_COLLISION_LAYER = 2;
const int DRONE_COLLISION_LAYER = 3;
const int FLOOR_COLLISION_LAYER = 5;
const int SCORE_ADDITION_RATE = 1;


const uint MAX_DRONE_COUNT = 15;

const float EASY_PHASE = 0;
const float MODERATE_PHASE = 60;
const float CRITICAL_PHASE = 120;
const float EASY_PHASE_RATE = 5;
const float MODERATE_PHASE_RATE = 3;
const float CRITICAL_PHASE_RATE = 1;
const float SCENE_TO_UI_SCALE = 1.6f;
const float SPRITE_UPDATE_TIME = 0.04f;

const String NORMAL_DRONE_SPRITE = "Resources/Textures/drone_sprite.png";
const String ALTERNATE_DRONE_SPRITE = "Resources/Textures/alt_drone_sprite.png";

int playerScore_ = 0;

float spriteUpdateCounter_ = 0.0f;
float droneSpawnCounter_ = 0.0f;
float gamePhaseCounter_ = 0.0f;

bool onQuit_ = false;
bool playerDestroyed_ = false;

int joydirection_ = -1;  // which way the game controller wants to go

String optionsMessage_ = "<SPACE> To Replay | <ESC> To Quit";

GameState gameState_ = GS_OUTGAME;

Scene@ scene_;
Node@ cameraNode_;
Node@ playerNode_;

Viewport@ viewport_;
SoundSource@ backgroundMusicSource_;


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





void Start()
{
	graphics.windowTitle = "Drone Anarchy";
	
	cache.AddResourceDir("DroneAnarchy");
	
	CreateGameControllers();   // in GameController.as
	
	
	CreateDebugHud();
	LoadDisplayInterface();
	
	//This is to prevent the pause that occurs in loading a resource for the first time
	LoadBackgroundResources();
	
	//Load Attribute animation files from disk
	LoadAttributeAnimations();
	
	CreateScene();
	CreateCameraAndLight();
	SubscribeToEvents();
	
	CreateAudioSystem();
	
	StartGame();
}


void StartGame()
{
	playerScoreMessageText_.text = "";
	optionsInfoText_.text = "";
	gamePhaseCounter_ = 0.0f;
	droneSpawnCounter_ = 0.0f;
	playerScore_ = 0;
	
	SetSoundListener(cameraNode_);

	PlayBackgroundMusic("Resources/Sounds/cyber_dance.ogg");
	StartCounterToGame();
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


void CreateCameraAndLight()
{
	cameraNode_ = scene_.CreateChild();
	cameraNode_.CreateComponent("Camera");
	cameraNode_.Translate(Vector3(0,1.7,0));
	
	Node@ lightNode = cameraNode_.CreateChild("DirectionalLight");
    lightNode.direction = Vector3(0.6f, -1.0f, 0.8f);
    Light@ light = lightNode.CreateComponent("Light");
    light.lightType = LIGHT_DIRECTIONAL;	
	
	cameraNode_.CreateComponent("SoundListener");
	
	renderer.viewports[0] = Viewport(scene_, cameraNode_.GetComponent("Camera"));
}


void SetViewportCamera(Node@ cameraNode)
{
	renderer.viewports[0].camera = cameraNode.GetComponent("Camera");
}

void CreatePlayer()
{
	
	playerNode_ = scene_.CreateChild("PlayerNode");
	Node@ cameraNode = playerNode_.CreateChild("CameraNode");
	cameraNode.CreateComponent("Camera");
	cameraNode.Translate(Vector3(0,1.7,0));
	
	ScriptFile@ sFile = cache.GetResource("ScriptFile","Resources/Scripts/GameObjects.as");
	playerNode_.CreateScriptObject(sFile,"PlayerObject");
	
	cameraNode.CreateComponent("SoundListener");
	SetSoundListener(cameraNode);
	
	SetViewportCamera(cameraNode);
	
	playerDestroyed_ = false;

}


void LoadDisplayInterface()
{
	UIElement@ displayRoot = ui.root.CreateChild("UIElement");
	
	displayRoot.LoadXML(cache.GetFile("Resources/UI/ScreenDisplay.xml"));
	
	//Load the various UI Elements
	healthFillSprite_ = displayRoot.GetChild("HealthFill", true);
	radarScreenBase_ = displayRoot.GetChild("RadarScreenBase");
	
	targetSprite_ = displayRoot.GetChild("Target");
	
	enemyCounterText_ = displayRoot.GetChild("EnemyCounter");
	playerScoreText_ = displayRoot.GetChild("PlayerScore");
	
	
	statusText_ = displayRoot.GetChild("StatusText");
	playerScoreMessageText_ = displayRoot.GetChild("ScoreMessage");
	optionsInfoText_ = displayRoot.GetChild("OptionInfo");
}

void LoadAttributeAnimations()
{
	textAnimation_ = cache.GetResource("ValueAnimation", "Resources/AttributeAnimations/GameStartCounterAnimation.xml");
	damageAnimation_ = cache.GetResource("ValueAnimation", "Resources/AttributeAnimations/DamageWarningAnimation.xml");
}



void StartCounterToGame()
{
	
	statusText_.SetAttributeAnimation("Text", textAnimation_,WM_ONCE);
}



void CreateAudioSystem()
{
    audio.masterGain[SOUND_MASTER] = 0.75;
    audio.masterGain[SOUND_MUSIC] = 0.13;
	audio.masterGain[SOUND_EFFECT] = 0.5;
	
	Node@ backgroundMusicNode = scene_.CreateChild();
	backgroundMusicSource_ = backgroundMusicNode.CreateComponent("SoundSource");
    backgroundMusicSource_.soundType = SOUND_MUSIC;
	
}

void SetSoundListener(Node@ listenerNode)
{
	if(listenerNode.GetComponent("SoundListener") is null)
	{
		return;
	}
	audio.listener = listenerNode.GetComponent("SoundListener");
}

void LoadBackgroundResources()
{
	cache.BackgroundLoadResource("Model","Resources/Models/drone_body.mdl");
	cache.BackgroundLoadResource("Model","Resources/Models/drone_arm.mdl");
	cache.BackgroundLoadResource("Animation","Resources/Models/open_arm.ani");
	cache.BackgroundLoadResource("Animation","Resources/Models/close_arm.ani");
	
	cache.BackgroundLoadResource("Texture2D", "Resources/Textures/explosion.png");
	
	cache.BackgroundLoadResource("ParticleEffect", "Resources/Particles/bullet_particle.xml");
	cache.BackgroundLoadResource("ParticleEffect", "Resources/Particles/explosion.xml");
	
	cache.BackgroundLoadResource("Material","Resources/Materials/drone_arm.xml");
	cache.BackgroundLoadResource("Material","Resources/Materials/drone_body.xml");
	cache.BackgroundLoadResource("Material", "Resources/Materials/bullet_particle.xml");
	cache.BackgroundLoadResource("Material", "Resources/Materials/explosion.xml");
	
	cache.BackgroundLoadResource("Texture2D", "Resources/Textures/drone_sprite.png");
	cache.BackgroundLoadResource("Texture2D", "Resources/Textures/health_bar_green.png");
	cache.BackgroundLoadResource("Texture2D", "Resources/Textures/health_bar_red.png");
	cache.BackgroundLoadResource("Texture2D", "Resources/Textures/health_bar_yellow.png");
	
	cache.BackgroundLoadResource("Sound", "Resources/Sounds/boom1.wav");
	
	
}


void CreateScene()
{
	XMLFile@ file = cache.GetResource("XMLFile", "Resources/Objects/Scene.xml");
	
	scene_ = Scene();
	scene_.LoadXML(file.root);
	scene_.updateEnabled = false;
}

void PlayBackgroundMusic(String musicName)
{
	Sound@ musicFile = cache.GetResource("Sound",musicName);
	
	if(musicFile is null)
		return;
		
    musicFile.looped = true;
    backgroundMusicSource_.Play(musicFile);
}


void SubscribeToEvents()
{
	SubscribeToEvent("KeyDown","HandleKeyDown");
	SubscribeToEvent("Update", "HandleUpdate");
	SubscribeToEvent("MouseMove", "HandleMouseMove");
	SubscribeToEvent("MouseButtonDown", "HandleMouseClick");
	SubscribeToEvent("PlayerHit","HandlePlayerHit");
	SubscribeToEvent("DroneDestroyed", "HandleDroneDestroyed");
	SubscribeToEvent("CountFinished", "HandleCountFinished");
	SubscribeToEvent("SoundGenerated", "HandleSoundGenerated");
	SubscribeToEvent("PlayerDestroyed", "HandlePlayerDestroyed");
	SubscribeToEvent(scene_.physicsWorld, "PhysicsPreStep", "HandleFixedUpdate");
	
}


void HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
	int key = eventData["key"].GetInt();
	
	if(key == KEY_ESC)
		onQuit_ = true;
	else if(key == KEY_F2)
		debugHud.ToggleAll();
	else if(gameState_ == GS_OUTGAME)
	{
		HandleKeyOnOutGame(key);
	}
	else 
	{
		HandleKeyOnInGame(key);
	}
		
}

void PauseGame()
{
	scene_.updateEnabled = !scene_.updateEnabled;
	
	if(scene_.updateEnabled)
	{
		statusText_.text = "";
		gameState_ = GS_INGAME;
	}
	else
	{
		statusText_.text = "PAUSED";
		gameState_ = GS_PAUSED;
	}
	
	targetSprite_.visible = scene_.updateEnabled;
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
		PauseGame();
	}
}

void HandleUpdate(StringHash eventType, VariantMap& eventData)
{
	if(onQuit_)
	{
		engine.Exit();
	}
	else if(playerDestroyed_ && gameState_ == GS_INGAME)
	{
		InitiateGameOver();
	}
	else if(gameState_ == GS_INGAME)
	{
		joystickUpdate(joydirection_);
	}
}

void InitiateGameOver()
{
	scene_.updateEnabled = false;
	gameState_ = GS_OUTGAME;
	
	CleanupScene();
	PlayBackgroundMusic("Resources/Sounds/defeated.ogg");
	
	targetSprite_.visible = false;
	statusText_.text = "YOU FAILED";
	playerScoreMessageText_.text = "Score : " + String(playerScore_);
	optionsInfoText_.text = optionsMessage_ ;

}

void CleanupScene()
{
	//Remove All Nodes with script object : Drones, Bullets and even the player
	Array<Node@> scriptedNodes = scene_.GetChildrenWithScript(true);
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


void HandleMouseMove(StringHash eventType, VariantMap& eventData)
{
	if(gameState_ != GS_INGAME)
	{
		return;
	}
	
	int dx = eventData["DX"].GetInt();
	int dy = eventData["DY"].GetInt();
	
	RotatePlayer(dx, dy);
}


void RotatePlayer(int dx, int dy)
{
	VariantMap eventData;
	eventData["DX"] = dx;
	eventData["DY"] = dy;
	SendEvent("PlayerRotation", eventData);
	
	radarScreenBase_.rotation = -playerNode_.worldRotation.yaw;
}


void HandleMouseClick(StringHash eventType, VariantMap& eventData)
{
	if(gameState_ != GS_INGAME)
	{
		return;
	}
	
	int mouseButton = eventData["Button"].GetInt();
	
	if(mouseButton == MOUSEB_LEFT)
	{
		Fire();
	}
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


int GetDroneCount()
{
	Array<Node@> scriptNodes = scene_.GetChildrenWithScript(true);
	int count = 0;
	
	for(uint i=0; i < scriptNodes.length ; i++)
	{
		Node@ droneNode = scriptNodes[i];
		Sprite@ nodeSprite = droneNode.vars["Sprite"].GetPtr();
		
		if(nodeSprite !is null)
		{
			count += 1;
		}

	}
	
	return count;
}

void HandlePlayerHit(StringHash eventType, VariantMap& eventData)
{
	//Update Health
	float playerHealthFraction = eventData["CurrentHealthFraction"].GetFloat();
	
	
	int range = 512 - int( 512 * playerHealthFraction);
	healthFillSprite_.imageRect = IntRect(range, 0, 512 + range, 64);
	UpdateHealthTexture(playerHealthFraction);
	
	
	//Show Warning
	radarScreenBase_.SetAttributeAnimation("Color", damageAnimation_, WM_ONCE);
	PlaySoundFX(cameraNode_,"Resources/Sounds/boom5.ogg");
	
	if(playerHealthFraction == 0)
	{
		playerDestroyed_ = true;
	}
}


void HandleDroneDestroyed(StringHash eventType, VariantMap& eventData)
{
	playerScore_ += eventData["DronePoint"].GetInt();
	UpdateScoreDisplay();
}

void HandlePlayerDestroyed(StringHash eventType, VariantMap& eventData)
{
	Vector3 camPosition = eventData["CamPosition"].GetVector3();
	Quaternion camRotation = eventData["CamRotation"].GetQuaternion();
	cameraNode_.worldRotation = camRotation;
	cameraNode_.worldPosition = camPosition;
	
	SetViewportCamera(cameraNode_);
	cameraNode_.GetChild("DirectionalLight").enabled = true;
	
	SetSoundListener(cameraNode_);
}

void HandleCountFinished(StringHash eventType, VariantMap& eventData)
{
	CreatePlayer();
	
	cameraNode_.GetChild("DirectionalLight").enabled = false;
	
	//The following two lines come into play when restarting the game
	healthFillSprite_.imageRect = IntRect(0, 0, 512, 64);
	UpdateHealthTexture(1);
	
	scene_.updateEnabled = true;
	gameState_ = GS_INGAME;
	
	targetSprite_.visible = true;
	enemyCounterText_.text = 0;
	playerScoreText_.text = 0;
}

void UpdateScoreDisplay()
{
	playerScoreText_.text = playerScore_;
}
 
 
void SpawnDrone()
{
	Node@ droneNode = scene_.CreateChild();
	
	XMLFile@ file = cache.GetResource("XMLFile", "Resources/Objects/LowLevelDrone.xml");
	droneNode.LoadXML(file.root);
	
	float nodeYaw = Random(360);
	droneNode.rotation = Quaternion(0,nodeYaw, 0);
	droneNode.Translate(Vector3(0,7,40));
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


void UpdateDroneSprites()
{
	Array<Node@> scriptNodes = scene_.GetChildrenWithScript(true);
	int count = 0;
	
	for(uint i=0; i < scriptNodes.length ; i++)
	{
		Node@ droneNode = scriptNodes[i];
		Sprite@ nodeSprite = droneNode.vars["Sprite"].GetPtr();
		
		if(nodeSprite !is null)
		{
			Vector3 relativePos = droneNode.worldPosition - playerNode_.worldPosition ;
			nodeSprite.position = Vector2(relativePos.x, -(relativePos.z))* SCENE_TO_UI_SCALE;
			count += 1;
		}
	
	}
	
	enemyCounterText_.text = count;
	
}

void UpdateHealthTexture(float healthFraction)
{
	if(healthFraction > 0.5)
	{
		healthFillSprite_.texture = cache.GetResource("Texture2D", "Resources/Textures/health_bar_green.png");
	}
	else if(healthFraction > 0.2)
	{
		healthFillSprite_.texture = cache.GetResource("Texture2D", "Resources/Textures/health_bar_yellow.png");
	}
	else
	{
		healthFillSprite_.texture = cache.GetResource("Texture2D", "Resources/Textures/health_bar_red.png");
	}
}


void Fire()
{	
	SendEvent("ActivateWeapon");
}


void HandleSoundGenerated(StringHash eventType, VariantMap& eventData)
{
	Node@ soundNode = eventData["SoundNode"].GetPtr();
	String soundName = eventData["SoundName"].GetString();
	
	PlaySoundFX(soundNode, soundName);
}


void PlaySoundFX(Node@ soundNode, String soundName )
{
	SoundSource3D@ source = soundNode.CreateComponent("SoundSource3D");
	
	Sound@ sound = cache.GetResource("Sound", soundName);
    source.SetDistanceAttenuation(0.2, 120, 0.1);
	source.soundType = SOUND_EFFECT;
    source.Play(sound);
    source.autoRemove = true;
}

